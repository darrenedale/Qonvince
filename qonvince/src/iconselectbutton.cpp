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

/// \file iconselectbutton.cpp
/// \brief Implementation of the IconSelectButton widget.

#include "iconselectbutton.h"
#include "ui_iconselectbutton.h"

#include <QDir>
#include <QFileDialog>

#include "application.h"

using namespace Qonvince;

namespace Qonvince::Detail::IconSelectButton {
	static constexpr const int Padding = 8;
}

IconSelectButton::IconSelectButton(QWidget * parent)
:	IconSelectButton(QIcon(), parent)
{
}

IconSelectButton::IconSelectButton(const QIcon & ic, QWidget * parent)
:	QWidget(parent),
	m_ui(std::make_unique<Ui::IconSelectButton>()),
	m_icon(std::move(ic)),
	m_iconPath()
{
	m_ui->setupUi(this);
	m_ui->chooseIcon->setIcon(m_icon);
	m_ui->clear->setVisible(!m_icon.isNull());
}

IconSelectButton::IconSelectButton(const QString & path, QWidget * parent)
:	IconSelectButton(QIcon(path), parent)
{
	m_iconPath = path;
}

IconSelectButton::~IconSelectButton() = default;

QSize IconSelectButton::sizeHint() const
{
	static const auto padding = QSize(Detail::IconSelectButton::Padding, Detail::IconSelectButton::Padding);
	return m_ui->chooseIcon->iconSize() + padding;
}

void IconSelectButton::clear()
{
	m_icon = {};
	m_iconPath.clear();
	m_ui->chooseIcon->setIcon({});
	m_ui->clear->setVisible(false);
	Q_EMIT cleared();
}

void IconSelectButton::chooseIcon()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("%1 - Choose Icon").arg(QApplication::applicationDisplayName()), (m_iconPath.isEmpty() ? QDir::homePath() : m_iconPath));

	if(fileName.isEmpty()) {
		return;
	}

	if(!setIcon(fileName)) {
		qonvinceApp->showNotification(tr("An error was encountered when loading the icon file \"%1\".").arg(fileName));
		return;
	}
}

void IconSelectButton::setIcon(const QIcon & ic)
{
	m_iconPath.clear();
	m_icon = ic;
	m_ui->chooseIcon->setIcon(ic);
	m_ui->clear->setVisible(!m_icon.isNull());
	Q_EMIT iconChanged(ic);
}

bool IconSelectButton::setIcon(const QString & fileName)
{
	QIcon ic(fileName);

	if(ic.isNull()) {
		return false;
	}

	m_icon = ic;
	m_iconPath = fileName;
	m_ui->chooseIcon->setIcon(m_icon);
	m_ui->clear->setVisible(!m_icon.isNull());
	Q_EMIT iconChanged(m_icon);
	Q_EMIT iconChanged(m_iconPath);
	return true;
}

void IconSelectButton::setIconSize(const QSize & size)
{
	m_ui->chooseIcon->setIconSize(size);
}

void IconSelectButton::resizeEvent(QResizeEvent *)
{
	m_ui->clear->move(m_ui->chooseIcon->width() - m_ui->clear->width(), m_ui->clear->y());
}
