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

#ifndef QONVINCE_ICONSELECTBUTTON_H
#define QONVINCE_ICONSELECTBUTTON_H

#include <memory>

#include <QWidget>
#include <QString>
#include <QIcon>

namespace Qonvince {

	namespace Ui {
		class IconSelectButton;
	}

	class IconSelectButton
	: public QWidget {

			Q_OBJECT

		public:
			explicit IconSelectButton( QWidget * parent = nullptr );
			explicit IconSelectButton( const QIcon & icon, QWidget * parent = nullptr );
			explicit IconSelectButton( const QString & path, QWidget * parent = nullptr );
			~IconSelectButton( void );

			inline const QIcon & icon( void ) const {
				return m_icon;
			}

			inline const QString & iconPath( void ) const {
				return m_iconPath;
			}

			virtual QSize sizeHint( void ) const;

		Q_SIGNALS:
			void iconChanged( const QIcon & icon );
			void iconChanged( const QString & path );
			void cleared( void );

		public Q_SLOTS:
			void clear( void );
			void chooseIcon( void );
			void setIcon( const QIcon & ic );
			bool setIcon( const QString & path );
			void setIconSize( const QSize & size );

		protected:
			virtual void resizeEvent( QResizeEvent * );

		private:
			std::unique_ptr<Ui::IconSelectButton> m_ui;
			QIcon m_icon;
			QString m_iconPath;
	};


} // namespace Qonvince

#endif // QONVINCE_ICONSELECTBUTTON_H
