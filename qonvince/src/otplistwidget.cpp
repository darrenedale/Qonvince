/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Qonvince. If not, see <http://www.gnu.org/licenses/>.
 */

/** \file otplistwidget.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the OtpListWidget class.
  *
  * When retrieving items, static_cast() is used to cast QListWidgetItem * to
  * OtpListWidgetItem * because even though it's a downcast through the inheritance
  * hierarchy, we know that the only items stored in the list are OtpListWidgetItem
  * instances because we've overridden addItem().
  *
  * \todo visual feedback that a manual refresh has executed (e.g. flash
  *   code)?
  * \todo optimise painting
  */

#include "otplistwidget.h"

#include <QDebug>
#include <QListWidget>
#include <QApplication>
#include <QMenu>
#include <QClipboard>
#include <QScrollBar>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include <QStringBuilder>
#include <QToolTip>
#include <QStyleHints>
#include <qmath.h>

#include "application.h"
#include "otp.h"
#include "otplistwidgetitem.h"
#include "otpqrcodereader.h"

#define QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE 32
#define QONVINCE_OTPCODELISTWIDGET_PROGRESS_GAUGE_SIZE 0.5L /* ratio of gauge size to item height */
#define QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN 4
#define QONVINCE_OTPCODELISTWIDGET_DO_BACKGROUND_MESSAGE true
#define QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_FONT_SIZE 20
#define QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_RECT_HEIGHT 40
#define QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE 22

#define QONVINCE_OTPCODELISTWIDGET_TOTP_CRITICAL_THRESHOLD 4
#define QONVINCE_OTPCODELISTWIDGET_TOTP_WARNING_THRESHOLD 8


using namespace Qonvince;


OtpListWidget::OtpListWidget(QWidget * parent)
: QListWidget(parent),
  m_hoverItemIndex(-1),
  m_tickTimerIsResynchronising(false),
  m_imageDropEnabled(OtpQrCodeReader::isAvailable()),
  m_tickTimerId(-1),
  m_doubleClickWaitTimer(),
  m_receivedDoubleClickEvent(false),
  m_itemMenu(),
  m_menuCodeItem(nullptr) {
	m_itemMenu.addAction(QIcon::fromTheme("document-edit"), tr("Edit"), this, &OtpListWidget::onEditActionTriggered);
	m_itemMenu.addAction(tr("Remove icon"), this, &OtpListWidget::onRemoveIconActionTriggered);
	m_itemMenu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &OtpListWidget::onCopyActionTriggered);
	m_itemMenu.addAction(QIcon::fromTheme("view-refresh", QIcon(":/icons/codeactions/refresh")), tr("Refresh now"), this, &OtpListWidget::onRefreshActionTriggered);
	m_itemMenu.addAction(QIcon::fromTheme("list-remove", QIcon(":/icons/codeactions/remove")), tr("Remove"), this, &OtpListWidget::onRemoveActionTriggered);

	m_countdownColour = palette().color(QPalette::WindowText).lighter(150);
	m_countdownWarningColour = m_countdownColour;
	m_countdownCriticalColour = m_countdownColour;

	m_doubleClickWaitTimer.setInterval(qonvinceApp->styleHints()->mouseDoubleClickInterval());
	m_doubleClickWaitTimer.setSingleShot(true);

	connect(&m_doubleClickWaitTimer, &QTimer::timeout, [this]() {
		QMouseEvent ev(QMouseEvent::MouseButtonRelease, mapFromGlobal(QCursor::pos()), Qt::LeftButton, Qt::LeftButton, 0);
		mouseClickEvent(&ev);
	});

	connect(&(qonvinceApp->settings()), &Settings::codeLabelDisplayStyleChanged, this, qOverload<>(&OtpListWidget::update));

	synchroniseTickTimer();
}


OtpListWidget::~OtpListWidget() {
	// TODO are these strictly necessary?
	m_doubleClickWaitTimer.stop();
}


