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
  * \todo work out how to make action widget stop stealing "hover" focus
  * from item
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
#include "otplistitemactionbuttons.h"
#include "otpqrcodereader.h"
#include "qtiostream.h"


namespace Qonvince {

	namespace Detail {
		namespace OtpListView {
			static constexpr const int SpacingSize = 4;
			static constexpr const int BackgroundTextSize = 20;
			static constexpr const int BackgroundTextVerticalOffset = 40;
			static constexpr const int ItemActionsHeight = 22;
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
	  m_actionButtons(std::make_unique<OtpListItemActionButtons>(this)) {
		m_delegate->setCountdownWarningColour(QColor(160, 160, 92));
		m_delegate->setCountdownCriticalColour(QColor(220, 78, 92));
		QListView::setModel(m_model.get());
		QListView::setItemDelegate(m_delegate.get());

		m_actionButtons->setVisible(false);
		m_actionButtons->setFocusPolicy(Qt::NoFocus);

		connect(m_actionButtons.get(), &OtpListItemActionButtons::copyClicked, this, &OtpListView::onCopyActionTriggered);
		connect(m_actionButtons.get(), &OtpListItemActionButtons::refreshClicked, this, &OtpListView::onRefreshActionTriggered);
		connect(m_actionButtons.get(), &OtpListItemActionButtons::removeClicked, this, &OtpListView::onRemoveActionTriggered);

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
		connect(this, &QListView::entered, this, &OtpListView::onItemEntered);

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
		auto index = hoveredOtpIndex();

		if(0 > index) {
			return nullptr;
		}

		return qonvinceApp->otp(index);
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
			auto index = indexAt(widgetMousePos);

			if(index.isValid()) {
				QString txt;
				QString label = index.data(OtpListModel::LabelRole).toString();

				if(m_revealIconHitRect.contains(widgetMousePos)) {
					txt = tr("Reveal the current code.");
				}
				else {
					txt = tr("Double-click to edit.");

					if(qonvinceApp->settings().copyCodeOnClick()) {
						txt = QStringLiteral("<html><body><p>") % txt % QStringLiteral("</p><p>") % tr("Click to copy the current code to the clipboard.") % QStringLiteral("</p></body></html>");
					}
				}

				if(!txt.isEmpty()) {
					QToolTip::showText(globalMousePos, txt, this);
					ev->accept();
					return true;
				}
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
		setMouseTracking(false);
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
			m_mousePressItemIndex = indexAt(ev->pos());
			ev->accept();
		}

		QListView::mousePressEvent(ev);
		return;
	}


	void OtpListView::mouseReleaseEvent(QMouseEvent * ev) {
		if(Qt::LeftButton == ev->button()) {
			if(m_mousePressItemIndex.isValid()) {
				m_doubleClickWaitTimer.start();
				ev->accept();
			}
		}

		QListView::mouseReleaseEvent(ev);
		return;
	}


	void OtpListView::mouseDoubleClickEvent(QMouseEvent * ev) {
		std::cout << "received double-click event\n"
					 << std::flush;
		m_doubleClickWaitTimer.stop();
		m_receivedDoubleClickEvent = true;
		QListView::mouseDoubleClickEvent(ev);

		auto * otp = hoveredOtp();

		if(otp) {
			std::cout << "emitting codeDoubleClicked signal\n"
						 << std::flush;
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
		std::cout << __PRETTY_FUNCTION__ << ": item # " << index.row() << "\n"
					 << std::flush;
		auto rect = visualRect(index);
		std::cout << "item geometry: " << rect << "\n"
					 << std::flush;
		rect.setTopLeft({rect.right() - (3 * (Detail::OtpListView::ItemActionsHeight + Detail::OtpListView::SpacingSize)), rect.top() + ((rect.height() - Detail::OtpListView::ItemActionsHeight) / 2)});
		rect.setSize({rect.width() - Detail::OtpListView::SpacingSize, Detail::OtpListView::ItemActionsHeight});
		std::cout << "action buttons geometry: " << rect << "\n"
					 << std::flush;
		m_actionButtons->setGeometry(rect);
		m_actionButtons->setVisible(true);
		m_actionItemIndex = index;
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
			std::cerr << "action item index is not valid\n"
						 << std::flush;
			return;
		}

		auto * otp = m_actionItemIndex.data(OtpListModel::OtpRole).value<Otp *>();

		if(!otp) {
			std::cerr << "failed to retrieve OTP from model\n"
						 << std::flush;
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
		ev->accept();

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
