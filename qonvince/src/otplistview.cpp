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
  * \brief Implementation of the OtpListView class.
  *
  * \todo use unobtrusive notification for OTP removal confirmation
  */

#include "otplistview.h"

#include <QDebug>
#include <QListView>
#include <QApplication>
#include <QMenu>
#include <QClipboard>
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


namespace Qonvince {

	namespace Detail {
		namespace OtpListView {
			static constexpr const int SpacingSize = 4;
			static constexpr const int BackgroundTextSize = 20;
			static constexpr const int BackgroundTextVerticalOffset = 40;
			static constexpr const int ActionIconExtent = 22;
			static constexpr const QSize ActionIconSize = {ActionIconExtent, ActionIconExtent};
			static constexpr const int ActionIconHoverRectRounding = 3;
		}  // namespace OtpListView
	}		// namespace Detail


	OtpListView::OtpListView(QWidget * parent)
	: QListView(parent),
	  m_tickTimerIsResynchronising(false),
	  m_imageDropEnabled(OtpQrCodeReader::isAvailable()),
	  m_tickTimerId(-1),
	  m_doubleClickWaitTimer(),
	  m_receivedDoubleClickEvent(false),
	  m_itemContextMenu(),
	  m_model(std::make_unique<OtpListModel>()),
	  m_delegate(std::make_unique<OtpListItemDelegate>()),
	  m_copy({QIcon::fromTheme("edit-copy", QIcon(":/icons/codeactions/copy")), {{0, 0}, Detail::OtpListView::ActionIconSize}}),
	  m_refresh({QIcon::fromTheme("view-refresh", QIcon(":/icons/codeactions/refresh")), {{Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize, 0}, Detail::OtpListView::ActionIconSize}}),
	  m_remove({QIcon::fromTheme("list-remove", QIcon(":/icons/codeactions/remove")), {{2 * (Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize), 0}, Detail::OtpListView::ActionIconSize}}),
	  m_reveal({QIcon(":/icons/codeactions/reveal"), {{3 * (Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize), 0}, Detail::OtpListView::ActionIconSize}}),
	  m_actionIconHoverRect(),
	  m_actionIconMouseClickStartRect() {
		m_delegate->setCountdownWarningColour(QColor(160, 160, 92));
		m_delegate->setCountdownCriticalColour(QColor(220, 78, 92));
		m_delegate->setActionIconAreaWidth(4 * (Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize) + Detail::OtpListView::SpacingSize);
		QListView::setModel(m_model.get());
		QListView::setItemDelegate(m_delegate.get());

		m_itemContextMenu.addAction(QIcon::fromTheme("document-edit"), tr("Edit"), this, &OtpListView::onEditActionTriggered);
		m_itemContextMenu.addAction(tr("Remove icon"), this, &OtpListView::onRemoveIconActionTriggered);
		m_itemContextMenu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &OtpListView::onCopyActionTriggered);
		m_itemContextMenu.addAction(QIcon::fromTheme("view-refresh", QIcon(":/icons/codeactions/refresh")), tr("Refresh now"), this, &OtpListView::onRefreshActionTriggered);
		m_itemContextMenu.addAction(QIcon::fromTheme("list-remove", QIcon(":/icons/codeactions/remove")), tr("Remove"), this, &OtpListView::onRemoveActionTriggered);

		m_doubleClickWaitTimer.setInterval(qonvinceApp->styleHints()->mouseDoubleClickInterval());
		m_doubleClickWaitTimer.setSingleShot(true);

		connect(&m_doubleClickWaitTimer, &QTimer::timeout, [this]() {
			if(m_receivedDoubleClickEvent) {
				m_receivedDoubleClickEvent = false;
			}
			else {
				QMouseEvent ev(QMouseEvent::MouseButtonRelease, mapFromGlobal(QCursor::pos()), Qt::LeftButton, Qt::LeftButton, 0);
				mouseClickEvent(&ev);
			}

			m_mousePressItemIndex = {};
		});