void OtpListWidget::synchroniseTickTimer() {
	if(-1 != m_tickTimerId) {
		killTimer(m_tickTimerId);
	}

	m_tickTimerIsResynchronising = true;

	/* sync just after a 1sec boundary so that the timer doesn't tick down
	 * to 0 before the code objects generate their new code and emit their
	 * signals - effectively this gives the codes a 0.05s window to run the
	 * code to actually generate the new code */
	/* TODO could we obviate the need for this by synchronising both this
	 * and the code objects to a global sync timer at application start?
	 */
	m_tickTimerId = startTimer(50 + (QDateTime::currentMSecsSinceEpoch() % 1000), Qt::PreciseTimer);
}


OtpListWidgetItem * OtpListWidget::findOtp(const Otp * otp) const {
	for(int i = count() - 1; i >= 0; --i) {
		OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(i));

		if(it->otp() == otp) {
			return it;
		}
	}

	return nullptr;
}


Otp * OtpListWidget::hoveredCodeSpecification() const {
	if(0 > m_hoverItemIndex) {
		return nullptr;
	}

	OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

	if(!it) {
		return nullptr;
	}

	return it->otp();
}


void OtpListWidget::onOtpChanged() {
	Otp * otp = dynamic_cast<Otp *>(sender());

	if(!otp) {
		qWarning() << "failed to find sender of newCodeGenerated() signal";
		return;
	}

	OtpListWidgetItem * otpItem = findOtp(otp);

	if(!otpItem) {
		qWarning() << "failed to find list item for code" << otp->name();
		return;
	}

	QRect itemViewportRect(0, itemHeight() * row(otpItem) - verticalOffset(), width(), itemHeight());
	qDebug() << "updating item" << otp->name() << itemViewportRect;
	viewport()->update(itemViewportRect);
}


void OtpListWidget::updateCountdowns() {
	if(m_tickTimerIsResynchronising) {
		killTimer(m_tickTimerId);
		m_tickTimerIsResynchronising = false;
		m_tickTimerId = startTimer(1000, Qt::VeryCoarseTimer);
	}

	viewport()->update();
}


void OtpListWidget::addCode(Otp * otp) {
	addItem(new OtpListWidgetItem(std::unique_ptr<Otp>(otp)));
}


void OtpListWidget::addCode(const QByteArray & seed) {
	addItem(new OtpListWidgetItem(std::make_unique<Otp>(Otp::CodeType::Totp, seed)));
}


void OtpListWidget::addCode(const QString & name, const QByteArray & seed) {
	addItem(new OtpListWidgetItem(std::make_unique<Otp>(Otp::CodeType::Totp, name, seed)));
}


void OtpListWidget::addItem(OtpListWidgetItem * item) {
	item->setSizeHint({width(), 40});
	QListWidget::addItem(item);
	Otp * code = item->otp();
	connect(code, &Otp::changed, this, &OtpListWidget::onOtpChanged);
	connect(code, &Otp::newCodeGenerated, this, &OtpListWidget::onOtpChanged);

#ifndef NDEBUG
	connect(code, &Otp::newCodeGenerated, this, &OtpListWidget::debugLogNewCode);
#endif
	//	Q_EMIT codeAdded(code);
}


Otp * OtpListWidget::otp(int i) const {
	if(0 <= i && count() > i) {
		OtpListWidgetItem * it = dynamic_cast<OtpListWidgetItem *>(item(i));

		if(!!it) {
			return it->otp();
		}
	}

	return nullptr;
}


void OtpListWidget::setCountdownColour(const QColor & colour) {
	m_countdownColour = colour;
	viewport()->update();
}


void OtpListWidget::setCountdownWarningColour(const QColor & colour) {
	m_countdownWarningColour = colour;
	viewport()->update();
}


void OtpListWidget::setCountdownCriticalColour(const QColor & colour) {
	m_countdownCriticalColour = colour;
	viewport()->update();
}


