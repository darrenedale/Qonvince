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

#ifndef QONVINCE_SETTINGS_H
#define QONVINCE_SETTINGS_H

#include <QObject>

class QSettings;

namespace Qonvince {
	class Settings
	: public QObject {
		Q_OBJECT

	public:
		enum CodeLabelDisplayStyle {
			IssuerAndName = 0,
			NameOnly,
			IssuerOnly
		};

		explicit Settings(QObject * parent = nullptr);
		Settings(const Settings &) = delete;
		Settings(Settings &&) = delete;
		void operator=(const Settings &) = delete;
		void operator=(Settings &&) = delete;

		inline bool singleInstance() const {
			return m_singleInstance;
		}

		inline bool quitOnMainWindowClosed() const {
			return m_quitOnMainWindowClosed;
		}

		inline bool startMinimised() const {
			return m_startMinimised;
		}

		inline bool copyCodeOnClick() const {
			return m_copyCodeOnClick;
		}

		// if codes are copied on click, does the user also want
		// to hide the app when this happens?
		inline bool hideOnCodeCopyClick() const {
			return m_hideOnCodeCopyClick;
		}

		// when a code is copied to the clipboard, clear the clipboard
		// after a specified interval
		inline bool clearClipboardAfterInterval() const {
			return m_clearClipboardAfterInterval;
		}

		// how many seconds to wait after a copy before clearing the
		// clipboard
		inline int clipboardClearInterval() const {
			return m_clipboardClearInterval;
		}

		inline CodeLabelDisplayStyle codeLabelDisplayStyle() const {
			return m_codeLabelDisplayStyle;
		}

		inline int codeRevealTimeout() const {
			return m_revealTimeout;
		}

		void read(const QSettings & settings);
		void write(QSettings & settings) const;

	Q_SIGNALS:
		void changed();
		void singleInstanceChanged(bool newValue);
		void singleInstanceChanged(bool oldValue, bool newValue);
		void quitOnMainWindowClosedChanged(bool newValue);
		void quitOnMainWindowClosedChanged(bool oldValue, bool newValue);
		void startMinimisedChanged(bool newValue);
		void startMinimisedChanged(bool oldValue, bool newValue);
		void copyCodeOnClickChanged(bool newValue);
		void copyCodeOnClickChanged(bool oldValue, bool newValue);
		void hideOnCodeCopyClickChanged(bool newValue);
		void hideOnCodeCopyClickChanged(bool oldValue, bool newValue);
		void clearClipboardAfterIntervalChanged(bool newValue);
		void clearClipboardAfterIntervalChanged(bool oldValue, bool newValue);
		void clipboardClearIntervalChanged(int newValue);
		void clipboardClearIntervalChanged(int oldValue, int newValue);
		void codeLabelDisplayStyleChanged(CodeLabelDisplayStyle newStyle);
		void codeLabelDisplayStyleChanged( CodeLabelDisplayStyle oldStyle, CodeLabelDisplayStyle newStyle );
		void codeRevealTimeoutChanged(int newValue);
		void codeRevealTimeoutChanged(int oldValue, int newValue);

	public Q_SLOTS:
		inline void setSingleInstance(bool single) {
			if(single != m_singleInstance) {
				m_singleInstance = single;
								Q_EMIT singleInstanceChanged(!m_singleInstance, m_singleInstance);
				Q_EMIT singleInstanceChanged(m_singleInstance);
				Q_EMIT changed();
			}
		}

		inline void setQuitOnMainWindowClosed(bool quit) {
			if(quit != m_quitOnMainWindowClosed) {
				m_quitOnMainWindowClosed = quit;
								Q_EMIT quitOnMainWindowClosedChanged(!m_quitOnMainWindowClosed, m_quitOnMainWindowClosed);
				Q_EMIT quitOnMainWindowClosedChanged(m_quitOnMainWindowClosed);
				Q_EMIT changed();
			}
		}

		inline void setStartMinimised(bool minimised) {
			if(minimised != m_startMinimised) {
				m_startMinimised = minimised;
								Q_EMIT startMinimisedChanged(!m_startMinimised, m_startMinimised);
				Q_EMIT startMinimisedChanged(m_startMinimised);
				Q_EMIT changed();
			}
		}

		inline void setCopyCodeOnClick(bool copy) {
			if(copy != m_copyCodeOnClick) {
				m_copyCodeOnClick = copy;
								Q_EMIT copyCodeOnClickChanged(!m_copyCodeOnClick, m_copyCodeOnClick);
				Q_EMIT copyCodeOnClickChanged(m_copyCodeOnClick);
				Q_EMIT changed();
			}
		}

		inline void setHideOnCodeCopyClick(bool hide) {
			if(hide != m_hideOnCodeCopyClick) {
				m_hideOnCodeCopyClick = hide;
								Q_EMIT hideOnCodeCopyClickChanged(!m_hideOnCodeCopyClick, m_hideOnCodeCopyClick);
				Q_EMIT hideOnCodeCopyClickChanged(m_hideOnCodeCopyClick);
				Q_EMIT changed();
			}
		}

		inline void setClearClipboardAfterInterval(bool clear) {
			if(clear != m_clearClipboardAfterInterval) {
				m_clearClipboardAfterInterval = clear;
								Q_EMIT hideOnCodeCopyClickChanged(!m_clearClipboardAfterInterval, m_clearClipboardAfterInterval);
				Q_EMIT hideOnCodeCopyClickChanged(m_clearClipboardAfterInterval);
				Q_EMIT changed();
			}
		}

		inline void setClipboardClearInterval(int seconds) {
			if(seconds != m_clipboardClearInterval) {
				qSwap(seconds, m_clipboardClearInterval);
								Q_EMIT clipboardClearIntervalChanged(seconds, m_clipboardClearInterval);
				Q_EMIT clipboardClearIntervalChanged(m_clipboardClearInterval);
				Q_EMIT changed();
			}
		}

		inline void setCodeRevealTimeout(int seconds) {
			if(seconds != m_revealTimeout) {
				qSwap(seconds, m_revealTimeout);
								Q_EMIT clipboardClearIntervalChanged(seconds, m_revealTimeout);
				Q_EMIT clipboardClearIntervalChanged(m_revealTimeout);
				Q_EMIT changed();
			}
		}

		inline void setCodeLabelDisplayStyle(CodeLabelDisplayStyle style) {
			if(style != m_codeLabelDisplayStyle) {
				qSwap(style, m_codeLabelDisplayStyle);
								Q_EMIT codeLabelDisplayStyleChanged(style, m_codeLabelDisplayStyle);
				Q_EMIT codeLabelDisplayStyleChanged(m_codeLabelDisplayStyle);
				Q_EMIT changed();
			}
		}

	private:
		bool m_singleInstance;
		bool m_quitOnMainWindowClosed;
		bool m_startMinimised;
		bool m_copyCodeOnClick;
		bool m_hideOnCodeCopyClick;
		bool m_clearClipboardAfterInterval;
		int m_clipboardClearInterval;
		CodeLabelDisplayStyle m_codeLabelDisplayStyle;
		int m_revealTimeout;
	};
}  // namespace Qonvince

#endif  // QONVINCE_SETTINGS_H
