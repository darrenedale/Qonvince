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

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QIcon>

#include "base32.h"

class QBasicTimer;
class QTimerEvent;
class QSettings;

namespace Qonvince {
	class Otp
	:	public QObject {

		Q_OBJECT

		Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
		Q_PROPERTY(QString issuer READ issuer WRITE setIssuer NOTIFY issuerChanged)
		Q_PROPERTY(QByteArray seed READ seed WRITE setSeed NOTIFY seedChanged)
		Q_PROPERTY(int digits READ digits WRITE setDigits NOTIFY digitsChanged)
		Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
		Q_PROPERTY(QDateTime baselineTime READ baselineTime WRITE setBaselineTime NOTIFY baselineTimeChanged)
		Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY counterChanged)
		Q_PROPERTY(bool revealOnDemand READ revealOnDemand WRITE setRevealOnDemand NOTIFY revealOnDemandChanged)

		public:
			static const int DefaultInterval;
			static const int DefaultDigits;
			static const QDateTime DefaultBaselineTime;

			enum CodeType {
				TotpCode = 0,
				HotpCode
			};

			enum SeedType {
				PlainSeed = 0,
				Base32Seed
			};

			explicit Otp( const CodeType & type = TotpCode, QObject * parent = nullptr );
			Otp( const CodeType & type, const QString & issuer, const QString & name, const QByteArray & seed, const SeedType & seedType = PlainSeed, QObject * parent = nullptr );
			Otp( const CodeType & type, const QString & name, const QByteArray & seed, const SeedType & seedType = PlainSeed, QObject * parent = nullptr );
			Otp( const CodeType & type, const QByteArray & seed, const SeedType & seedType = PlainSeed, QObject * parent = nullptr );
			virtual ~Otp( void );

			static Otp * fromSettings( const QSettings & settings );

			inline const CodeType & type( void ) const {
				return m_type;
			}

			inline const QString & name( void ) const {
				return m_name;
			}

			inline const QString & issuer( void ) const {
				return m_issuer;
			}

			inline const QIcon & icon( void ) const {
				return m_icon;
			}

			inline int digits( void ) const {
				return m_digits;
			}

			QByteArray seed( const SeedType & seedType = PlainSeed ) const;

			inline int interval( void ) const {
				return m_interval;
			}

			inline bool revealOnDemand( void ) const {
				return m_revealOnDemand;
			}

			/* only for type = HOTP */
			quint64 counter( void ) const {
				return m_counter;
			}

			/* only for type = TOTP */
			/** TODO use c++11 move semantics for retval
			  * is *always* in UTC */
			inline const QDateTime baselineTime( void ) const {
				return QDateTime::fromMSecsSinceEpoch(m_baselineTime).toUTC();
			}

			inline qint64 baselineSecSinceEpoch( void ) const {
				return m_baselineTime;
			}

			inline int timeSinceLastCode( void ) const {
				int d(interval());

				if(0 >= d) {
					d = 30;
				}
//qDebug() << "OtpCode::timeSinceLastCode() - current date/time (UTC):" << QDateTime::currentDateTimeUtc();
				return ((QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000) - baselineSecSinceEpoch()) % d;
			}

			inline int timeToNextCode( void ) const {
				int d(interval());

				if(0 >= d) {
					d = 30;
				}

				return d - timeSinceLastCode();
			}

			inline const QString & code( void ) {
				return m_currentCode;
			}

			void writeSettings( QSettings & settings ) const;

		Q_SIGNALS:
			void typeChanged( CodeType oldType, CodeType newType );
			void issuerChanged( QString oldIssuer, QString newIssuer );
			void nameChanged( QString oldName, QString newName );
			void seedChanged( QByteArray oldSeed, QByteArray newSeed );
			void seedChanged( QString oldSeedBase32, QString newSeedBase32 );
			void digitsChanged( int oldDigits, int newDigits );
			void intervalChanged( int oldInterval, int newInterval );
			void baselineTimeChanged( QDateTime oldTime, QDateTime newTime );
			void baselineTimeChanged( qint64 oldInterval, qint64 newInterval );
			void counterChanged( quint64 oldCounter, quint64 newCounter );

			void changed();

			void typeChanged( Otp::CodeType newType );
			void issuerChanged( QString newIssuer );
			void nameChanged( QString newName );
			void iconChanged( QIcon newIcon );
			void seedChanged( QByteArray newSeed );
			void seedChanged( QString newSeedBase32 );
			void digitsChanged( int newDigits );
			void revealOnDemandChanged( bool );
			void intervalChanged( int newInterval );
			void baselineTimeChanged( QDateTime newTime );
			void baselineTimeChanged( qint64 newSecSinceEpoch );
			void counterChanged( quint64 newCounter );

			void newCodeGenerated( QString );

		protected:
			virtual void timerEvent( QTimerEvent * );

		public Q_SLOTS:
			void setType( Otp::CodeType type );
			void setName( const QString & name );
			void setIssuer( const QString & issuer );
			void setIcon( const QIcon & icon );
			bool setSeed( const QByteArray & seed, const SeedType & seedType = PlainSeed );
			void setInterval( const int & duration );

			inline bool setDigits( int digits ) {
				if(digits >= 6 && digits <= 8) {
					if(digits != m_digits) {
						qSwap(m_digits, digits);
						Q_EMIT digitsChanged(digits, m_digits);
						Q_EMIT digitsChanged(m_digits);
						Q_EMIT changed();
					}

					return true;
				}

				return false;
			}

			inline void setRevealOnDemand( bool onDemandOnly ) {
				if(onDemandOnly != m_revealOnDemand) {
					m_revealOnDemand = onDemandOnly;
					Q_EMIT revealOnDemandChanged(m_revealOnDemand);
					Q_EMIT changed();
				}
			}

			inline void setCounter( quint64 c ) {
				if(c != m_counter) {
					qSwap(c, m_counter);
					Q_EMIT counterChanged(c, m_counter);
					Q_EMIT counterChanged(m_counter);
					Q_EMIT changed();
				}
			}

			inline void incrementCounter( void ) {
				setCounter(m_counter + 1);
			}

			/* time is *always* converted to UTC */
			inline void setBaselineTime( const QDateTime & time ) {
				setBaselineTime(time.toUTC().toMSecsSinceEpoch() / 1000);
			}

			void setBaselineTime( const qint64 & secSinceEpoch );
			void resynchroniseRefreshTimer( void );
			void refreshCode();

		private Q_SLOTS:
			void internalRefreshCode();

		public:
			static QString totp( const QByteArray & seed, time_t base = 0, int interval = 30, int digits = 6 );

			static QString hotp( const QByteArray & seed, quint64 counter, int digits = 6 );

			static QByteArray hmac( const QByteArray & key, const QByteArray & message );

		private:
			CodeType m_type;
			QString m_issuer;
			QString m_name;
			QIcon m_icon;

			/* the name (JUST the name, not the path) of the temporary file for the icon so that it persists between invokations of Qonvince */
			QString m_iconFileName;

//			QByteArray m_seed;
			mutable Base32 m_seed;
			bool m_revealOnDemand;
			quint64 m_counter;
			QString m_currentCode;
			int m_interval;
			int m_digits;
			qint64 m_baselineTime;

			QBasicTimer * m_refreshTimer;
			bool m_resync;
	};
}

#endif // QONVINCE_OTP_H