bool OtpListWidget::event(QEvent * ev) {
	if(QEvent::ToolTip == ev->type()) {
		QPoint globalMousePos(QCursor::pos());
		QPoint mousePos(mapFromGlobal(globalMousePos));
		QString txt;
		Otp * item = hoveredCodeSpecification();

		if(!!item) {
			QString label;

			switch(qonvinceApp->settings().codeLabelDisplayStyle()) {
				case Settings::IssuerAndName:
					label = item->issuer() % ": " % item->name();
					break;

				case Settings::IssuerOnly:
					label = item->issuer();
					break;

				case Settings::NameOnly:
					label = item->name();
					break;
			}

			if(m_copyIconHitRect.contains(mousePos)) {
				txt = tr("Copy the current code for %1 to the clipboard.").arg(label);
			}
			else if(m_refreshIconHitRect.contains(mousePos)) {
				txt = tr("Refresh the code for %1 now.").arg(label);
			}
			else if(m_removeIconHitRect.contains(mousePos)) {
				txt = tr("Remove %1 from %2.").arg(label).arg(QApplication::applicationDisplayName());
			}
			else if(m_revealIconHitRect.contains(mousePos)) {
				txt = tr("Reveal the current code for %1.").arg(label);
			}
			else if(-1 != m_hoverItemIndex) {
				/* TODO this was put together quickly - review and make construction of tooltip
				 * more efficient. */
				txt = tr("Double-click to edit %1.").arg(label);

				if(qonvinceApp->settings().copyCodeOnClick()) {
					txt = "<html><body><p>" + txt + "</p><p>" + tr("Click to copy the current code for %1 to the clipboard.").arg(label) + "</p></body></html>";
				}
			}
		}

		if(!txt.isEmpty()) {
			QToolTip::showText(globalMousePos, txt, this);
			ev->accept();
			return true;
		}
	}

	return QListWidget::event(ev);
}


void OtpListWidget::timerEvent(QTimerEvent * ev) {
	if(ev->timerId() == m_tickTimerId) {
		updateCountdowns();
		ev->accept();
		return;
	}

	QListWidget::timerEvent(ev);
}


void OtpListWidget::enterEvent(QEvent *) {
	setMouseTracking(true);
}


void OtpListWidget::leaveEvent(QEvent *) {
	if(-1 != m_hoverItemIndex) {
		int oldIndex = m_hoverItemIndex;
		m_hoverItemIndex = -1;
		setDirtyRegion(QRect(0, itemHeight() * oldIndex, width(), itemHeight()));
	}

	setMouseTracking(false);
}


void OtpListWidget::mouseMoveEvent(QMouseEvent * ev) {
	OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(itemAt(ev->x(), ev->y()));
	int oldHoverIndex = m_hoverItemIndex;

	if(!it) {
		if(m_hoverItemIndex == -1) {
			return;
		}

		m_hoverItemIndex = -1;
	}
	else {
		int i = row(it);

		if(i == m_hoverItemIndex) {
			/* TODO capture whether the last rendering of the hovered item
			 * rendered the refresh/remove icons in "Active" state or not
			 * and use this to determine whether a repaint is really necessary */
			if(m_hoverItemIndex == -1) {
				return;
			}
		}
		else if(0 <= i && count() > i) {
			m_hoverItemIndex = i;
		}
		else {
			m_hoverItemIndex = -1;
		}
	}

	QRegion damage;

	if(-1 != oldHoverIndex) {
		damage += QRect(0, itemHeight() * oldHoverIndex, viewport()->width(), itemHeight());
	}

	if(-1 != m_hoverItemIndex) {
		damage += QRect(0, itemHeight() * m_hoverItemIndex, viewport()->width(), itemHeight());
	}

	viewport()->update(damage);
}


void OtpListWidget::keyReleaseEvent(QKeyEvent * ev) {
	if(ev->matches(QKeySequence::Copy)) {
		if(0 > m_hoverItemIndex) {
			return;
		}

		OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

		if(!it) {
			return;
		}

		QApplication::clipboard()->setText(it->otp()->code());
	}
}


void OtpListWidget::mousePressEvent(QMouseEvent * ev) {
	if(Qt::LeftButton == ev->button() && (m_refreshIconHitRect.contains(ev->pos()) || m_removeIconHitRect.contains(ev->pos()) || m_copyIconHitRect.contains(ev->pos()) || m_revealIconHitRect.contains(ev->pos()))) {
		m_mousePressLeftStart = ev->pos();
		ev->accept();
		return;
	}
	else {
		m_mousePressLeftStart = QPoint();
	}

	QListWidget::mousePressEvent(ev);
}


