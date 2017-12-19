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

/** \file otplistview.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the OtpListWidget class.
  *
  * When retrieving items, static_cast() is used to cast QListWidgetItem * to
  * OtpListWidgetItem * because even though it's a downcast through the inheritance
  * hierarchy, we know that the only items stored in the list are OtpListWidgetItem
  * instances because we've overridden addItem().
  */

#include "otplistview.h"

#include <QDebug>
#include <QListView>
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
#include "otplistmodel.h"
#include "otpqrcodereader.h"
#include "qtiostream.h"

#define QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE 32
#define QONVINCE_OTPCODELISTWIDGET_PROGRESS_GAUGE_SIZE 0.5L /* ratio of gauge size to item height */
#define QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN 4
#define QONVINCE_OTPCODELISTWIDGET_DO_BACKGROUND_MESSAGE true
#define QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_FONT_SIZE 20
#define QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_RECT_HEIGHT 40
#define QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE 22

#define QONVINCE_OTPCODELISTWIDGET_TOTP_CRITICAL_THRESHOLD 4
#define QONVINCE_OTPCODELISTWIDGET_TOTP_WARNING_THRESHOLD 8


namespace Qonvince {


	OtpListView::OtpListView(QWidget * parent)
	: QListView(parent),
	  m_hoverItemIndex(-1),
	  m_tickTimerIsResynchronising(false),
	  m_imageDropEnabled(OtpQrCodeReader::isAvailable()),
	  m_tickTimerId(-1),
	  m_doubleClickWaitTimer(),
	  m_receivedDoubleClickEvent(false),
	  m_itemContextMenu() {
		m_itemContextMenu.addAction(QIcon::fromTheme("document-edit"), tr("Edit"), this, &OtpListView::onEditActionTriggered);
		m_itemContextMenu.addAction(tr("Remove icon"), this, &OtpListView::onRemoveIconActionTriggered);
		m_itemContextMenu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &OtpListView::onCopyActionTriggered);
		m_itemContextMenu.addAction(QIcon::fromTheme("view-refresh", QIcon(":/icons/codeactions/refresh")), tr("Refresh now"), this, &OtpListView::onRefreshActionTriggered);
		m_itemContextMenu.addAction(QIcon::fromTheme("list-remove", QIcon(":/icons/codeactions/remove")), tr("Remove"), this, &OtpListView::onRemoveActionTriggered);

		m_doubleClickWaitTimer.setInterval(qonvinceApp->styleHints()->mouseDoubleClickInterval());
		m_doubleClickWaitTimer.setSingleShot(true);

		connect(&m_doubleClickWaitTimer, &QTimer::timeout, [this]() {
			QMouseEvent ev(QMouseEvent::MouseButtonRelease, mapFromGlobal(QCursor::pos()), Qt::LeftButton, Qt::LeftButton, 0);
			mouseClickEvent(&ev);
		});

		connect(&(qonvinceApp->settings()), qOverload<Settings::CodeLabelDisplayStyle>(&Settings::codeLabelDisplayStyleChanged), this, qOverload<>(&OtpListView::update));

