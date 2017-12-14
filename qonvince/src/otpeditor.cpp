/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Qonvince. If not, see <http://www.gnu.org/licenses/>.
 */

/** \file otpeditor.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the OtpEditor class.
  *
  * \todo convert to widget-and-dialogue (two classes) from widget-with-
  *   control-buttons for future flexibility
  */
#include "otpeditor.h"
#include "ui_otpeditor.h"

#include <QDebug>
#include <QStringBuilder>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExp>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>
#include <QMessageBox>

#include "application.h"
#include "otpqrcodereader.h"
#include "qrcodecreator.h"
#include "otpdisplayplugin.h"


using namespace Qonvince;


OtpEditor::OtpEditor(QWidget * parent)
: OtpEditor(nullptr, parent) {
}


OtpEditor::OtpEditor(Otp * code, QWidget * parent)
: QWidget(parent),
  m_ui{std::make_unique<Ui::OtpEditor>()},
  m_code(nullptr) {
	m_ui->setupUi(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
	/* do this here rather than in .ui file so that source is compatible with
	 * earlier QT versions that don't support this method */
	m_ui->issuerEdit->setClearButtonEnabled(true);
	m_ui->nameEdit->setClearButtonEnabled(true);
	m_ui->seedEdit->setClearButtonEnabled(true);
#endif

	for(const auto & plugin : qonvinceApp->codeDisplayPlugins()) {
		m_ui->displayPlugin->addItem(plugin->pluginName());
	}

	m_ui->advancedSettingsWidget->setVisible(m_ui->advancedSettingsToggle->isChecked());
	m_ui->createBarcodeButton->setVisible(false);

	m_ui->codeTypeGroup->setId(m_ui->hotpButton, static_cast<int>(Otp::CodeType::Hotp));
	m_ui->codeTypeGroup->setId(m_ui->totpButton, static_cast<int>(Otp::CodeType::Totp));

	connect(m_ui->issuerEdit, &QLineEdit::textEdited, this, &OtpEditor::issuerChanged);
	connect(m_ui->nameEdit, &QLineEdit::textEdited, this, &OtpEditor::nameChanged);
	connect(m_ui->seedEdit, &QLineEdit::textEdited, this, &OtpEditor::seedWidgetTextEdited);
	connect(m_ui->displayPlugin, &QComboBox::currentTextChanged, this, &OtpEditor::displayPluginNameChanged);

	connect(m_ui->intervalSpin, qOverload<int>(&QSpinBox::valueChanged), this, &OtpEditor::durationChanged);
	connect(m_ui->issuerEdit, &QLineEdit::textChanged, this, &OtpEditor::updateHeading);
	connect(m_ui->nameEdit, &QLineEdit::textChanged, this, &OtpEditor::updateHeading);

	connect(m_ui->codeTypeGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled), [this](int id, bool checked) {
		if(!checked) {
			return;
		}

		Q_EMIT typeChanged(static_cast<Otp::CodeType>(id));
	});

	connect(m_ui->counterSpin, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
		Q_EMIT counterChanged(static_cast<quint64>(value));
	});

	connect(m_ui->seedEdit, &QLineEdit::editingFinished, [this]() {
		Q_EMIT seedChanged(m_ui->seedEdit->text());
	});

	connect(&(qonvinceApp->settings()), &Settings::codeLabelDisplayStyleChanged, [this]() {
		updateHeading();
		updateWindowTitle();
	});

	connect(m_ui->baseTimeEdit, &QDateTimeEdit::editingFinished, [this]() {
		QDateTime dt(m_ui->baseTimeEdit->dateTime());
		Q_EMIT baseTimeChanged(dt);
		Q_EMIT baseTimeChangedInSeconds(dt.toMSecsSinceEpoch());
	});

	if(!OtpQrCodeReader::isAvailable()) {
		qWarning() << "reading of QR code images is not available";
		m_ui->readBarcodeButton->setVisible(false);
	}

	if(!QrCodeCreator::isAvailable()) {
		qWarning() << "creation of QR code images is not available";
		m_ui->createBarcodeButton->setVisible(false);
	}

	setCode(code);
}


OtpEditor::~OtpEditor() {
#if defined(QT_DEBUG)
	/* NOTE: m_code might be in the process of being destroyed (i.e. the destruction of m_code
	 * can result in the destruction of the widget editing it) */
	qDebug() << "deleting editor for code" << ((void *) m_code);

	if(m_code) {
		qDebug() << "deleting editor for" << m_code->issuer() << ":" << m_code->name();
	}
	else {
		qDebug() << "deleting editor for" << tr("Untitled");
	}
#endif
}


