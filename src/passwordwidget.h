#ifndef QONVINCE_PASWORDWIDGET_H
#define QONVINCE_PASWORDWIDGET_H

#include <QWidget>

namespace Qonvince {

	namespace Ui {
		class PasswordWidget;
	}

	class PasswordWidget
	: public QWidget {

			Q_OBJECT

		public:
			explicit PasswordWidget( QWidget * parent = nullptr );
			~PasswordWidget( void );

			QString password( void ) const;

		Q_SIGNALS:
			void passwordChanged( QString );

		public Q_SLOTS:
			void setPassword( const QString & pw );

		private:
			Ui::PasswordWidget * m_ui;
	};


} // namespace Qonvince

#endif // QONVINCE_PASWORDWIDGET_H
