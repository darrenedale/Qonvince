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

/** \file otplistitemdelegate.cpp
  * \author Darren Edale
  * \date December 2017
  *
  * \brief Implementation of the OtpListItemDelegate class.
  */

#include "otplistitemdelegate.h"

#include <QPen>
#include <QPainter>
#include <QScreen>
#include <optional>

#include "otp.h"
#include "otplistmodel.h"

namespace
{
    /**
     * All pixel-based measurements are the # of pixels at the reference PPI (see Application.h)
     */
    constexpr const auto SpacingSize = 4;
    constexpr const auto OtpIconExtent = 20;
    constexpr const auto ItemHeight = OtpIconExtent + (2 * SpacingSize);
    constexpr const QSize OtpIconSize = {OtpIconExtent, OtpIconExtent};
    constexpr const auto CountdownSizeRatio = 0.5L;  // ratio of counter size to item height
    constexpr const auto CountdownWarningThreshold = 8;
    constexpr const auto CountdownCriticalThreshold = 4;
}

namespace Qonvince
{
	OtpListItemDelegate::OtpListItemDelegate()
	: m_countdownWarningColour(-1, -1, -1),
	  m_countdownCriticalColour(-1, -1, -1),
	  m_actionIconAreaWidth(0)
	{}

	// TODO this code needs an efficiency review
	void OtpListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
	   auto screenScale = option.widget->screen()->physicalDotsPerInchY() / Qonvince::Application::ReferencePixelDensity;
       auto * screen = option.widget->screen();
	   auto spacing = qonvinceApp->referencePxToScreenPx(SpacingSize, screen);
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->translate(option.rect.topLeft());

		// this should be the correct size for the screen pixel density since sizeHint(), where the option rect comes from, already accounts for that
		QRect itemRect({0, 0}, option.rect.size());
		auto height = itemRect.height();
		auto timerRectSize = static_cast<int>(height * CountdownSizeRatio);

		QRect iconRect({spacing, spacing,}, qonvinceApp->referencePxToScreenPx(OtpIconSize, screen));
		QRectF timerRect({0.5 + itemRect.right() - timerRectSize - actionIconAreaWidth(), 0.5 + ((height - timerRectSize) / 2.0)}, QSize(timerRectSize, timerRectSize));
		QRect codeRect(
		        {
		            static_cast<int>(qonvinceApp->referencePxToScreenPx(OtpIconExtent + (2 * SpacingSize), screen)),
		            0,
		            },
		            QPoint(static_cast<int>(timerRect.left()) - static_cast<int>(qonvinceApp->referencePxToScreenPx(SpacingSize + SpacingSize, screen)), height - 1));

		QString codeString;

		// only show the code if it's not hidden, or is hidden but user has manually
		// revealed it and it's not timed out
		bool showCode = index.data(OtpListModel::IsRevealedRole).toBool();

		if(showCode) {
			codeString = index.data(OtpListModel::CodeRole).toString();
		}

		auto displayName = index.data(OtpListModel::LabelRole).toString();
		auto icon = index.data(OtpListModel::IconRole).value<QIcon>();
		auto codeType = static_cast<OtpType>(index.data(OtpListModel::TypeRole).toInt());
		auto countdown = index.data(OtpListModel::TimeToNextCodeRole).toInt();
		auto interval = index.data(OtpListModel::IntervalRole).toInt();

		if(0 == interval) {
			interval = Otp::DefaultInterval;
		}

		if(displayName.isEmpty()) {
			displayName = tr("<unnamed>");
		}

		if(showCode && codeString.isEmpty()) {
			codeString = tr("<no code>");
		}

		QPen itemPen = painter->pen();
		QBrush backgroundBrush = option.palette.base();

		if(option.state & QStyle::State_MouseOver) {
			backgroundBrush = option.palette.highlight();
			itemPen.setColor(option.palette.highlightedText().color());
		}
		else if(0 != (index.row() % 2)) {
			backgroundBrush = option.palette.alternateBase();
		}

		painter->fillRect(itemRect, backgroundBrush);

		if(!icon.isNull()) {
			icon.paint(painter, iconRect);
		}

		if(CountdownWarningThreshold >= countdown && OtpType::Totp == codeType) {
			painter->setPen(itemPen.color().lighter(200 + (30 * (5 - countdown))));
		}
		else {
			painter->setPen(itemPen);
		}

		QFont nameFont = painter->font();
		QFont codeFont = nameFont;
		nameFont.setBold(false);
		nameFont.setPixelSize(qMax(static_cast<int>(nameFont.pixelSize() * 1.2), static_cast<int>(height / 3)));
		codeFont.setBold(true);
		codeFont.setPixelSize(height / 2);

		QRect nameRect(static_cast<int>(qonvinceApp->referencePxToScreenPx(OtpIconExtent + (2 * SpacingSize), screen)), 0, 0, height);

		if(showCode) {
			painter->setFont(codeFont);
			painter->drawText(codeRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight, codeString, &codeRect);
			nameRect.setRight(codeRect.left() - spacing - spacing);
		}
		else {
			nameRect.setRight(codeRect.right());
		}

		// draw the countdown
		if(OtpType::Totp == codeType) {
			auto countdownColour = option.palette.color(QPalette::WindowText).lighter(150);
			QPen timerPen(countdownColour);
			timerPen.setWidthF(0.5);
			QBrush timerBrush(countdownColour);

			if(CountdownCriticalThreshold >= countdown && m_countdownCriticalColour.isValid()) {
				timerBrush.setColor(m_countdownCriticalColour);
			}
			else if(CountdownWarningThreshold >= countdown && m_countdownWarningColour.isValid()) {
				timerBrush.setColor(m_countdownWarningColour);
			}

			painter->save();
			painter->setPen(Qt::transparent);
			painter->setRenderHint(QPainter::Antialiasing, true);
			painter->setBrush(timerBrush);

			// pies take angles in 1/16ths of a degree anticlockwise from 3 o'clock
			// so 5760 is a full circle, 1440 is a quarter circle and represents
			// 12 o'clock
			painter->drawPie(timerRect, 1440, (5760 * countdown / interval));
			painter->setPen(timerPen);
			painter->setBrush(Qt::transparent);
			painter->drawEllipse(timerRect);
			painter->restore();
		}

		// draw the name after the code to ensure the name does not bleed into the code
		painter->setFont(nameFont);
		itemPen.setColor(itemPen.color().lighter(200));
		painter->setPen(itemPen);
		painter->drawText(nameRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft, displayName);

		painter->resetTransform();
		painter->restore();
	}


	QSize OtpListItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		if(0 != index.column()) {
			return {};
		}

		return {option.rect.width(), static_cast<int>(option.widget->screen()->physicalDotsPerInchY() / Qonvince::Application::ReferencePixelDensity * ItemHeight)};
	}
}  // namespace Qonvince
