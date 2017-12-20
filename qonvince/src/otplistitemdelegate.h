#ifndef QONVINCE_OTPLISTITEMDELEGATE_H
#define QONVINCE_OTPLISTITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace Qonvince {

	class OtpListItemDelegate
	: public QStyledItemDelegate {
		public:
			OtpListItemDelegate();

			void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
			QSize sizeHint(const QStyleOptionViewItem & , const QModelIndex &) const override;

			inline const QColor & countdownWarningColour() const {
				return m_countdownWarningColour;
			}

			inline const QColor & countdownCriticalColour() const {
				return m_countdownCriticalColour;
			}

			inline void setCountdownWarningColour(const QColor & colour) {
				if(colour.isValid()) {
					m_countdownWarningColour = colour;
					return;
				}

				// invalid means no custom colour
				m_countdownWarningColour = {-1, -1, -1};
			}

			inline void setCountdownCriticalColour(const QColor & colour) {
				if(colour.isValid()) {
					m_countdownCriticalColour = colour;
					return;
				}

				// invalid means no custom colour
				m_countdownCriticalColour = {-1, -1, -1};
			}

		private:
			QColor m_countdownWarningColour;
			QColor m_countdownCriticalColour;
	};

} // namespace Qonvince

#endif // OTPLISTITEMDELEGATE_H