		connect(&(qonvinceApp->settings()), qOverload<Settings::CodeLabelDisplayStyle>(&Settings::codeLabelDisplayStyleChanged), this, qOverload<>(&OtpListView::update));
		synchroniseTickTimer();
	}


	int OtpListView::hoveredOtpIndex() const {
		auto index = indexAt(mapFromGlobal(QCursor::pos()));

		if(!index.isValid()) {
			return -1;
		}

		return index.row();
	}


	OtpListView::~OtpListView() = default;


	void OtpListView::synchroniseTickTimer() {
		if(-1 != m_tickTimerId) {
			killTimer(m_tickTimerId);
		}

		m_tickTimerIsResynchronising = true;

		// sync just after a 1sec boundary so that the timer doesn't tick down
		// to 0 before the code objects generate their new code and emit their
		// signals - effectively this gives the codes a 0.05s window to run the
		// code to actually generate the new code */
		// TODO could we obviate the need for this by synchronising both this
		// and the code objects to a global sync timer at application start?
		m_tickTimerId = startTimer(50 + (QDateTime::currentMSecsSinceEpoch() % 1000), Qt::PreciseTimer);
	}


	Otp * OtpListView::hoveredOtp() const {
		auto otpIndex = hoveredOtpIndex();

		if(0 > otpIndex) {
			return nullptr;
		}

		return qonvinceApp->otp(otpIndex);
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
			auto globalMousePos = QCursor::pos();
			auto widgetMousePos = mapFromGlobal(globalMousePos);
			QString tooltipTxt = QStringLiteral();

			if(m_copy.geometry.contains(widgetMousePos)) {
				tooltipTxt = tr("Copy the current code to the clipboard.");
			}
			else if(m_refresh.geometry.contains(widgetMousePos)) {
				tooltipTxt = tr("Refresh the code now.");
			}
			else if(m_remove.geometry.contains(widgetMousePos)) {
				tooltipTxt = tr("Remove this OTP from %1.").arg(QApplication::applicationDisplayName());
			}
			else if(m_reveal.geometry.contains(widgetMousePos)) {
				tooltipTxt = tr("Reveal the current code.");
			}
			else {
				auto index = indexAt(widgetMousePos);

				if(index.isValid()) {
					tooltipTxt = tr("Double-click to edit.");

					if(qonvinceApp->settings().copyCodeOnClick()) {
						tooltipTxt = QStringLiteral("<html><body><p>") % tooltipTxt % QStringLiteral("</p><p>") % tr("Click to copy the current code to the clipboard.") % QStringLiteral("</p></body></html>");
					}
				}
			}

			if(!tooltipTxt.isEmpty()) {
				QToolTip::showText(globalMousePos, tooltipTxt, this);
				ev->accept();
				return true;
			}
		}

		return QListView::event(ev);
	}


	void OtpListView::resizeEvent(QResizeEvent * ev) {
		m_actionIconMouseClickStartRect = {};
		m_actionIconHoverRect = {};
		QListView::resizeEvent(ev);
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
		setMouseTracking(false);
		m_actionItemIndex = {};
	}


	void OtpListView::mouseMoveEvent(QMouseEvent * ev) {
		auto mousePos = ev->pos();

		if(m_copy.geometry.contains(mousePos)) {
			if(m_actionIconHoverRect != m_copy.geometry) {
				QRegion damage(m_actionIconHoverRect);
				damage += m_copy.geometry;
				m_actionIconHoverRect = m_copy.geometry;
				viewport()->update(damage);
			}
		}
		else if(m_refresh.geometry.contains(mousePos)) {
			if(m_actionIconHoverRect != m_refresh.geometry) {
				QRegion damage(m_actionIconHoverRect);
				damage += m_refresh.geometry;
				m_actionIconHoverRect = m_refresh.geometry;
				viewport()->update(damage);
			}
		}
		else if(m_remove.geometry.contains(mousePos)) {
			if(m_actionIconHoverRect != m_remove.geometry) {
				QRegion damage(m_actionIconHoverRect);
				damage += m_remove.geometry;
				m_actionIconHoverRect = m_remove.geometry;
				viewport()->update(damage);
			}
		}
		else if(!m_reveal.geometry.isNull() && m_reveal.geometry.contains(mousePos)) {
			if(m_actionIconHoverRect != m_reveal.geometry) {
				QRegion damage(m_actionIconHoverRect);
				damage += m_reveal.geometry;
				m_actionIconHoverRect = m_reveal.geometry;
				viewport()->update(damage);
			}
		}
		else if(!m_actionIconHoverRect.isNull()) {
			QRegion damage(m_actionIconHoverRect);
			m_actionIconHoverRect = {};
			viewport()->update(damage);
		}

		onItemEntered(indexAt(mousePos));
		ev->accept();
		QListView::mouseMoveEvent(ev);
	}


	void OtpListView::keyReleaseEvent(QKeyEvent * ev) {
		if(ev->matches(QKeySequence::Copy)) {
			auto otpIndex = indexAt(mapFromGlobal(QCursor::pos()));

			if(!otpIndex.isValid()) {
				return;
			}

			QApplication::clipboard()->setText(otpIndex.data(OtpListModel::CodeRole).toString());
		}
	}


	void OtpListView::mousePressEvent(QMouseEvent * ev) {
		if(Qt::LeftButton == ev->button()) {
			auto mousePos = ev->pos();
			m_mousePressItemIndex = indexAt(mousePos);
			ev->accept();

			// mouseMoveEvent() tracks which icon is hovered, and we can't
			// click on an icon unless it's hovered, so we can take advantage
			// of the work done in mouseMoveEvent and only check whether the
			// click is within the hovered rect - we don't need to check any
			// others because we know they can't be clicked
			if(!m_actionIconHoverRect.isNull() && m_actionIconHoverRect.contains(mousePos)) {
				m_actionIconMouseClickStartRect = m_actionIconHoverRect;
				ev->accept();
				return;
			}
			else {
				m_actionIconMouseClickStartRect = {};
			}
		}

		QListView::mousePressEvent(ev);
		return;
	}


	void OtpListView::mouseReleaseEvent(QMouseEvent * ev) {
		if(Qt::LeftButton == ev->button()) {
			if(!m_actionIconMouseClickStartRect.isNull()) {
				auto mouseReleasePos = ev->pos();

				if(m_actionIconMouseClickStartRect.contains(mouseReleasePos)) {
					// could check for equality between click rect and icon rect, but
					// contains is likely to be marginally quicker
					if(m_copy.geometry.contains(mouseReleasePos)) {
						onCopyActionTriggered();
					}
					else if(m_refresh.geometry.contains(mouseReleasePos)) {
						onRefreshActionTriggered();
					}
					else if(m_remove.geometry.contains(mouseReleasePos)) {
						onRemoveActionTriggered();
					}
					else if(!m_reveal.geometry.isNull() && m_reveal.geometry.contains(mouseReleasePos)) {
						onRevealActionTriggered();
					}

					ev->accept();
					return;
				}

				m_actionIconMouseClickStartRect = {};
			}

			if(m_mousePressItemIndex.isValid()) {
				m_doubleClickWaitTimer.start();
				ev->accept();
			}
		}

		QListView::mouseReleaseEvent(ev);
		return;
	}


	void OtpListView::mouseDoubleClickEvent(QMouseEvent * ev) {
		m_doubleClickWaitTimer.stop();
		m_receivedDoubleClickEvent = true;
		QListView::mouseDoubleClickEvent(ev);

		auto * otp = hoveredOtp();

		if(otp) {
			Q_EMIT codeDoubleClicked(otp);
			ev->accept();
			return;
		}

		QListView::mouseDoubleClickEvent(ev);
	}


	void OtpListView::mouseClickEvent(QMouseEvent * ev) {
		// this "fake" mouseClickEvent method is only called if the user clicked
		// on an item (but not one of its action icons) and it didn't turn out
		// to be a double-click
		if(Qt::LeftButton == ev->button() && m_mousePressItemIndex.isValid()) {
			auto * otp = m_mousePressItemIndex.data(OtpListModel::OtpRole).value<Otp *>();

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

		if(!otp) {
			return;
		}

		QApplication::clipboard()->setText(otp->code());
	}


	void OtpListView::onItemEntered(const QModelIndex & index) {
		if(!index.isValid()) {
			m_actionItemIndex = {};
			return;
		}

		auto itemRect = visualRect(index);
		auto actionIconTopLeft = QPoint(itemRect.right() - (4 * (Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize)), itemRect.top() + ((itemRect.height() - Detail::OtpListView::ActionIconExtent) / 2));
		auto & actionIconLeft = actionIconTopLeft.rx();

		auto * otp = index.data(OtpListModel::OtpRole).value<Otp *>();

		if(otp && otp->revealCodeOnDemand()) {
			m_reveal.geometry = {actionIconTopLeft, Detail::OtpListView::ActionIconSize};
		}
		else {
			m_reveal.geometry = {};
		}

		actionIconLeft += Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize;
		m_copy.geometry.moveTopLeft(actionIconTopLeft);

		actionIconLeft += Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize;
		m_refresh.geometry.moveTopLeft(actionIconTopLeft);

		actionIconLeft += Detail::OtpListView::ActionIconExtent + Detail::OtpListView::SpacingSize;
		m_remove.geometry.moveTopLeft(actionIconTopLeft);

		m_actionItemIndex = index;
	}


	void OtpListView::onRevealActionTriggered() {
		if(!m_actionItemIndex.isValid()) {
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();

		if(!otp) {
			return;
		}

		if(otp->codeIsVisible()) {
			return;
		}

		otp->reveal();
		QTimer::singleShot(1000 * qonvinceApp->settings().codeRevealTimeout(), otp, &Otp::hide);
	}


	void OtpListView::onRefreshActionTriggered() {
		if(!m_actionItemIndex.isValid()) {
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();

		if(!otp) {
			return;
		}

		otp->refreshCode();
	}


	void OtpListView::onRemoveActionTriggered() {
		if(!m_actionItemIndex.isValid()) {
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();

		if(!otp) {
			return;
		}

		if(QMessageBox::Yes == QMessageBox::question(this, tr("%1: Remove code").arg(qApp->applicationName()), tr("Are you sure you wish to remove this code?"), QMessageBox::Yes | QMessageBox::No)) {
			qonvinceApp->removeOtp(otp);
			return;
		}
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


	void OtpListView::paintEvent(QPaintEvent * ev) {
		if(m_imageDropEnabled) {
			QPainter painter(viewport());
			int y = height() - Detail::OtpListView::BackgroundTextVerticalOffset;
			int w = viewport()->width();
			painter.setPen(palette().text().color().lighter());
			QFont bgFont = font();
			bgFont.setPixelSize(Detail::OtpListView::BackgroundTextSize);
			bgFont.setBold(true);
			painter.setFont(bgFont);
			painter.drawText(0, y, w, Detail::OtpListView::BackgroundTextVerticalOffset, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignCenter, tr("Drop QR code images here ..."));
		}

		QListView::paintEvent(ev);

		// render the action icon overlay
		if(m_actionItemIndex.isValid()) {
			QPainter painter(viewport());
			painter.setRenderHint(QPainter::Antialiasing, true);

			if(!m_actionIconHoverRect.isNull()) {
				painter.setPen(Qt::transparent);
				painter.setBrush(palette().highlight().color().darker(125));
				painter.drawRoundedRect(m_actionIconHoverRect, Detail::OtpListView::ActionIconHoverRectRounding, Detail::OtpListView::ActionIconHoverRectRounding);
			}

			m_copy.icon.paint(&painter, m_copy.geometry);
			m_refresh.icon.paint(&painter, m_refresh.geometry);
			m_remove.icon.paint(&painter, m_remove.geometry);

			if(!m_reveal.geometry.isNull()) {
				m_reveal.icon.paint(&painter, m_reveal.geometry);
			}
		}
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