void OtpListWidget::mouseReleaseEvent(QMouseEvent * ev) {
	/* check if one of the action icons has been clicked... */
	if(Qt::LeftButton == ev->button()) {
		QPoint mousePos(ev->pos());

		if(m_refreshIconHitRect.contains(m_mousePressLeftStart) && m_refreshIconHitRect.contains(mousePos)) {
			/* clicked on refresh */
			OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

			if(it) {
				if(Otp::CodeType::Hotp == it->otp()->type()) {
					it->otp()->incrementCounter();
				}

				it->otp()->refreshCode();
				ev->accept();
				return;
			}
		}
		else if(m_removeIconHitRect.contains(m_mousePressLeftStart) && m_removeIconHitRect.contains(mousePos)) {
			/* clicked on remove */
			m_menuCodeItem = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));
			onRemoveActionTriggered();
			ev->accept();
			return;
		}
		else if(m_copyIconHitRect.contains(m_mousePressLeftStart) && m_copyIconHitRect.contains(mousePos)) {
			/* clicked on copy */
			OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

			if(!!it) {
				QApplication::clipboard()->setText(it->otp()->code());
				const Settings & settings = qonvinceApp->settings();

				if(settings.clearClipboardAfterInterval() && 0 < settings.clipboardClearInterval()) {
					QTimer::singleShot(1000 * settings.clipboardClearInterval(), qonvinceApp, &Application::clearClipboard);
				}

				ev->accept();
				return;
			}

			return;
		}
		else if(m_revealIconHitRect.contains(m_mousePressLeftStart) && m_revealIconHitRect.contains(mousePos)) {
			OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

			if(!it) {
				qWarning() << "clicked reveal for a non-existent item";
				return;
			}

			Otp * otp = it->otp();

			if(!otp) {
				qWarning() << "clicked reveal for an item that has no Otp object";
				return;
			}

			if(contains(m_revealedPasscodes, otp)) {
				qWarning() << "apparently clicked reveal for a code that is already revealed";
				return;
			}

			const Settings & settings = qonvinceApp->settings();
			m_revealedPasscodes.push_back(otp);

			// even if otp is removed while this timer is running, it's safe -
			// indeed necessary - to still execute the lambda because the ponter
			// is only used to remove it from the array of revealed passcodes, it
			// is not dereferenced
			QTimer::singleShot(1000 * settings.codeRevealTimeout(), Qt::VeryCoarseTimer, [this, otp]() {
				removeAll(m_revealedPasscodes, otp);
				viewport()->update();
			});

			viewport()->update();
			return;
		}
	}

	/* ... if no action icon clicked, wait to see if it's a double click. if a double-click
	 * event notification is received within the timeout, the widget will respond as if
	 * the user double-clicked; if the timer times out, the widget will respond as if a
	 * single-click was received. this means that for single-clicks there is a short
	 * delay before action is taken, which is why the action icons are checked before
	 * starting the double-click checking timer - so that clicks on the icons don't feel
	 * unresponsive */
	if(m_doubleClickWaitTimer.isActive() || m_receivedDoubleClickEvent) {
		m_receivedDoubleClickEvent = false;
		QListWidget::mouseReleaseEvent(ev);
		return;
	}

	m_doubleClickWaitTimer.start();
	ev->accept();
	QListWidget::mouseReleaseEvent(ev);
	return;
}


void OtpListWidget::mouseDoubleClickEvent(QMouseEvent * ev) {
	m_doubleClickWaitTimer.stop();
	m_receivedDoubleClickEvent = true;

	// don't emit code double-clicked signal if one of the action icons was double-clicked?
	if(m_removeIconHitRect.contains(ev->pos()) || m_refreshIconHitRect.contains(ev->pos()) || m_copyIconHitRect.contains(ev->pos())) {
		QListWidget::mouseDoubleClickEvent(ev);
		return;
	}

	OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

	if(!it) {
		QListWidget::mouseDoubleClickEvent(ev);
		return;
	}

	Q_EMIT codeDoubleClicked(it->otp());
}


