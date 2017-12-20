#include "otplistitemdelegate.h"

#include <QPen>
#include <QPainter>

#include "otp.h"
#include "otplistmodel.h"


namespace Qonvince {


	namespace Detail {
		namespace OtpListItemDelegate {
			static constexpr const auto SpacingSize = 4;
			static constexpr const auto OtpIconExtent = 32;
			static constexpr const auto ActionIconExtent = 22;
			static constexpr const auto CountdownSizeRatio = 0.5L;	// ratio of counter size to item height
			static constexpr const auto CountdownWarningThreshold = 8;
			static constexpr const auto CountdownCriticalThreshold = 4;

			static const QSize OtpIconSize = {OtpIconExtent, OtpIconExtent};
		}
	}


	OtpListItemDelegate::OtpListItemDelegate()
	: m_countdownWarningColour(-1, -1, -1),
	  m_countdownCriticalColour(-1, -1, -1) {
	}


	// TODO this code needs an efficiency review
	void OtpListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->translate(option.rect.topLeft());
		QRect itemRect({0, 0}, option.rect.size());
		int height = itemRect.height();
		int timerRectSize = static_cast<int>(height * Detail::OtpListItemDelegate::CountdownSizeRatio);
		QRect iconRect({Detail::OtpListItemDelegate::SpacingSize, Detail::OtpListItemDelegate::SpacingSize}, Detail::OtpListItemDelegate::OtpIconSize);
		QRectF timerRect({0.5 + itemRect.right() - timerRectSize - (3 * (Detail::OtpListItemDelegate::ActionIconExtent + (2 * Detail::OtpListItemDelegate::SpacingSize))), 0.5 + ((height - timerRectSize) / 2.0)}, QSize(timerRectSize, timerRectSize));
		QRect codeRect({Detail::OtpListItemDelegate::OtpIconExtent + (2 * Detail::OtpListItemDelegate::SpacingSize), 0}, QPoint(static_cast<int>(timerRect.left()) - Detail::OtpListItemDelegate::SpacingSize - Detail::OtpListItemDelegate::SpacingSize, height - 1));

		QString codeString;

		// only show the code if it's not hidden, or is hidden but user has manually
		// revealed it and it's not timed out
		bool onDemand = index.data(OtpListModel::RevealOnDemandRole).toBool();
		bool showCode = !onDemand;
		//		bool showCode = !onDemand || contains(m_revealedPasscodes, code);

		if(showCode) {
			codeString = index.data(OtpListModel::CodeRole).toString();
		}

		auto displayName = index.data(OtpListModel::LabelRole).toString();
		auto icon = index.data(OtpListModel::IconRole).value<QIcon>();
		auto codeType = static_cast<Otp::CodeType>(index.data(OtpListModel::TypeRole).toInt());
		auto countdown = index.data(OtpListModel::TimeToNextCodeRole).toInt();
		auto interval = index.data(OtpListModel::IntervalRole).toInt();

		if(0 == interval) {
			interval = Otp::DefaultInterval;
		}

		if(displayName.isEmpty()) {
			displayName = tr("<unnamed>");
		}

		if(!onDemand && codeString.isEmpty()) {
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

		if(Detail::OtpListItemDelegate::CountdownWarningThreshold >= countdown && Otp::CodeType::Totp == codeType) {
			painter->setPen(itemPen.color().lighter(200 + (30 * (5 - countdown))));
		}
		else {
			painter->setPen(itemPen);
		}

		QFont nameFont = painter->font();
		QFont codeFont = nameFont;
		nameFont.setBold(false);
		nameFont.setPixelSize(qMax(int(nameFont.pixelSize() * 1.2), height / 3));
		codeFont.setBold(true);
		codeFont.setPixelSize(height / 2);

		QRect nameRect(Detail::OtpListItemDelegate::OtpIconExtent + (2 * Detail::OtpListItemDelegate::SpacingSize), 0, 0, height);

		if(!onDemand || showCode) {
			painter->setFont(codeFont);
			painter->drawText(codeRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight, codeString, &codeRect);
			nameRect.setRight(codeRect.left() - (2 * Detail::OtpListItemDelegate::SpacingSize));
		}
		else {
			nameRect.setRight(codeRect.right() - Detail::OtpListItemDelegate::ActionIconExtent - (2 * Detail::OtpListItemDelegate::SpacingSize));
		}

		// draw the countdown
		if(Otp::CodeType::Totp == codeType) {
			auto countdownColour = option.palette.color(QPalette::WindowText).lighter(150);
			QPen timerPen(countdownColour);
			timerPen.setWidthF(0.5);
			QBrush timerBrush(countdownColour);

			if(Detail::OtpListItemDelegate::CountdownCriticalThreshold >= countdown && m_countdownCriticalColour.isValid()) {
				timerBrush.setColor(m_countdownCriticalColour);
			}
			else if(Detail::OtpListItemDelegate::CountdownWarningThreshold >= countdown && m_countdownWarningColour.isValid()) {
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


	QSize OtpListItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
		if(0 != index.column()) {
			return {};
		}

		return {option.rect.width(), 40};
	}


}  // namespace Qonvince
