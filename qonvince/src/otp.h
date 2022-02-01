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

#ifndef QONVINCE_OTP_H
#define QONVINCE_OTP_H

#include <memory>

#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QDateTime>
#include <QIcon>
#include <QtCrypto>

#include "types.h"
#include "base32.h"
#include "application.h"

class QBasicTimer;
class QTimerEvent;
class QSettings;

namespace LibQonvince
{
	class OtpDisplayPlugin;
}

namespace Qonvince
{

	using Base32 = LibQonvince::Base32<QByteArray, char>;

	class Otp
	: public QObject
	{
		Q_OBJECT

	public:
		static constexpr const int DefaultInterval = 30;
		static constexpr const int DefaultDigits = 6;
		static const QDateTime DefaultBaselineTime;

		enum class SeedType
		{
			Plain = 0,
			Base32
		};

		explicit Otp(OtpType type = OtpType::Totp, QObject * parent = nullptr);
		Otp(OtpType type, const QString & issuer, const QString & name, const QByteArray & seed, SeedType seedType = SeedType::Plain, QObject * parent = nullptr);
		Otp(OtpType type, const QString & name, const QByteArray & seed, SeedType seedType = SeedType::Plain, QObject * parent = nullptr);
		Otp(OtpType type, const QByteArray & seed, SeedType seedType = SeedType::Plain, QObject * parent = nullptr);
		~Otp() override;

		static std::unique_ptr<Otp> fromSettings(const QSettings & settings, const QCA::SecureArray & cryptKey);

		inline const OtpType & type() const
		{
			return m_type;
		}

		inline const QString & name() const
		{
			return m_name;
		}

		inline const QString & issuer() const
		{
			return m_issuer;
		}

		inline const QIcon & icon() const
		{
			return m_icon;
		}

		QByteArray seed(SeedType seedType = SeedType::Plain) const;

		inline int interval() const
		{
			return m_interval;
		}

		inline bool revealCodeOnDemand() const
		{
			return m_revealOnDemand;
		}

		inline bool codeIsVisible() const
		{
			return !m_revealOnDemand || m_isRevealed;
		}

		// only for type = HOTP
		quint64 counter() const
		{
			return m_counter;
		}

		// only for type = TOTP; *always* in UTC
		inline const QDateTime baselineTime() const
		{
			return QDateTime::fromMSecsSinceEpoch(m_baselineTime).toUTC();
		}

		inline qint64 baselineSecSinceEpoch() const
		{
			return m_baselineTime;
		}

		inline int timeSinceLastCode() const
		{
			int d(interval());

			if(0 >= d) {
				d = 30;
			}

			return ((QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000) - baselineSecSinceEpoch()) % d;
		}

		inline int timeToNextCode() const
		{
			int d(interval());

			if(0 >= d) {
				d = 30;
			}

			return d - timeSinceLastCode();
		}

		const QString & code();

		inline const QString & displayPluginName() const
		{
			return m_displayPluginName;
		}

		void writeSettings(QSettings & settings, const QCA::SecureArray & cryptKey) const;

	Q_SIGNALS:
		// TODO consider slimming down the # of signals
		void typeChanged(OtpType oldType, OtpType newType);
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

		void typeChanged(OtpType newType);
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

		void codeRevealed();
		void codeHidden();
		void newCodeGenerated(QString);

	protected:
		void timerEvent(QTimerEvent *) override;

	public Q_SLOTS:
		void setType(OtpType);
		void setName(const QString &);
		void setIssuer(const QString &);
		void setIcon(const QIcon &);
		bool setSeed(const QByteArray &, SeedType = SeedType::Plain);
		void setInterval(int);

		bool setDisplayPluginName(const QString & pluginName);

		inline void setRevealOnDemand(bool onDemandOnly)
		{
			if(onDemandOnly != m_revealOnDemand) {
				m_revealOnDemand = onDemandOnly;
				Q_EMIT revealOnDemandChanged(m_revealOnDemand);
				Q_EMIT changed();
			}
		}

		inline void reveal()
		{
			auto wasVisible = codeIsVisible();
			m_isRevealed = true;

			if(!wasVisible && codeIsVisible()) {
				Q_EMIT codeRevealed();
				Q_EMIT changed();
			}
		}

		inline void hide()
		{
			auto wasVisible = codeIsVisible();
			m_isRevealed = false;

			if(wasVisible && !codeIsVisible()) {
				Q_EMIT codeHidden();
				Q_EMIT changed();
			}
		}

		inline void setCounter(quint64 c)
		{
			if(c != m_counter) {
				qSwap(c, m_counter);
				Q_EMIT counterChanged(c, m_counter);
				Q_EMIT counterChanged(m_counter);
				Q_EMIT changed();
			}
		}

		inline void incrementCounter()
		{
			setCounter(m_counter + 1);
		}

		/* time is *always* converted to UTC */
		inline void setBaselineTime(const QDateTime & time)
		{
			setBaselineTime(time.toUTC().toMSecsSinceEpoch() / 1000);
		}

		void setBaselineTime(qint64);
		void resynchroniseRefreshTimer();
		void refreshCode();

	private Q_SLOTS:
		void internalRefreshCode();

	protected:
		static QByteArray totp(const QByteArray & seed, time_t base = 0, int interval = 30);
		static QByteArray hotp(const QByteArray & seed, uint64_t counter);
		static QByteArray hmac(const QByteArray & key, const QByteArray & message);

	private:
		QString m_issuer;
		QString m_name;
		QIcon m_icon;

		// the name (JUST the name, not the path) of the temporary file for the
		// icon so that it persists between invokations of Qonvince
		QString m_iconFileName;

		QString m_displayPluginName;
		mutable Base32 m_seed;
		quint64 m_counter;
		QString m_currentCode;
		qint64 m_baselineTime;
		int m_interval;
		OtpType m_type;
		std::unique_ptr<QBasicTimer> m_refreshTimer;
		bool m_revealOnDemand;
		bool m_isRevealed;

		bool m_resync;
	};

}	// namespace Qonvince

#endif  // QONVINCE_OTP_H