void OtpListWidget::mouseClickEvent(QMouseEvent * ev) {
	/* our "fake" mouseClickEvent method is only called if the user clicked
	 * on an item (but not one of its action icons) and it didn't turn out
	 * to be a double-click */
	if(Qt::LeftButton == ev->button()) {
		OtpListWidgetItem * it = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

		if(it) {
			Q_EMIT codeClicked(it->otp());
		}

		return;
	}
}


void OtpListWidget::contextMenuEvent(QContextMenuEvent * ev) {
	m_menuCodeItem = static_cast<OtpListWidgetItem *>(itemAt(ev->pos()));

	if(!m_menuCodeItem) {
		return;
	}

	ev->accept();
	m_itemMenu.move(ev->globalPos());
	m_itemMenu.show();
}


void OtpListWidget::onEditActionTriggered() {
	if(!m_menuCodeItem) {
		return;
	}

	Q_EMIT editCodeRequested(m_menuCodeItem->otp());
}


void OtpListWidget::onCopyActionTriggered() {
	if(!m_menuCodeItem) {
		return;
	}

	QApplication::clipboard()->setText(m_menuCodeItem->otp()->code());
}


void OtpListWidget::onRemoveIconActionTriggered() {
	if(!m_menuCodeItem) {
		return;
	}

	m_menuCodeItem->otp()->setIcon(QIcon());
}


void OtpListWidget::onRefreshActionTriggered() {
	if(!m_menuCodeItem) {
		return;
	}

	m_menuCodeItem->otp()->refreshCode();
}


void OtpListWidget::onRemoveActionTriggered() {
	if(!m_menuCodeItem) {
		return;
	}

	if(QMessageBox::Yes == QMessageBox::question(this, tr("%1: Remove code").arg(qApp->applicationName()), tr("Are you sure you wish to remove this code?"), QMessageBox::Yes | QMessageBox::No)) {
		delete m_menuCodeItem;
		m_menuCodeItem = nullptr;

		/* reset the hover item in case it's the one we've just deleted
		 * it will be updated on the next mouse move event we receive */
		m_mousePressLeftStart = {};
		m_removeIconHitRect = m_refreshIconHitRect = {};
		m_hoverItemIndex = -1;
		return;
	}
}


/* TODO this needs a bit of TLC to tidy it up and perhaps
 * optimise it a little
 * also, should it be a delegate instead?
 * also, it only works when the vertical scroll mode is per-item
 * rather than per-pixel
 */
