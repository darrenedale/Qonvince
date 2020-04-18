/*
 * Copyright 2015 - 2020 Darren Edale
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

#ifndef QONVINCE_OTPLISTVIEW_H
#define QONVINCE_OTPLISTVIEW_H

#include <vector>

#include <QListView>
#include <QPushButton>
#include <QMenu>
#include <QColor>
#include <QHash>
#include <QBasicTimer>
#include <QTimer>

#include "otp.h"
#include "otplistmodel.h"
#include "otplistitemdelegate.h"

class QEvent;
class QPaintEvent;
class QMouseEvent;
class QKeyEvent;
class QContextMenuEvent;

namespace Qonvince {

	class OtpListView
	: public QListView {
		Q_OBJECT

	public:
		explicit OtpListView(QWidget * = nullptr);
		~OtpListView() override;

		int hoveredOtpIndex() const;
		Otp * hoveredOtp() const;

		inline QString hoveredOtpCode() const {
			auto * otp = hoveredOtp();

			if(!otp) {
				return {};
			}

			return otp->code();
		}

		inline int selectedOtpIndex() const {
			return hoveredOtpIndex();
		}

		inline Otp * selectedOtp() const {
			return hoveredOtp();
		}

		inline QString selectedOtpCode() const {
			return hoveredOtpCode();
		}

		constexpr inline int itemHeight() const {
			return 40;
		}

		inline void setModel(QAbstractItemModel *) override {
		}

		void setItemDelegate() = delete;

	Q_SIGNALS:
		void codeClicked(Otp *);
		void codeDoubleClicked(Otp *);
		void editCodeRequested(Otp *);

	protected:
		bool event(QEvent * event) override;
		void resizeEvent(QResizeEvent * event) override;
		void timerEvent(QTimerEvent * event) override;
		void enterEvent(QEvent * event) override;
		void leaveEvent(QEvent * event) override;
		void mouseMoveEvent(QMouseEvent * event) override;
		void mousePressEvent(QMouseEvent * event) override;
		void mouseReleaseEvent(QMouseEvent * event) override;
		void mouseDoubleClickEvent(QMouseEvent * event) override;
		void paintEvent(QPaintEvent * event) override;
		void contextMenuEvent(QContextMenuEvent * event) override;
		void keyReleaseEvent(QKeyEvent * event) override;

		// this is not a Qt event method, it's one we've synthesised by
		// filtering out cases where the click is part of a double-click
		virtual void mouseClickEvent(QMouseEvent * event);

		void synchroniseTickTimer();

	private Q_SLOTS:
		//		void onOtpChanged();
		void updateCountdowns();

		void onEditActionTriggered();
		void onRefreshActionTriggered();
		void onRevealActionTriggered();
		void onRemoveActionTriggered();
		void onCopyActionTriggered();
		void onRemoveIconActionTriggered();
		void onItemEntered(const QModelIndex &);

	private:
		bool m_tickTimerIsResynchronising;
		bool m_imageDropEnabled;

		// triggers widget redraw on TOTP timer ticks (i.e. 1s boundaries)
		int m_tickTimerId;

		// inserts a delay between receiving a mouseReleaseEvent() that looks like a click
		// on a code and actually acting on a click so that we can determine whether it's
		// actually a double-click. the timer is started by the mouseReleaseEvent() and
		// stopped either by the mouseDoubleClickEvent() or the mouseClickEvent(). any
		// mouseReleaseEvent()s that occur while the timer is running are ignored. The flag
		// is set if a double-click occurred so that if, as on X, the mouseReleseEvent() for
		// the second click of a double-click occurs after the doubleClickEvent(), the
		// mouseClickEvent() is not called as well as the mouseDoubleClickEvent().
		QTimer m_doubleClickWaitTimer;
		bool m_receivedDoubleClickEvent;

		QMenu m_itemContextMenu;
		QModelIndex m_actionItemIndex;
		QModelIndex m_mousePressItemIndex;

		std::unique_ptr<OtpListModel> m_model;
		std::unique_ptr<OtpListItemDelegate> m_delegate;

		struct ActionButtonSpec {
			QIcon icon;
			QRect geometry;
		};

		ActionButtonSpec m_copy;
		ActionButtonSpec m_refresh;
		ActionButtonSpec m_remove;
		ActionButtonSpec m_reveal;

		QRect m_actionIconHoverRect;
		QRect m_actionIconMouseClickStartRect;
	};

}  // namespace Qonvince

#endif  // QONVINCE_OTPLISTVIEW_H