Otp::CodeType OtpEditor::type() const {
	return (m_ui->hotpButton->isChecked() ? Otp::CodeType::Hotp : Otp::CodeType::Totp);
}


bool OtpEditor::revealOnDemand() const {
	return m_ui->revealOnDemand->isChecked();
}


void OtpEditor::setName(const QString & name) {
	if(name != m_ui->nameEdit->text()) {
		m_ui->nameEdit->setText(name);
		Q_EMIT nameChanged(name);
	}
}


void OtpEditor::setIssuer(const QString & issuer) {
	if(issuer != m_ui->issuerEdit->text()) {
		m_ui->issuerEdit->setText(issuer);
		Q_EMIT issuerChanged(issuer);
	}
}


void OtpEditor::setCode(Otp * code) {
	if(code != m_code) {
		if(m_code) {
			m_code->disconnect(this);
			m_ui->intervalSpin->disconnect(this);
			m_ui->nameEdit->disconnect(this);
			m_ui->seedEdit->disconnect(this);
			m_originalSeed = {};
		}

		m_code = code;

		if(m_code) {
			m_originalSeed = m_code->seed();
			/* set initial state for widgets before connecting code so that
			 * widget changes (to initial state) do not trigger code updates */
			m_ui->seedEdit->setText(QString());
			m_ui->issuerEdit->setText(m_code->issuer());
			m_ui->nameEdit->setText(m_code->name());
			m_ui->icon->setIcon(m_code->icon());

			{
				auto plugin = m_code->displayPlugin().lock();

				if(plugin) {
					m_ui->displayPlugin->setCurrentText(plugin->pluginName());
				}
				else {
					m_ui->displayPlugin->setCurrentText("");
				}
			}

			m_ui->intervalSpin->setValue(m_code->interval());
			m_ui->baseTimeEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(code->baselineSecSinceEpoch() * 1000));
			m_ui->counterSpin->setValue(static_cast<int>(m_code->counter()));
			m_ui->revealOnDemand->setChecked(m_code->revealOnDemand());
			setType(m_code->type());

			connect(m_code, &Otp::destroyed, this, &OtpEditor::close);
			connect(m_code, qOverload<Otp::CodeType>(&Otp::typeChanged), this, &OtpEditor::setType);
			connect(m_code, qOverload<QString>(&Otp::issuerChanged), this, &OtpEditor::setIssuer);
			connect(m_code, qOverload<QString>(&Otp::issuerChanged), this, &OtpEditor::updateWindowTitle);
			connect(m_code, qOverload<QString>(&Otp::nameChanged), this, &OtpEditor::setName);
			connect(m_code, qOverload<QString>(&Otp::nameChanged), this, &OtpEditor::updateWindowTitle);
			connect(m_code, qOverload<int>(&Otp::intervalChanged), m_ui->intervalSpin, &QSpinBox::setValue);
			connect(m_code, qOverload<QString>(&Otp::displayPluginChanged), this, &OtpEditor::onDisplayPluginChanged);
			connect(m_code, qOverload<quint64>(&Otp::counterChanged), this, &OtpEditor::setCounter);
			connect(m_code, &Otp::revealOnDemandChanged, this, &OtpEditor::setRevealOnDemand);
			connect(m_ui->intervalSpin, qOverload<int>(&QSpinBox::valueChanged), m_code, &Otp::setInterval);
			connect(m_ui->nameEdit, &QLineEdit::textEdited, m_code, &Otp::setName);
			connect(m_ui->issuerEdit, &QLineEdit::textEdited, m_code, &Otp::setIssuer);
			connect(m_ui->seedEdit, &QLineEdit::editingFinished, this, &OtpEditor::onCodeSeedEditingFinished);
			connect(m_ui->displayPlugin, qOverload<int>(&QComboBox::currentIndexChanged), this, &OtpEditor::onDisplayPluginChanged);
			connect(m_ui->baseTimeEdit, &QDateTimeEdit::editingFinished, this, &OtpEditor::setCodeBaseTimeFromWidget);
			connect(m_ui->revealOnDemand, &QCheckBox::toggled, m_code, &Otp::setRevealOnDemand);
			connect(this, &OtpEditor::typeChanged, m_code, &Otp::setType);
			connect(this, &OtpEditor::counterChanged, m_code, &Otp::setCounter);
		}
		else {
			m_ui->issuerEdit->setText(QString());
			m_ui->nameEdit->setText(QString());
			m_ui->seedEdit->setText(QString());
			m_ui->icon->setIcon(QIcon());
			m_ui->displayPlugin->setCurrentText("");
			m_ui->baseTimeEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(0));
			m_ui->intervalSpin->setValue(0);
			m_ui->counterSpin->setValue(0);
			m_ui->revealOnDemand->setChecked(false);
		}

		updateWindowTitle();
		updateHeading();
	}
}