void OtpListWidget::paintEvent(QPaintEvent * ev) {
	ev->accept();

	m_revealIconHitRect.setRect(0, 0, 0, 0);
	m_copyIconHitRect.setRect(0, 0, 0, 0);
	m_refreshIconHitRect.setRect(0, 0, 0, 0);
	m_removeIconHitRect.setRect(0, 0, 0, 0);

	int w(viewport()->width());

	QRegion paintRegion(ev->region());
	QPainter painter;
	painter.begin(viewport());

	QFont nameFont(font());
	QFont codeFont(nameFont);
	QPalette pal(palette());

	nameFont.setStyleStrategy(static_cast<QFont::StyleStrategy>(QFont::PreferQuality | QFont::PreferAntialias));
	codeFont.setStyleStrategy(static_cast<QFont::StyleStrategy>(QFont::PreferQuality | QFont::PreferAntialias));

	/* paint background */
	if(QONVINCE_OTPCODELISTWIDGET_DO_BACKGROUND_MESSAGE && m_imageDropEnabled) {
		QString txt(tr("Drop QR code images here ..."));
		int y = height() - QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_RECT_HEIGHT;
		int w = viewport()->width();
		QFont bgFont(nameFont);
		bgFont.setPixelSize(QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_FONT_SIZE);
		painter.save();
		painter.setPen(QColor(192, 192, 192));
		painter.setFont(bgFont);
		painter.drawText(0, y, w, QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_RECT_HEIGHT, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignCenter, txt);
		painter.restore();
	}

	nameFont.setBold(false);
	nameFont.setPixelSize(qMax(int(nameFont.pixelSize() * 1.2), itemHeight() / 3));
	codeFont.setBold(true);
	codeFont.setPixelSize(itemHeight() / 2);

	int i = verticalScrollBar()->value();
	int last = qMin(count(), i + qCeil(qreal(height()) / qreal(itemHeight())));
	QBrush backgroundBrush;
	QPen normalPen(painter.pen());

	Settings::CodeLabelDisplayStyle style(qonvinceApp->settings().codeLabelDisplayStyle());
	int h = itemHeight();
	QRect itemRect(0, 0, w, h);
	int timerRectSize = static_cast<int>(static_cast<long double>(h) * QONVINCE_OTPCODELISTWIDGET_PROGRESS_GAUGE_SIZE);
	QRect iconRect(QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE);
	QRectF timerRect(0.5L + itemRect.right() - timerRectSize - (3 * (QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN)), 0.5L + ((h - timerRectSize) / 2.0L), timerRectSize, timerRectSize);
	QRect codeRect(itemRect);
	codeRect.setRight(timerRect.left() - (3 * QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN));
	/* NB right() of this rect is set once we know how far left the code and timer rendering extends */
	QRect nameRect(QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0, 0, h);

	for(; i < last; ++i) {
		if(paintRegion.intersects(itemRect)) {
			/* reset the left margin of the code rect so that when we render the
			 * next code, which might have more digits than the last, we have the
			 * best chance of fitting in the whole code */
			codeRect.setLeft(QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN);

			Otp * code = static_cast<OtpListWidgetItem *>(item(i))->otp();
			QString codeString;

			/* only show the code if it's not hidden, or is hidden but user has manually
			 * revealed it and it's not timed out */
			bool onDemand = code->revealOnDemand();
			bool showCode = !onDemand || contains(m_revealedPasscodes, code);

			if(showCode) {
				codeString = code->code();
			}

			QString name(code->name());
			QString issuer(code->issuer());
			QString displayName;
			QIcon icon(code->icon());
			Otp::CodeType codeType = code->type();

			switch(style) {
				case Settings::IssuerAndName:
					if(issuer.isEmpty()) {
						displayName = name;
					}
					else if(name.isEmpty()) {
						displayName = issuer;
					}
					else {
						displayName = issuer % ": " % name;
					}
					break;

				case Settings::NameOnly:
					displayName = name;
					break;

				case Settings::IssuerOnly:
					displayName = issuer;
					break;
			}

			int countdown = code->timeToNextCode();
			int interval = (0 == code->interval() ? Otp::DefaultInterval : code->interval());

			if(displayName.isEmpty()) {
				displayName = tr("<unnamed>");
			}

			if(!onDemand && codeString.isEmpty()) {
				codeString = tr("<no code>");
			}

			QPen itemPen(normalPen);

			if(i == m_hoverItemIndex) {
				backgroundBrush = pal.highlight();
				itemPen.setColor(pal.highlightedText().color());
			}
			else if(0 == (i % 2)) {
				backgroundBrush = pal.base();
			}
			else {
				backgroundBrush = pal.alternateBase();
			}

			painter.setPen(itemPen);

			/* draw the item background */
			painter.fillRect(itemRect, backgroundBrush);

			/* draw the item icon */
			if(!icon.isNull()) {
				icon.paint(&painter, iconRect);
			}

			/* draw the current code */
			if(QONVINCE_OTPCODELISTWIDGET_TOTP_WARNING_THRESHOLD >= countdown && Otp::CodeType::Totp == codeType) {
				painter.setPen(itemPen.color().lighter(200 + (30 * (5 - countdown))));
			}

			QPoint mousePos(mapFromGlobal(QCursor::pos()));

			if(!onDemand || showCode) {
				painter.setFont(codeFont);
				painter.drawText(codeRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight, codeString, &codeRect);
				nameRect.setRight(codeRect.left() - (2 * QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN));
			}
			else {
				nameRect.setRight(codeRect.right() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - (2 * QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN));
			}

			/* draw the item new code timeout counter */
			if(Otp::CodeType::Totp == codeType) {
				QPen timerPen(m_countdownColour);
				timerPen.setWidthF(0.5);
				QBrush timerBrush(m_countdownColour);

				if(QONVINCE_OTPCODELISTWIDGET_TOTP_CRITICAL_THRESHOLD >= countdown) {
					timerBrush.setColor(m_countdownCriticalColour);
				}
				else if(QONVINCE_OTPCODELISTWIDGET_TOTP_WARNING_THRESHOLD >= countdown) {
					timerBrush.setColor(m_countdownWarningColour);
				}

				painter.save();
				painter.setPen(Qt::transparent);
				painter.setRenderHint(QPainter::Antialiasing, true);
				painter.setBrush(timerBrush);

				/* pies take angles in 1/16ths of a degree anticlockwise from 3 o'clock
				 * so 5760 is a full circle, 1440 is a quarter circle and represents
				 * 12 o'clock */
				painter.drawPie(timerRect, 1440, (5760 * countdown / interval));
				painter.setPen(timerPen);
				painter.setBrush(Qt::transparent);
				painter.drawEllipse(timerRect);
				painter.restore();
			}

			/* draw the item name */
			/* NB this has to be done after the code is drawn so that we can ensure the name does not
			 * bleed into the code */
			painter.setFont(nameFont);
			itemPen.setColor(itemPen.color().lighter(200));
			painter.setPen(itemPen);
			painter.drawText(nameRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft, displayName);

			/* draw the action icons */
			if(i == m_hoverItemIndex) {
				painter.save();
				painter.setRenderHint(QPainter::Antialiasing, false);
				painter.setBrush(backgroundBrush.color().darker(125));
				painter.setPen(Qt::NoPen);

				if(onDemand && !showCode) {
					m_revealIconHitRect = QRect(codeRect.right() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, itemRect.top() + ((itemRect.height() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE) / 2), QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE);
					QIcon icon = QIcon::fromTheme("password-show-on", QIcon(":/icons/codeactions/reveal"));

					if(m_revealIconHitRect.contains(mousePos)) {
						painter.drawRoundedRect(m_revealIconHitRect, 3, 3);
					}

					painter.drawPixmap(m_revealIconHitRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));
				}

				m_removeIconHitRect = QRect(itemRect.right() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, itemRect.top() + ((itemRect.height() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE) / 2), QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE);
				QIcon icon(QIcon::fromTheme("list-remove", QIcon(":/icons/codeactions/remove")));
				bool doHoverEffect = m_removeIconHitRect.contains(mousePos);

				if(doHoverEffect) {
					painter.drawRoundedRect(m_removeIconHitRect, 3, 3);
				}

				painter.drawPixmap(m_removeIconHitRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));

				m_refreshIconHitRect = m_removeIconHitRect.adjusted(0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0, 0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0);
				doHoverEffect = m_refreshIconHitRect.contains(mousePos);
				icon = QIcon::fromTheme("view-refresh", QIcon(":/icons/codeactions/refresh"));

				if(doHoverEffect) {
					painter.drawRoundedRect(m_refreshIconHitRect, 3, 3);
				}

				painter.drawPixmap(m_refreshIconHitRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));

				m_copyIconHitRect = m_refreshIconHitRect.adjusted(0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0, 0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0);
				doHoverEffect = m_copyIconHitRect.contains(mousePos);
				icon = QIcon::fromTheme("edit-copy", QIcon(":/icons/codeactions/copy"));

				if(doHoverEffect) {
					painter.drawRoundedRect(m_copyIconHitRect, 3, 3);
				}

				painter.drawPixmap(m_copyIconHitRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));
				painter.restore();
			}
		}

		itemRect.adjust(0, h, 0, h);
		codeRect.adjust(0, h, 0, h);
		timerRect.adjust(0, h, 0, h);
		iconRect.adjust(0, h, 0, h);
		nameRect.adjust(0, h, 0, h);
	}
}


#if defined(QT_DEBUG)
void OtpListWidget::debugLogNewCode(const QString & code) const {
	Otp * c = dynamic_cast<Otp *>(sender());

	if(!!c) {
		qDebug() << "Otp object" << c->name() << "generated new code" << code;
	}
}
#endif