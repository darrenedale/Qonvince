#ifndef QONVINCE_PASSWORDDIALOGUE_H
#define QONVINCE_PASSWORDDIALOGUE_H

#include <memory>

#include <QDialog>
#include <QString>

class QWidget;

namespace Qonvince {

	namespace Ui {
		class PasswordDialogue;
	}

	class PasswordDialogue
	:	public QDialog {
			Q_OBJECT

		public:
			explicit PasswordDialogue(QWidget * = nullptr);
			explicit PasswordDialogue(const QString &, QWidget * = nullptr);
			~PasswordDialogue();

			QString message() const;
			void setMessage(const QString &);

			QString password() const;
			void setPassword(const QString &);

		public Q_SLOTS:
			inline void showMessage() {
				setMessageVisible(true);
			}

			inline void hideMessage() {
				setMessageVisible(false);
			}

			void setMessageVisible(bool);

		Q_SIGNALS:
			void passwordChanged(QString);

		private:
			std::unique_ptr<Ui::PasswordDialogue> m_ui;
	};

} // namespace Qonvince

#endif // QONVINCE_PASSWORDDIALOGUE_H
