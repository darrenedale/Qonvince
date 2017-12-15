#ifndef QONVINCE_OTPEDITORDIALOGUE_H
#define QONVINCE_OTPEDITORDIALOGUE_H

#include <memory>

#include <QDialog>

#include "otp.h"

namespace Qonvince {

	class OtpEditor;

	namespace Ui {
		class OtpEditorDialogue;
	}

	class OtpEditorDialogue
	: public QDialog {
			Q_OBJECT

		public:
			explicit OtpEditorDialogue(QWidget * = nullptr);
			explicit OtpEditorDialogue(Otp *, QWidget * = nullptr);
			~OtpEditorDialogue();

			OtpEditor * editor();
			Otp * otp();

		private:
			std::unique_ptr<Ui::OtpEditorDialogue> m_ui;
	};

} // namespace Qonvince

#endif // QONVINCE_OTPEDITORDIALOGUE_H
