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
	: public QDialog {
		Q_OBJECT

	public:
		explicit PasswordDialogue(QWidget * parent = nullptr);
		explicit PasswordDialogue(const QString & msg, QWidget * parent = nullptr);
		~PasswordDialogue() override;

		QString message() const;
		void setMessage(const QString & msg);

		QString password() const;
		void setPassword(const QString & pwd);

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

}  // namespace Qonvince

#endif  // QONVINCE_PASSWORDDIALOGUE_H
