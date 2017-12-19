#include "otplistitemdelegate.h"

#include <QPen>
#include <QPainter>

#include "otp.h"
#include "otplistmodel.h"

namespace Qonvince {

#define QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE 32
#define QONVINCE_OTPCODELISTWIDGET_PROGRESS_GAUGE_SIZE 0.5L /* ratio of gauge size to item height */
#define QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN 4
#define QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE 22

#define QONVINCE_OTPCODELISTWIDGET_TOTP_CRITICAL_THRESHOLD 4
#define QONVINCE_OTPCODELISTWIDGET_TOTP_WARNING_THRESHOLD 8

	OtpListItemDelegate::OtpListItemDelegate()
	: m_countdownWarningColour(-1, -1, -1),
	  m_countdownCriticalColour(-1, -1, -1) {
	}


	// TODO this painting code could do with a review
	void OtpListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->translate(option.rect.topLeft());
		QRect itemRect({0, 0}, option.rect.size());
		int h = itemRect.height();
		int timerRectSize = static_cast<int>(static_cast<long double>(h) * QONVINCE_OTPCODELISTWIDGET_PROGRESS_GAUGE_SIZE);
		QRect iconRect(QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE);
		QRectF timerRect(0.5 + itemRect.right() - timerRectSize - (3 * (QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN)), 0.5 + ((h - timerRectSize) / 2.0), timerRectSize, timerRectSize);
		QRect codeRect({QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0}, QPoint(static_cast<int>(timerRect.left()) - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, h - 1));

		QString codeString;

		/* only show the code if it's not hidden, or is hidden but user has manually
		 * revealed it and it's not timed out */
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

		auto palette = option.widget->palette();
		QPen itemPen = painter->pen();
		QBrush backgroundBrush = palette.base();

		if(option.state & QStyle::State_MouseOver) {
			backgroundBrush = palette.highlight();
			itemPen.setColor(palette.highlightedText().color());
		}
		else if(0 != (index.row() % 2)) {
			backgroundBrush = palette.alternateBase();
		}

		painter->setPen(itemPen);

		/* draw the item background */
		painter->fillRect(itemRect, backgroundBrush);

		/* draw the item icon */
		if(!icon.isNull()) {
			icon.paint(painter, iconRect);
		}

		/* draw the current code */
		if(QONVINCE_OTPCODELISTWIDGET_TOTP_WARNING_THRESHOLD >= countdown && Otp::CodeType::Totp == codeType) {
			painter->setPen(itemPen.color().lighter(200 + (30 * (5 - countdown))));
		}

		QFont nameFont = painter->font();
		QFont codeFont = nameFont;
		nameFont.setBold(false);
		nameFont.setPixelSize(qMax(int(nameFont.pixelSize() * 1.2), h / 3));
		codeFont.setBold(true);
		codeFont.setPixelSize(h / 2);

		QRect nameRect(QONVINCE_OTPCODELISTWIDGET_CODE_ICON_SIZE + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN + QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0, 0, h);

		if(!onDemand || showCode) {
			painter->setFont(codeFont);
			painter->drawText(codeRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight, codeString, &codeRect);
			nameRect.setRight(codeRect.left() - (2 * QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN));
		}
		else {
			nameRect.setRight(codeRect.right() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - (2 * QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN));
		}

		/* draw the item new code timeout counter */
		if(Otp::CodeType::Totp == codeType) {
			auto countdownColour = palette.color(QPalette::WindowText).lighter(150);
			QPen timerPen(countdownColour);
			timerPen.setWidthF(0.5);
			QBrush timerBrush(countdownColour);

			if(QONVINCE_OTPCODELISTWIDGET_TOTP_CRITICAL_THRESHOLD >= countdown && m_countdownCriticalColour.isValid()) {
				timerBrush.setColor(m_countdownCriticalColour);
			}
			else if(QONVINCE_OTPCODELISTWIDGET_TOTP_WARNING_THRESHOLD >= countdown && m_countdownWarningColour.isValid()) {
				timerBrush.setColor(m_countdownWarningColour);
			}

			painter->save();
			painter->setPen(Qt::transparent);
			painter->setRenderHint(QPainter::Antialiasing, true);
			painter->setBrush(timerBrush);

			/* pies take angles in 1/16ths of a degree anticlockwise from 3 o'clock
			 * so 5760 is a full circle, 1440 is a quarter circle and represents
			 * 12 o'clock */
			painter->drawPie(timerRect, 1440, (5760 * countdown / interval));
			painter->setPen(timerPen);
			painter->setBrush(Qt::transparent);
			painter->drawEllipse(timerRect);
			painter->restore();
		}

		// draw the name
		// this has to be done after the code is drawn to ensure the name does not bleed into the code
		painter->setFont(nameFont);
		itemPen.setColor(itemPen.color().lighter(200));
		painter->setPen(itemPen);
		painter->drawText(nameRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft, displayName);

		// draw the action icons
		if(option.state & QStyle::State_MouseOver) {
			QPoint mousePos = option.widget->mapFromGlobal(QCursor::pos()) - option.rect.topLeft();
			painter->setRenderHint(QPainter::Antialiasing, false);
			painter->setBrush(backgroundBrush.color().darker(125));
			painter->setPen(Qt::NoPen);

			if(onDemand && !showCode) {
				QRect buttonRect(codeRect.right() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, itemRect.top() + ((h - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE) / 2), QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE);
				QIcon icon = QIcon::fromTheme("password-show-on", QIcon(":/icons/codeactions/reveal"));

				if(buttonRect.contains(mousePos)) {
					painter->drawRoundedRect(buttonRect, 3, 3);
				}

				painter->drawPixmap(buttonRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));
			}

			QRect buttonRect(itemRect.right() - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, itemRect.top() + ((h - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE) / 2), QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE);
			QIcon icon = QIcon::fromTheme("list-remove", QIcon(":/icons/codeactions/remove"));
			bool doHoverEffect = buttonRect.contains(mousePos);

			if(doHoverEffect) {
				painter->drawRoundedRect(buttonRect, 3, 3);
			}

			painter->drawPixmap(buttonRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));

			buttonRect.adjust(0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0, 0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0);
			doHoverEffect = buttonRect.contains(mousePos);
			icon = QIcon::fromTheme("view-refresh", QIcon(":/icons/codeactions/refresh"));

			if(doHoverEffect) {
				painter->drawRoundedRect(buttonRect, 3, 3);
			}

			painter->drawPixmap(buttonRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));

			buttonRect.adjust(0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0, 0 - QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE - QONVINCE_OTPCODELISTWIDGET_INTERNAL_MARGIN, 0);
			doHoverEffect = buttonRect.contains(mousePos);
			icon = QIcon::fromTheme("edit-copy", QIcon(":/icons/codeactions/copy"));

			if(doHoverEffect) {
				painter->drawRoundedRect(buttonRect, 3, 3);
			}

			painter->drawPixmap(buttonRect, icon.pixmap(QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE, QONVINCE_OTPCODELISTWIDGET_ITEM_ACTION_ICON_SIZE));
		}

		painter->resetTransform();
		painter->restore();
	}


	QSize OtpListItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
		if(0 != index.column()) {
			return {};
		}

		return {option.rect.width(), 40};
		//		return {(option.widget ? option.widget->width() : 400), 40};
	}


}  // namespace Qonvince