		synchroniseTickTimer();
	}


	OtpListView::~OtpListView() = default;


	void OtpListView::synchroniseTickTimer() {
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


	Otp * OtpListView::hoveredOtp() const {
		if(0 > m_hoverItemIndex) {
			return nullptr;
		}

		return qonvinceApp->otp(m_hoverItemIndex);
	}


	void OtpListView::updateCountdowns() {
		if(m_tickTimerIsResynchronising) {
			killTimer(m_tickTimerId);
			m_tickTimerIsResynchronising = false;
			m_tickTimerId = startTimer(1000, Qt::VeryCoarseTimer);
		}

		viewport()->update();
	}


	bool OtpListView::event(QEvent * ev) {
		if(QEvent::ToolTip == ev->type()) {
			QPoint globalMousePos = QCursor::pos();
			QPoint mousePos = mapFromGlobal(globalMousePos);
			QString txt;
			Otp * item = hoveredOtp();

			if(item) {
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
					txt = tr("Double-click to edit %1.").arg(label);

					if(qonvinceApp->settings().copyCodeOnClick()) {
						txt = QStringLiteral("<html><body><p>") % txt % QStringLiteral("</p><p>") % tr("Click to copy the current code for %1 to the clipboard.").arg(label) % QStringLiteral("</p></body></html>");
					}
				}
			}

			if(!txt.isEmpty()) {
				QToolTip::showText(globalMousePos, txt, this);
				ev->accept();
				return true;
			}
		}

		return QListView::event(ev);
	}


	void OtpListView::timerEvent(QTimerEvent * ev) {
		if(ev->timerId() == m_tickTimerId) {
			updateCountdowns();
			ev->accept();
			return;
		}

		QListView::timerEvent(ev);
	}


	void OtpListView::enterEvent(QEvent *) {
		setMouseTracking(true);
	}


	void OtpListView::leaveEvent(QEvent *) {
		if(-1 != m_hoverItemIndex) {
			int oldIndex = m_hoverItemIndex;
			m_hoverItemIndex = -1;
			setDirtyRegion(QRect(0, itemHeight() * oldIndex, width(), itemHeight()));
		}

		setMouseTracking(false);
	}


	void OtpListView::mouseMoveEvent(QMouseEvent * ev) {
		int newHoverIndex = -1;

		{
			auto newHoverModelIndex = indexAt(ev->pos());

			if(newHoverModelIndex.isValid()) {
				newHoverIndex = newHoverModelIndex.row();
			}
		}

		int oldHoverIndex = m_hoverItemIndex;

		if(-1 == newHoverIndex) {
			if(-1 == m_hoverItemIndex) {
				return;
			}

			m_hoverItemIndex = -1;
		}
		else {
			if(newHoverIndex == m_hoverItemIndex) {
				/* TODO capture whether the last rendering of the hovered item
			 * rendered the refresh/remove icons in "Active" state or not
			 * and use this to determine whether a repaint is really necessary */
				if(m_hoverItemIndex == -1) {
					return;
				}
			}
			else if(0 <= newHoverIndex && model()->rowCount() > newHoverIndex) {
				m_hoverItemIndex = newHoverIndex;
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


	void OtpListView::keyReleaseEvent(QKeyEvent * ev) {
		if(ev->matches(QKeySequence::Copy)) {
			if(0 > m_hoverItemIndex) {
				return;
			}

			auto otpIndex = indexAt(mapFromGlobal(QCursor::pos()));

			if(!otpIndex.isValid()) {
				return;
			}

			auto * otp = qonvinceApp->otp(otpIndex.row());

			if(!otp) {
				return;
			}

			QApplication::clipboard()->setText(otp->code());
		}
	}


	void OtpListView::mousePressEvent(QMouseEvent * ev) {
		if(Qt::LeftButton == ev->button() && (m_refreshIconHitRect.contains(ev->pos()) || m_removeIconHitRect.contains(ev->pos()) || m_copyIconHitRect.contains(ev->pos()) || m_revealIconHitRect.contains(ev->pos()))) {
			m_mousePressLeftStart = ev->pos();
			ev->accept();
			return;
		}
		else {
			m_mousePressLeftStart = {};
		}

		QListView::mousePressEvent(ev);
	}


	void OtpListView::mouseReleaseEvent(QMouseEvent *) {
		//		/* check if one of the action icons has been clicked... */
		//		if(Qt::LeftButton == ev->button()) {
		//			QPoint releasePos = ev->pos();

		//			if(m_refreshIconHitRect.contains(m_mousePressLeftStart) && m_refreshIconHitRect.contains(releasePos)) {
		//				auto * refreshItem = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

		//				if(refreshItem) {
		//					if(Otp::CodeType::Hotp == refreshItem->otp()->type()) {
		//						refreshItem->otp()->incrementCounter();
		//					}

		//					refreshItem->otp()->refreshCode();
		//					ev->accept();
		//					return;
		//				}
		//			}
		//			else if(m_removeIconHitRect.contains(m_mousePressLeftStart) && m_removeIconHitRect.contains(releasePos)) {
		//				m_actionItem = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));
		//				onRemoveActionTriggered();
		//				ev->accept();
		//				return;
		//			}
		//			else if(m_copyIconHitRect.contains(m_mousePressLeftStart) && m_copyIconHitRect.contains(releasePos)) {
		//				auto * copyItem = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

		//				if(copyItem) {
		//					QApplication::clipboard()->setText(copyItem->otp()->code());
		//					const auto & settings = qonvinceApp->settings();

		//					if(settings.clearClipboardAfterInterval() && 0 < settings.clipboardClearInterval()) {
		//						QTimer::singleShot(1000 * settings.clipboardClearInterval(), qonvinceApp, &Application::clearClipboard);
		//					}

		//					ev->accept();
		//					return;
		//				}

		//				return;
		//			}
		//			else if(m_revealIconHitRect.contains(m_mousePressLeftStart) && m_revealIconHitRect.contains(releasePos)) {
		//				auto * revealItem = static_cast<OtpListWidgetItem *>(item(m_hoverItemIndex));

		//				if(!revealItem) {
		//					qWarning() << "clicked reveal for a non-existent item";
		//					return;
		//				}

		//				Otp * otp = revealItem->otp();

		//				if(!otp) {
		//					qWarning() << "clicked reveal for an item that has no Otp object";
		//					return;
		//				}

		//				if(contains(m_revealedPasscodes, otp)) {
		//					qWarning() << "apparently clicked reveal for a code that is already revealed";
		//					return;
		//				}

		//				const auto & settings = qonvinceApp->settings();
		//				m_revealedPasscodes.push_back(otp);

		//				// even if otp is removed while this timer is running, it's safe -
		//				// indeed necessary - to still execute the lambda because the ponter
		//				// is only used to remove it from the array of revealed passcodes, it
		//				// is not dereferenced
		//				// HOWEVER if the widget is destroyed before the timout ... UB
		//				QTimer::singleShot(1000 * settings.codeRevealTimeout(), Qt::VeryCoarseTimer, [this, otp]() {
		//					removeAll(m_revealedPasscodes, otp);
		//					viewport()->update();
		//				});

		//				viewport()->update();
		//				return;
		//			}
		//		}

		//		/* ... if no action icon clicked, wait to see if it's a double click. if a double-click
		//	 * event notification is received within the timeout, the widget will respond as if
		//	 * the user double-clicked; if the timer times out, the widget will respond as if a
		//	 * single-click was received. this means that for single-clicks there is a short
		//	 * delay before action is taken, which is why the action icons are checked before
		//	 * starting the double-click checking timer - so that clicks on the icons don't feel
		//	 * unresponsive */
		//		if(m_doubleClickWaitTimer.isActive() || m_receivedDoubleClickEvent) {
		//			m_receivedDoubleClickEvent = false;
		//			QListView::mouseReleaseEvent(ev);
		//			return;
		//		}

		//		m_doubleClickWaitTimer.start();
		//		ev->accept();
		//		QListView::mouseReleaseEvent(ev);
		//		return;
	}


	void OtpListView::mouseDoubleClickEvent(QMouseEvent * ev) {
		m_doubleClickWaitTimer.stop();
		m_receivedDoubleClickEvent = true;

		if(m_removeIconHitRect.contains(ev->pos()) || m_refreshIconHitRect.contains(ev->pos()) || m_copyIconHitRect.contains(ev->pos())) {
			QListView::mouseDoubleClickEvent(ev);
			return;
		}

		auto * otp = qonvinceApp->otp(m_hoverItemIndex);

		if(!otp) {
			QListView::mouseDoubleClickEvent(ev);
			return;
		}

		Q_EMIT codeDoubleClicked(otp);
	}


	void OtpListView::mouseClickEvent(QMouseEvent * ev) {
		// this "fake" mouseClickEvent method is only called if the user clicked
		// on an item (but not one of its action icons) and it didn't turn out
		// to be a double-click */
		if(Qt::LeftButton == ev->button()) {
			auto itemIndex = indexAt(ev->pos());

			if(!itemIndex.isValid()) {
				return;
			}

			auto * otp = itemIndex.data(OtpListModel::OtpRole).value<Otp *>();

			if(!otp) {
				return;
			}

			Q_EMIT codeClicked(otp);
		}
	}


	void OtpListView::contextMenuEvent(QContextMenuEvent * ev) {
		m_actionItemIndex = indexAt(ev->pos());

		if(!m_actionItemIndex.isValid()) {
			return;
		}

		ev->accept();
		m_itemContextMenu.move(ev->globalPos());
		m_itemContextMenu.show();
	}


	void OtpListView::onEditActionTriggered() {
		if(!m_actionItemIndex.isValid()) {
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();
		m_actionItemIndex = {};

		if(!otp) {
			return;
		}

		Q_EMIT editCodeRequested(otp);
	}


	void OtpListView::onCopyActionTriggered() {
		if(!m_actionItemIndex.isValid()) {
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();
		m_actionItemIndex = {};

		if(!otp) {
			return;
		}

		QApplication::clipboard()->setText(otp->code());
	}


	void OtpListView::onRemoveIconActionTriggered() {
		if(!m_actionItemIndex.isValid()) {
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();
		m_actionItemIndex = {};

		if(!otp) {
			return;
		}

		otp->setIcon({});
	}


	void OtpListView::onRefreshActionTriggered() {
		if(!m_actionItemIndex.isValid()) {
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();
		m_actionItemIndex = {};

		if(!otp) {
			return;
		}

		otp->refreshCode();
	}


	void OtpListView::onRemoveActionTriggered() {
		m_actionItemIndex = {};

		if(!m_actionItemIndex.isValid()) {
			return;
		}

		m_actionItemIndex = {};
		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();

		if(!otp) {
			return;
		}

		if(QMessageBox::Yes == QMessageBox::question(this, tr("%1: Remove code").arg(qApp->applicationName()), tr("Are you sure you wish to remove this code?"), QMessageBox::Yes | QMessageBox::No)) {
			qonvinceApp->removeOtp(otp);

			/* reset the hover item in case it's the one we've just deleted
		 * it will be updated on the next mouse move event we receive */
			m_mousePressLeftStart = {};
			m_removeIconHitRect = m_refreshIconHitRect = {};
			m_hoverItemIndex = -1;
			return;
		}
	}


	void OtpListView::paintEvent(QPaintEvent * ev) {
		ev->accept();

		if(QONVINCE_OTPCODELISTWIDGET_DO_BACKGROUND_MESSAGE && m_imageDropEnabled) {
			QPainter painter(viewport());
			int y = height() - QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_RECT_HEIGHT;
			int w = viewport()->width();
			painter.setPen(palette().text().color().lighter());
			QFont bgFont = font();
			bgFont.setPixelSize(QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_FONT_SIZE);
			painter.setFont(bgFont);
			painter.drawText(0, y, w, QONVINCE_OTPCODELISTWIDGET_BACKGROUND_MESSAGE_RECT_HEIGHT, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignCenter, tr("Drop QR code images here ..."));
		}

		QListView::paintEvent(ev);
	}


#if !defined(NDEBUG)
	void OtpListView::debugLogNewCode(const QString & code) const {
		auto * otp = qobject_cast<Otp *>(sender());

		if(otp) {
			std::cout << "Otp object \"" << otp->name() << "\" generated new code \"" << code << "\"\n"
						 << std::flush;
		}
	}
#endif


}  // namespace Qonvince
