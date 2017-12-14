#include "src/passworddialogue.h"
#include "ui_passworddialogue.h"

using namespace Qonvince;


PasswordDialogue::PasswordDialogue( QWidget * parent )
:	PasswordDialogue({}, parent) {
}


PasswordDialogue::PasswordDialogue( const QString & msg, QWidget * parent )
:	QDialog(parent),
	m_ui{std::make_unique<Ui::PasswordDialogue>()} {
	m_ui->setupUi(this);
	setMessage(msg);
	setMessageVisible(!msg.isEmpty());
	connect(m_ui->passwordWidget, &PasswordWidget::passwordChanged, this, &PasswordDialogue::passwordChanged);
	adjustSize();
}


PasswordDialogue::~PasswordDialogue( void ) {
}


QString PasswordDialogue::message( void ) const {
	return m_ui->message->text();
}


void PasswordDialogue::setMessage( const QString & msg ) {
	m_ui->message->setText(msg);
}


QString PasswordDialogue::password( void ) const {
	return m_ui->passwordWidget->password();
}


void PasswordDialogue::setPassword( const QString & password ) {
	if(password != m_ui->passwordWidget->password()) {
		m_ui->passwordWidget->setPassword(password);
		Q_EMIT passwordChanged(password);
	}
}


void PasswordDialogue::setMessageVisible( bool vis ) {
	m_ui->message->setVisible(vis);
}
