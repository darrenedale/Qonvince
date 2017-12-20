#ifndef OTPLISTITEMACTIONBUTTONS_H
#define OTPLISTITEMACTIONBUTTONS_H

#include <QWidget>
#include <QIcon>

class QResizeEvent;
class QMouseEvent;

namespace Qonvince {

	class OtpListItemActionButtons final
	: public QWidget {
			Q_OBJECT
		public:
			explicit OtpListItemActionButtons(QWidget * parent = nullptr);

		Q_SIGNALS:
			void copyClicked();
			void refreshClicked();
			void removeClicked();

		protected:
			bool event(QEvent *) override;
			void resizeEvent(QResizeEvent *) override;
			void paintEvent(QPaintEvent *) override;

			void enterEvent(QEvent *) override;
			void leaveEvent(QEvent *) override;
			void mouseMoveEvent(QMouseEvent *) override;
			void mousePressEvent(QMouseEvent *) override;
			void mouseReleaseEvent(QMouseEvent *) override;

		private:
			struct ButtonSpec {
				QIcon icon;
				QRect geometry;
			};

			ButtonSpec m_copy;
			ButtonSpec m_refresh;
			ButtonSpec m_remove;
			QRect m_hoverRect;
			QRect m_mouseClickStartRect;
	};

} // namespace Qonvince

#endif // OTPLISTITEMACTIONBUTTONS_H
