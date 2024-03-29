/*
 * Copyright 2015 - 2022 Darren Edale
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

namespace Qonvince
{
	namespace Ui
	{
		class IconSelectButton;
	}

	class IconSelectButton
	: public QWidget
	{
		Q_OBJECT

	public:
		explicit IconSelectButton(QWidget * = nullptr);
		explicit IconSelectButton(QIcon, QWidget * = nullptr);
		explicit IconSelectButton(const QString &, QWidget * = nullptr);
		~IconSelectButton() override;

		[[nodiscard]] inline const QIcon & icon() const
		{
			return m_icon;
		}

		[[nodiscard]] inline const QString & iconPath() const
		{
			return m_iconPath;
		}

		[[nodiscard]] QSize sizeHint() const override;

	Q_SIGNALS:
		void iconChanged(const QIcon &);
		void iconChanged(const QString &);
		void cleared();

	public Q_SLOTS:
		void clear();
		void chooseIcon();
		void setIcon(const QIcon & ic);
		bool setIcon(const QString & path);
		void setIconSize(const QSize & size);

	protected:
		void resizeEvent(QResizeEvent *) override;
		void dragEnterEvent(QDragEnterEvent *) override;
		void dragLeaveEvent(QDragLeaveEvent *) override;
		void dropEvent(QDropEvent *) override;

	private:
		std::unique_ptr<Ui::IconSelectButton> m_ui;
		QIcon m_icon;
		QString m_iconPath;
	};


}	// namespace Qonvince

#endif  // QONVINCE_ICONSELECTBUTTON_H
