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

#ifndef QONVINCE_OTP_H
#define QONVINCE_OTP_H

#include <memory>

#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QDateTime>
#include <QIcon>
#include <QtCrypto/QtCrypto>

#include "base32.h"
#include "application.h"

class QBasicTimer;
class QTimerEvent;
class QSettings;

namespace LibQonvince {
	class OtpDisplayPlugin;
}

namespace Qonvince {

	using Base32 = LibQonvince::Base32<QByteArray, char>;

	class Otp
	: public QObject {
		Q_OBJECT

	public:
		static constexpr const int DefaultInterval = 30;
		static constexpr const int DefaultDigits = 6;
		static const QDateTime DefaultBaselineTime;

		enum class CodeType {
			Totp = 0,
			Hotp
		};

		enum class SeedType {
			Plain = 0,
			Base32
		};

		explicit Otp(const CodeType & type = CodeType::Totp, QObject * parent = nullptr);
		Otp(const CodeType & type, const QString & issuer, const QString & name, const QByteArray & seed, const SeedType & seedType = SeedType::Plain, QObject * parent = nullptr);
		Otp(const CodeType & type, const QString & name, const QByteArray & seed, const SeedType & seedType = SeedType::Plain, QObject * parent = nullptr);
		Otp(const CodeType & type, const QByteArray & seed, const SeedType & seedType = SeedType::Plain, QObject * parent = nullptr);
		virtual ~Otp();

		static std::unique_ptr<Otp> fromSettings(const QSettings & settings, const QCA::SecureArray & cryptKey);

		inline const CodeType & type() const {
			return m_type;
		}

		inline const QString & name() const {
			return m_name;
		}

		inline const QString & issuer() const {
			return m_issuer;
		}

		inline const QIcon & icon() const {
			return m_icon;
		}

		QByteArray seed(const SeedType & seedType = SeedType::Plain) const;

		inline int interval() const {
			return m_interval;
		}

		inline bool revealOnDemand() const {
			return m_revealOnDemand;
		}

		/* only for type = HOTP */
		quint64 counter() const {
			return m_counter;
		}

		// only for type = TOTP
		// *always* in UTC
		inline const QDateTime baselineTime() const {
			return QDateTime::fromMSecsSinceEpoch(m_baselineTime).toUTC();
		}

		inline qint64 baselineSecSinceEpoch() const {
			return m_baselineTime;
		}

		inline int timeSinceLastCode() const {
			int d(interval());

			if(0 >= d) {
				d = 30;
			}

			return ((QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000) - baselineSecSinceEpoch()) % d;
		}

		inline int timeToNextCode() const {
			int d(interval());

			if(0 >= d) {
				d = 30;
			}

			return d - timeSinceLastCode();
		}

		const QString & code();

		void writeSettings(QSettings & settings, const QCA::SecureArray & cryptKey) const;

	Q_SIGNALS:
		void typeChanged(CodeType oldType, CodeType newType);
		void issuerChanged(QString oldIssuer, QString newIssuer);
		void nameChanged(QString oldName, QString newName);
		void seedChanged(QByteArray oldSeed, QByteArray newSeed);
		void seedBase32Changed(QString oldSeedBase32, QString newSeedBase32);
		void digitsChanged(int oldDigits, int newDigits);
		void displayPluginChanged(QString oldPlugin, QString newPlugin);
		void intervalChanged(int oldInterval, int newInterval);
		void baselineTimeChanged(QDateTime oldTime, QDateTime newTime);
		void baselineTimeChanged(qint64 oldInterval, qint64 newInterval);
		void counterChanged(quint64 oldCounter, quint64 newCounter);

		void changed();

		void typeChanged(Otp::CodeType newType);
		void issuerChanged(QString newIssuer);
		void nameChanged(QString newName);
		void iconChanged(QIcon newIcon);
		void seedChanged(QByteArray newSeed);
		void seedBase32Changed(QString newSeedBase32);
		void digitsChanged(int newDigits);
		void displayPluginChanged(QString newPlugin);
		void revealOnDemandChanged(bool);
		void intervalChanged(int newInterval);
		void baselineTimeChanged(QDateTime newTime);
		void baselineTimeChangedInSeconds(qint64 newSecSinceEpoch);
		void counterChanged(quint64 newCounter);

		void newCodeGenerated(QString);

	protected:
		virtual void timerEvent(QTimerEvent *);

	public Q_SLOTS:
		void setType(Otp::CodeType);
		void setName(const QString &);
		void setIssuer(const QString &);
		void setIcon(const QIcon &);
		bool setSeed(const QByteArray &, const SeedType & = SeedType::Plain);
		void setInterval(const int &);

		inline const QString & displayPluginName() const {
			return m_displayPluginName;
		}

		inline LibQonvince::OtpDisplayPlugin * displayPlugin() const {
			return qonvinceApp->otpDisplayPluginByName(m_displayPluginName);
		}

		bool setDisplayPluginName(const QString & pluginName);

		inline void setRevealOnDemand(bool onDemandOnly) {
			if(onDemandOnly != m_revealOnDemand) {
				m_revealOnDemand = onDemandOnly;
				Q_EMIT revealOnDemandChanged(m_revealOnDemand);
				Q_EMIT changed();
			}
		}

		inline void setCounter(quint64 c) {
			if(c != m_counter) {
				qSwap(c, m_counter);
				Q_EMIT counterChanged(c, m_counter);
				Q_EMIT counterChanged(m_counter);
				Q_EMIT changed();
			}
		}

		inline void incrementCounter() {
			setCounter(m_counter + 1);
		}

		/* time is *always* converted to UTC */
		inline void setBaselineTime(const QDateTime & time) {
			setBaselineTime(time.toUTC().toMSecsSinceEpoch() / 1000);
		}

		void setBaselineTime(const qint64 &);
		void resynchroniseRefreshTimer();
		void refreshCode();

	private Q_SLOTS:
		void internalRefreshCode();

	protected:
		static QString totp(const QByteArray & seed, LibQonvince::OtpDisplayPlugin * plugin, time_t base = 0, int interval = 30);
		static QString hotp(const QByteArray & seed, LibQonvince::OtpDisplayPlugin * plugin, quint64 counter);
		static QByteArray hmac(const QByteArray & key, const QByteArray & message);

	private:
		CodeType m_type;
		QString m_issuer;
		QString m_name;
		QIcon m_icon;

		// the name (JUST the name, not the path) of the temporary file for the
		// icon so that it persists between invokations of Qonvince
		QString m_iconFileName;

		mutable Base32 m_seed;
		bool m_revealOnDemand;
		quint64 m_counter;
		QString m_currentCode;
		int m_interval;
		qint64 m_baselineTime;

		std::unique_ptr<QBasicTimer> m_refreshTimer;
		bool m_resync;

		QString m_displayPluginName;
	};

}  // namespace Qonvince

#endif  // QONVINCE_OTP_H
