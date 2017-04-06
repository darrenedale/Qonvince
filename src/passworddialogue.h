#ifndef QONVINCE_PASSWORDDIALOGUE_H
#define QONVINCE_PASSWORDDIALOGUE_H

#include <QDialog>
#include <QString>

namespace Qonvince {

	namespace Ui {
		class PasswordDialogue;
	}

	class PasswordDialogue
	:	public QDialog {
			Q_OBJECT

		public:
			explicit PasswordDialogue( QWidget * parent = nullptr );
			explicit PasswordDialogue( const QString & msg, QWidget * parent = nullptr );
			~PasswordDialogue( void );

			QString message( void ) const;
			void setMessage( const QString & msg );

			QString password( void ) const;
			void setPassword( const QString & password );

		public Q_SLOTS:
			inline void showMessage( void ) {
				setMessageVisible(true);
			}

			inline void hideMessage( void ) {
				setMessageVisible(false);
			}

			void setMessageVisible( bool vis );

		Q_SIGNALS:
			void passwordChanged( QString );

		private:
			Ui::PasswordDialogue * m_ui;
	};
} // namespace Qonvince

#endif // QONVINCE_PASSWORDDIALOGUE_H
