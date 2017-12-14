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

#include <QListWidget>
#include <QColor>
#include <QHash>

#include "otp.h"

class QEvent;
class QPaintEvent;
class QMouseEvent;
class QKeyEvent;
class QContextMenuEvent;
class QTimer;

namespace Qonvince {

	class OtpListWidgetItem;

	class OtpListWidget
	: public QListWidget {
		Q_OBJECT

	public:
		explicit OtpListWidget(QWidget * parent = nullptr);
		virtual ~OtpListWidget();

		inline int hoveredCodeIndex() const {
			return m_hoverItemIndex;
		}

		Otp * hoveredCodeSpecification() const;

		inline QString hoveredCode() const {
			Otp * code = hoveredCodeSpecification();

			if(!code) {
				return QString();
			}

			return code->code();
		}

		inline int selectedCodeIndex() const {
			return hoveredCodeIndex();
		}

		inline Otp * selectedCodeSpecification() const {
			return hoveredCodeSpecification();
		}

		inline QString selectedCode() const {
			return hoveredCode();
		}

		void addItem(Otp * code);
		void addItem(OtpListWidgetItem * item);

		inline int itemHeight() const {
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

		Otp * code(int i) const;

	public Q_SLOTS:
		void setCountdownColour(const QColor & c);
		void setCountdownWarningColour(const QColor & c);
		void setCountdownCriticalColour(const QColor & c);
		void addCode(const QByteArray & seed);
		void addCode(const QString & name, const QByteArray & seed);
		void addCode(Otp * code);

		/* only for dev purposes, to log when settings changes update the widget */
		void onSettingsChanged();

	Q_SIGNALS:
		void codeAdded(Otp * code);
		void codeRemoved(Otp * code);
		void codeClicked(Otp * code);
		void codeDoubleClicked(Otp * code);
		void editCodeRequested(Otp * code);

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

#ifndef NDEBUG
		virtual void showEvent(QShowEvent *) override;
#endif

		/* this is not a Qt event method, it's one we've synthesised by
			 * filtering out cases where the click is part of a double-click */
		virtual void mouseClickEvent(QMouseEvent * ev);

		OtpListWidgetItem * itemFromOtp(const Otp * code) const;
		void synchroniseTickTimer();

	private Q_SLOTS:
		void onCodeDefinitionChanged();
		void onCodeDefinitionChanged(Otp * code);
		void onCodeChanged();
		void onCodeChanged(Otp * code);
		void updateCountdowns();
		void emitCodeDoubleClicked(QListWidgetItem * it);
		void emitCodeClicked(QListWidgetItem * it);

		void onEditActionTriggered();
		void onRefreshActionTriggered();
		void onRemoveActionTriggered();
		void onCopyActionTriggered();
		void onRemoveIconActionTriggered();

#if defined(QT_DEBUG)
		void debugLogNewCode(const QString & code) const;
#endif

	private:
		void callMouseClickEvent();

		int m_hoverItemIndex;

		QColor m_countdownColour;
		QColor m_countdownWarningColour;
		QColor m_countdownCriticalColour;

		bool m_tickResync;
		bool m_imageDropEnabled;
		QBasicTimer * m_tickTimer;

		/* inserts a delay between receiving a mouseReleaseEvent() that looks like a click
			 * on a code and actually acting on a click so that we can determine whether it's
			 * actually a double-click. the timer is started by the mouseReleaseEvent() and
			 * stopped either by the mouseDoubleClickEvent() or the mouseClickEvent(). any
			 * mouseReleaseEvent()s that occur while the timer is running are ignored. The flag
			 * is set if a double-click occurred so that if, as on X, the mouseReleseEvent() for
			 * the second click of a double-click occurs after the doubleClickEvent(), the
			 * mouseClickEvent() is not called as well as the mouseDoubleClickEvent().
			 */
		QTimer * m_doubleClickWaitTimer;
		bool m_receivedDoubleClickEvent;

		QRect m_removeIconHitRect, m_refreshIconHitRect, m_copyIconHitRect, m_revealIconHitRect;
		QPoint m_mousePressLeftStart;

		QMenu * m_itemMenu;
		OtpListWidgetItem * m_menuCodeItem;

		/* list of hidden passcodes that are currently, temporarily revealed */
		QList<Otp *> m_revealedPasscodes;
	};

}  // namespace Qonvince

#endif  // QUICKAUTH_OTPLISTWIDGET_H