void OtpEditor::updateWindowTitle() {
	if(m_code) {
		QString name(m_code->name());
		QString issuer(m_code->issuer());
		QString title;

		switch(qonvinceApp->settings().codeLabelDisplayStyle()) {
			case Settings::NameOnly:
				title = name;
				break;

			case Settings::IssuerOnly:
				title = issuer;
				break;

			default:
			case Settings::IssuerAndName:
				if(!name.isEmpty()) {
					if(!issuer.isEmpty()) {
						title = issuer % ":" % name;
					}
					else {
						title = name;
					}
				}
				else {
					title = issuer;
				}
				break;
		}

		setWindowTitle(title.isEmpty() ? tr("Untitled") : title);
	}
	else {
		setWindowTitle(tr("No code"));
	}
}


void OtpEditor::chooseIcon() {
	m_ui->icon->chooseIcon();
}


void OtpEditor::readBarcode() {
	QString fileName(QFileDialog::getOpenFileName(this, tr("%1: Open QR code").arg(QApplication::applicationName())));

	if(!fileName.isEmpty()) {
		readBarcode(fileName);
	}
}


void OtpEditor::closeEvent(QCloseEvent * ev) {
	ev->accept();
	Q_EMIT closing();
}


void OtpEditor::dragEnterEvent(QDragEnterEvent * ev) {
	if(((Qt::CopyAction | Qt::MoveAction) & ev->proposedAction()) && ev->mimeData()->hasUrls()) {
		for(const QUrl & u : ev->mimeData()->urls()) {
			if("file" == u.scheme()) {
				ev->acceptProposedAction();
				return;
			}
		}
	}
}


void OtpEditor::dropEvent(QDropEvent * ev) {
	for(const QUrl & u : ev->mimeData()->urls()) {
		if("file" == u.scheme()) {
			qonvinceApp->readQrCodeFrom(u.path());
		}
	}

	QMessageBox::critical(this, tr("%1 error").arg(QApplication::applicationName()), tr("The dropped file was not recognised as a QR code."));
}


void OtpEditor::readBarcode(const QString & fileName) {
	/* otpauth://{totp|hotp}/{domain}:{user}?params */
	OtpQrCodeReader reader(fileName, this);

	if(reader.decode()) {
		m_code->setName(reader.name());
		m_code->setSeed(reader.seed(), Otp::SeedType::Base32);
	}
	else {
		QMessageBox::critical(this, tr("%1: error").arg(QApplication::applicationName()), tr("The image could not be decoded. Is it really a QR code image?"));
	}
}


bool OtpEditor::createBarcode() {
	if(!QrCodeCreator::isAvailable()) {
		qWarning() << "creation of QR code images is not available";
		return false;
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("%1: Save QR Code").arg(Application::applicationName()));

	if(!fileName.isEmpty()) {
		if(QFileInfo(fileName).exists() && QMessageBox::No == QMessageBox::question(this, tr("%1: Overwrite file?").arg(Application::applicationName()), tr("The file %1 already exists.\n\nDo you want to replace it?").arg(fileName), QMessageBox::Yes | QMessageBox::No)) {
			return false;
		}

		return createBarcode(fileName);
	}

	return false;
}


bool OtpEditor::createBarcode(const QString & fileName) {
	if(!QrCodeCreator::isAvailable()) {
		qWarning() << "creation of QR code images is not available";
		return false;
	}

	QString seed(m_ui->seedEdit->text());

	if(!seed.isEmpty()) {
		QString uri = QString("otpauth://%1/%2:%3?secret=%4").arg(Otp::CodeType::Hotp == m_code->type() ? "hotp" : "totp").arg(QString::fromUtf8(m_ui->issuerEdit->text().toUtf8().toPercentEncoding())).arg(QString::fromUtf8(m_ui->nameEdit->text().toUtf8().toPercentEncoding())).arg(seed);

		if(Otp::CodeType::Hotp == m_code->type()) {
			if(0 != m_ui->counterSpin->value()) {
				uri += "&counter=" + QString::number(m_ui->counterSpin->value());
			}
		}
		else {
			if(30 != m_ui->intervalSpin->value()) {
				uri += QString("&period=") + m_ui->intervalSpin->value();
			}
		}

		/* TODO identify if the plugin is built-in and digits-based, and if so
		 * add the digits to the uri */
		//		if(6 != m_ui->digitsSpin->value()) {
		//			uri += "&digits=" + QString::number(m_ui->digitsSpin->value());
		//		}

		QrCodeCreator creator(uri);
		QImage img(creator.image(QSize(128, 128)));

		if(!img.save(fileName)) {
			QMessageBox::warning(this, tr("%1: Error").arg(Application::applicationName()), tr("Failed to save the QR code image to %1.").arg(fileName));
			qWarning() << "failed to save qr code image to" << fileName;
			return false;
		}

		qonvinceApp->showMessage(Application::applicationName(), tr("QR code created successfully."));
		return true;
	}

	QMessageBox::warning(this, tr("%1: Error").arg(Application::applicationName()), tr("You must enter a seed in order to create a QR code image.").arg(fileName));
	qWarning() << "no code for which to create a qr-code";
	return false;
}


