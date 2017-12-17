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

#ifndef QUICKAUTH_OTPLISTWIDGET_H
#define QUICKAUTH_OTPLISTWIDGET_H

#include <vector>

#include <QListWidget>
#include <QMenu>
#include <QColor>
#include <QHash>
#include <QBasicTimer>
#include <QTimer>

#include "otp.h"

class QEvent;
class QPaintEvent;
class QMouseEvent;
class QKeyEvent;
class QContextMenuEvent;

namespace Qonvince {

	class OtpListWidgetItem;

	class OtpListWidget
	: public QListWidget {
		Q_OBJECT

	public:
		explicit OtpListWidget(QWidget * = nullptr);
		virtual ~OtpListWidget();

		inline int hoveredOtpIndex() const {
			return m_hoverItemIndex;
		}

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

		void addItem(OtpListWidgetItem *);

		constexpr inline int itemHeight() const {
			return 40;
		}

		inline const QColor & countdownColour() const {
			return m_countdownColour;
		}

		inline const QColor & countdownWarningColour() const {
			return m_countdownWarningColour;
		}

		inline const QColor & countdownCriticalColour() const {
			return m_countdownCriticalColour;
		}

		Otp * otp(int i) const;
		void addOtp(std::unique_ptr<Otp> &&);

	public Q_SLOTS:
		void setCountdownColour(const QColor &);
		void setCountdownWarningColour(const QColor &);
		void setCountdownCriticalColour(const QColor &);
//		void addOtp(const QByteArray & seed);
//		void addOtp(const QString & name, const QByteArray & seed);

	Q_SIGNALS:
		//		void codeAdded(Otp *);
		//		void codeRemoved(Otp *);
		void codeClicked(Otp *);
		void codeDoubleClicked(Otp *);
		void editCodeRequested(Otp *);

	protected:
		virtual bool event(QEvent *) override;
		virtual void timerEvent(QTimerEvent *) override;
		virtual void enterEvent(QEvent *) override;
		virtual void leaveEvent(QEvent *) override;
		virtual void mouseMoveEvent(QMouseEvent *) override;
		virtual void mousePressEvent(QMouseEvent *) override;
		virtual void mouseReleaseEvent(QMouseEvent *) override;
		virtual void mouseDoubleClickEvent(QMouseEvent *) override;
		virtual void paintEvent(QPaintEvent *) override;
		virtual void contextMenuEvent(QContextMenuEvent *) override;
		virtual void keyReleaseEvent(QKeyEvent *) override;

		/* this is not a Qt event method, it's one we've synthesised by
		 * filtering out cases where the click is part of a double-click */
		virtual void mouseClickEvent(QMouseEvent *);

		OtpListWidgetItem * findOtp(const Otp *) const;
		void synchroniseTickTimer();

	private Q_SLOTS:
		void onOtpChanged();
		void updateCountdowns();

		void onEditActionTriggered();
		void onRefreshActionTriggered();
		void onRemoveActionTriggered();
		void onCopyActionTriggered();
		void onRemoveIconActionTriggered();

#ifndef NDEBUG
		void debugLogNewCode(const QString &) const;
#endif

	private:
		// index of item under mouse pointer
		int m_hoverItemIndex;

		QColor m_countdownColour;
		QColor m_countdownWarningColour;
		QColor m_countdownCriticalColour;

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

		// hit-test geometry for item under mouse pointer
		QRect m_removeIconHitRect, m_refreshIconHitRect, m_copyIconHitRect, m_revealIconHitRect;
		QPoint m_mousePressLeftStart;

		// item context menu
		QMenu m_itemMenu;

		// item whose context-menu is visible
		OtpListWidgetItem * m_menuCodeItem;

		// list of hidden passcodes that are currently revealed temporarily
		std::vector<Otp *> m_revealedPasscodes;
	};

}  // namespace Qonvince

#endif  // QUICKAUTH_OTPLISTWIDGET_H
