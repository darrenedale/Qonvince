#include "otplistitemactionbuttons.h"

#include <QPainter>
#include <QEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QCursor>
#include <QApplication>
#include <QToolTip>

#include "otplistview.h"


namespace Qonvince {


	namespace Detail {
		namespace OtpListItemActionButtons {
			static constexpr const int MinimumIconExtent = 22;
			static constexpr const int SpacingSize = 4;
			static constexpr const int HoverRectRounding = 3;
			static const QSize MinimumIconSize = {MinimumIconExtent, MinimumIconExtent};
		}  // namespace OtpListItemActionButtons
	}		// namespace Detail


	OtpListItemActionButtons::OtpListItemActionButtons(QWidget * parent)
	: QWidget(parent),
	  m_copy({QIcon::fromTheme("edit-copy", QIcon(":/icons/codeactions/copy")), {{0, 0}, Detail::OtpListItemActionButtons::MinimumIconSize}}),
	  m_refresh({QIcon::fromTheme("view-refresh", QIcon(":/icons/codeactions/refresh")), {{Detail::OtpListItemActionButtons::MinimumIconExtent + Detail::OtpListItemActionButtons::SpacingSize, 0}, Detail::OtpListItemActionButtons::MinimumIconSize}}),
	  m_remove({QIcon::fromTheme("list-remove", QIcon(":/icons/codeactions/remove")), {{2 * (Detail::OtpListItemActionButtons::MinimumIconExtent + Detail::OtpListItemActionButtons::SpacingSize), 0}, Detail::OtpListItemActionButtons::MinimumIconSize}}),
	  m_hoverRect(),
	  m_mouseClickStartRect() {
		setMinimumSize((3 * Detail::OtpListItemActionButtons::MinimumIconExtent) + (2 * Detail::OtpListItemActionButtons::SpacingSize), Detail::OtpListItemActionButtons::MinimumIconExtent);
	}


	bool OtpListItemActionButtons::event(QEvent * ev) {
		if(QEvent::ToolTip == ev->type()) {
			auto globalMousePos = QCursor::pos();
			auto widgetMousePos = mapFromGlobal(globalMousePos);
			QString txt;

			if(m_copy.geometry.contains(widgetMousePos)) {
				txt = tr("Copy the current code to the clipboard.");
			}
			else if(m_refresh.geometry.contains(widgetMousePos)) {
				txt = tr("Refresh the code now.");
			}
			else if(m_remove.geometry.contains(widgetMousePos)) {
				txt = tr("Remove this OTP from %1.").arg(QApplication::applicationDisplayName());
			}

			if(!txt.isEmpty()) {
				QToolTip::showText(globalMousePos, txt, this);
				ev->accept();
				return true;
			}
		}

		return QWidget::event(ev);
	}


	void OtpListItemActionButtons::resizeEvent(QResizeEvent * ev) {
		const int iconExtent = qMin(height(), (width() - (2 * Detail::OtpListItemActionButtons::SpacingSize)) / 3);
		const QSize iconSize = {iconExtent, iconExtent};
		m_copy.geometry.setSize(iconSize);
		m_refresh.geometry.setSize(iconSize);
		m_refresh.geometry.moveLeft(iconExtent + Detail::OtpListItemActionButtons::SpacingSize);
		m_remove.geometry.setSize(iconSize);
		m_remove.geometry.moveLeft(2 * (iconExtent + Detail::OtpListItemActionButtons::SpacingSize));
		m_mouseClickStartRect = {};
		m_hoverRect = {};
		QWidget::resizeEvent(ev);
	}


	void OtpListItemActionButtons::paintEvent(QPaintEvent *) {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);

		if(!m_hoverRect.isNull()) {
			painter.save();
			painter.setPen(Qt::transparent);
			painter.setBrush(palette().highlight().color().darker(125));
			painter.drawRoundedRect(m_hoverRect, Detail::OtpListItemActionButtons::HoverRectRounding, Detail::OtpListItemActionButtons::HoverRectRounding);
			painter.restore();
		}

		painter.drawPixmap(m_copy.geometry, m_copy.icon.pixmap(m_copy.geometry.size()));
		painter.drawPixmap(m_refresh.geometry, m_refresh.icon.pixmap(m_refresh.geometry.size()));
		painter.drawPixmap(m_remove.geometry, m_remove.icon.pixmap(m_remove.geometry.size()));
	}


	void OtpListItemActionButtons::mouseMoveEvent(QMouseEvent * ev) {
		auto mousePos = ev->pos();

		if(m_copy.geometry.contains(mousePos)) {
			if(m_hoverRect != m_copy.geometry) {
				QRegion damage(m_hoverRect);
				damage += m_copy.geometry;
				m_hoverRect = m_copy.geometry;
				update(damage);
			}
		}
		else if(m_refresh.geometry.contains(mousePos)) {
			if(m_hoverRect != m_refresh.geometry) {
				QRegion damage(m_hoverRect);
				damage += m_refresh.geometry;
				m_hoverRect = m_refresh.geometry;
				update(damage);
			}
		}
		else if(m_remove.geometry.contains(mousePos)) {
			if(m_hoverRect != m_remove.geometry) {
				QRegion damage(m_hoverRect);
				damage += m_remove.geometry;
				m_hoverRect = m_remove.geometry;
				update(damage);
			}
		}
		else if(!m_hoverRect.isNull()) {
			QRegion damage(m_hoverRect);
			m_hoverRect = {};
			update(damage);
		}
	}


	void OtpListItemActionButtons::mousePressEvent(QMouseEvent * ev) {
		// mouseMoveEvent() tracks which icon is hovered, and we can't
		// click on an icon unless it's hovered, so we can take advantage
		// of the work done in mouseMoveEvent and only check whether the
		// click is within the hovered rect - we don't need to check any
		// others because we know they can't be clicked

		if(!m_hoverRect.isNull() && m_hoverRect.contains(ev->pos())) {
			m_mouseClickStartRect = m_hoverRect;
			ev->accept();
		}
		else {
			m_mouseClickStartRect = {};
		}
	}


	void OtpListItemActionButtons::mouseReleaseEvent(QMouseEvent * ev) {
		if(m_mouseClickStartRect.isNull()) {
			return;
		}

		auto mouseReleasePos = ev->pos();

		if(m_mouseClickStartRect.contains(mouseReleasePos)) {
			// could check for equality between click rect and icon rect, but
			// contains is likely to be marginally quicker
			if(m_copy.geometry.contains(mouseReleasePos)) {
				Q_EMIT copyClicked();
			}
			else if(m_refresh.geometry.contains(mouseReleasePos)) {
				Q_EMIT refreshClicked();
			}
			else if(m_remove.geometry.contains(mouseReleasePos)) {
				Q_EMIT removeClicked();
			}
		}

		m_mouseClickStartRect = {};
		ev->accept();
	}


	void OtpListItemActionButtons::enterEvent(QEvent * ev) {
		setMouseTracking(true);

		auto * parent = qobject_cast<OtpListView *>(parentWidget());

		if(parent) {
			std::cout << "sending parent enter event\n"
						 << std::flush;
			QApplication::sendEvent(parent, ev);
		}
	}


	void OtpListItemActionButtons::leaveEvent(QEvent *) {
		setMouseTracking(false);

		if(!m_hoverRect.isNull()) {
			QRegion damage = std::move(m_hoverRect);
			m_hoverRect = {};
			update(damage);
		}
	}


}  // namespace Qonvince