void OtpEditor::updateHeading() {
	QString issuer(m_ui->issuerEdit->text());
	QString name(m_ui->nameEdit->text());
	QString heading;

	switch(qonvinceApp->settings().codeLabelDisplayStyle()) {
		case Settings::NameOnly:
			heading = name;
			break;

		case Settings::IssuerOnly:
			heading = issuer;
			break;

		default:
		case Settings::IssuerAndName:
			if(!name.isEmpty()) {
				if(!issuer.isEmpty()) {
					heading = issuer % ":" % name;
				}
				else {
					heading = name;
				}
			}
			else {
				heading = issuer;
			}
			break;
	}

	if(heading.isEmpty()) {
		heading = tr("<i>&lt;unnamed&gt;</i>");
	}

	m_ui->headingLabel->setText(heading);
}


void OtpEditor::onCodeSeedEditingFinished() {
	if(m_ui->seedEdit->text().isEmpty()) {
		m_code->setSeed(m_originalSeed);
	}
	else {
		m_code->setSeed(m_ui->seedEdit->text().toUtf8(), Otp::SeedType::Base32);
	}
}


void OtpEditor::setCodeBaseTimeFromWidget() {
	m_code->setBaselineTime(m_ui->baseTimeEdit->dateTime());
}


void OtpEditor::setCounter(quint64 c) {
	if(c != quint64(m_ui->counterSpin->value())) {
		m_ui->counterSpin->setValue(c);
	}
}


void OtpEditor::resetCounter() {
	setCounter(0);
}


void OtpEditor::seedWidgetTextEdited() {
	if(!QrCodeCreator::isAvailable() || m_ui->seedEdit->text().isEmpty()) {
		m_ui->createBarcodeButton->setVisible(false);
	}
	else {
		m_ui->createBarcodeButton->setVisible(true);
	}
}


void OtpEditor::onDisplayPluginChanged() {
	if(!m_code) {
		qWarning() << "no code for which to set plugin";
		return;
	}

	auto plugin = qonvinceApp->codeDisplayPluginByName(m_ui->displayPlugin->currentText());

	if(!plugin) {
		qCritical() << "display plugin named" << m_ui->displayPlugin->currentText() << "not found";
		return;
	}

	m_code->setDisplayPlugin(plugin);
}


void OtpEditor::onIconSelected(const QIcon & ic) {
	m_code->setIcon(ic);
	Q_EMIT iconChanged(ic);
}


void OtpEditor::onIconCleared() {
	m_code->setIcon({});
	Q_EMIT iconChanged({});
}


void OtpEditor::setType(const Otp::CodeType & codeType) {
	if(codeType != type()) {
		QSignalBlocker hBlocker(m_ui->hotpButton);
		QSignalBlocker tBlocker(m_ui->totpButton);

		if(Otp::CodeType::Hotp == codeType) {
			m_ui->hotpButton->setChecked(true);
			m_ui->totpButton->setChecked(false);
			m_ui->intervalSpin->setEnabled(false);
			m_ui->baseTimeEdit->setEnabled(false);
			m_ui->counterSpin->setEnabled(true);
		}
		else if(Otp::CodeType::Totp == codeType) {
			m_ui->hotpButton->setChecked(false);
			m_ui->totpButton->setChecked(true);
			m_ui->intervalSpin->setEnabled(true);
			m_ui->baseTimeEdit->setEnabled(true);
			m_ui->counterSpin->setEnabled(false);
		}
	}

	if(m_code && codeType != m_code->type()) {
		m_code->setType(codeType);
	}
}


void OtpEditor::setRevealOnDemand(bool onlyOnDemand) {
	if(onlyOnDemand != revealOnDemand()) {
		QSignalBlocker b(m_ui->revealOnDemand);
		m_ui->revealOnDemand->setChecked(onlyOnDemand);
	}

	if(m_code && m_code->revealOnDemand() != onlyOnDemand) {
		m_code->setRevealOnDemand(onlyOnDemand);
	}
}
