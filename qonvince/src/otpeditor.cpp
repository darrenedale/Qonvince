/*
 * Copyright 2015 - 2022 Darren Edale
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
  */
#include "otpeditor.h"
#include "ui_otpeditor.h"

#include <QStringBuilder>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegExp>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>
#include <QMessageBox>

#include "types.h"
#include "qtiostream.h"
#include "functions.h"
#include "application.h"
#include "otpqrcodereader.h"
#include "qrcodecreator.h"
#include "otpdisplayplugin.h"


namespace Qonvince {


	OtpEditor::OtpEditor(QWidget * parent)
	: OtpEditor(nullptr, parent) {
	}


	OtpEditor::OtpEditor(Otp * code, QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::OtpEditor>()),
	  m_otp(nullptr) {
		m_ui->setupUi(this);

		m_ui->advancedSettingsWidget->setVisible(m_ui->advancedToggle->isChecked());
		m_ui->createBarcodeButton->setVisible(false);

		m_ui->codeTypeGroup->setId(m_ui->hotpButton, static_cast<int>(OtpType::Hotp));
		m_ui->codeTypeGroup->setId(m_ui->totpButton, static_cast<int>(OtpType::Totp));

		connect(m_ui->issuerEdit, &QLineEdit::textEdited, this, &OtpEditor::issuerChanged);
		connect(m_ui->nameEdit, &QLineEdit::textEdited, this, &OtpEditor::nameChanged);
		connect(m_ui->seedEdit, &QLineEdit::textEdited, this, &OtpEditor::seedWidgetTextEdited);
		connect(m_ui->displayPlugin, &OtpDisplayPluginChooser::currentPluginChanged, this, &OtpEditor::displayPluginNameChanged);

		connect(m_ui->intervalSpin, qOverload<int>(&QSpinBox::valueChanged), this, &OtpEditor::durationChanged);
		connect(m_ui->issuerEdit, &QLineEdit::textChanged, this, &OtpEditor::updateHeading);
		connect(m_ui->nameEdit, &QLineEdit::textChanged, this, &OtpEditor::updateHeading);

		// no need to fix -Wclazy-connect-3arg-lambda for these: emitter can't outlive
		// captured this
		connect(m_ui->codeTypeGroup, qOverload<int, bool>(&QButtonGroup::idToggled), [this](int id, bool checked) {
			if(!checked) {
				return;
			}

			Q_EMIT typeChanged(static_cast<OtpType>(id));
		});

		connect(m_ui->counterSpin, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
			Q_EMIT counterChanged(static_cast<quint64>(value));
		});

		connect(m_ui->seedEdit, &QLineEdit::editingFinished, [this]() {
			Q_EMIT seedChanged(m_ui->seedEdit->text());
		});

		connect(m_ui->baseTimeEdit, &QDateTimeEdit::editingFinished, [this]() {
			QDateTime dt(m_ui->baseTimeEdit->dateTime());
			Q_EMIT baseTimeChanged(dt);
			Q_EMIT baseTimeChangedInSeconds(dt.toMSecsSinceEpoch());
		});

		// this one needs a context, however, otherwise the settings will continue to call the
		// lambda after this editor has been removed
		connect(&(qonvinceApp->settings()), qOverload<CodeLabelDisplayStyle>(&Settings::codeLabelDisplayStyleChanged), this, [this]() {
			updateHeading();
			updateWindowTitle();
		});

		if(!OtpQrCodeReader::isAvailable()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: reading of QR code images is not available\n";
			m_ui->readBarcodeButton->setVisible(false);
		}

		if(!QrCodeCreator::isAvailable()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: creation of QR code images is not available\n";
			m_ui->createBarcodeButton->setVisible(false);
		}

		setOtp(code);
	}


	OtpEditor::~OtpEditor() {
#ifndef NDEBUG
		// NOTE: m_otp might be in the process of being destroyed (i.e. the destruction of m_otp
		// can result in the destruction of the widget editing it) */
		std::cerr << "deleting editor for code @ 0x" << (static_cast<void *>(m_otp)) << " - ";

		if(m_otp) {
			std::cerr << otpLabel(m_otp) << "\n";
		}
		else {
			std::cerr << "{Untitled}\n";
		}
#endif
	}


	OtpType OtpEditor::type() const {
		return (m_ui->hotpButton->isChecked() ? OtpType::Hotp : OtpType::Totp);
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


	void OtpEditor::setOtp(Otp * code) {
		if(code != m_otp) {
			if(m_otp) {
				m_otp->disconnect(this);
				m_ui->intervalSpin->disconnect(this);
				m_ui->nameEdit->disconnect(this);
				m_ui->seedEdit->disconnect(this);
				m_originalSeed = {};
			}

			m_otp = code;

			if(m_otp) {
				m_originalSeed = m_otp->seed();
				m_ui->seedEdit->setText({});
				m_ui->issuerEdit->setText(m_otp->issuer());
				m_ui->nameEdit->setText(m_otp->name());
				m_ui->icon->setIcon(m_otp->icon());
				m_ui->displayPlugin->setCurrentPluginName(m_otp->displayPluginName());

				m_ui->intervalSpin->setValue(m_otp->interval());
				m_ui->baseTimeEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(code->baselineSecSinceEpoch() * 1000));
				m_ui->counterSpin->setValue(static_cast<int>(m_otp->counter()));
				m_ui->revealOnDemand->setChecked(m_otp->revealCodeOnDemand());
				setType(m_otp->type());

				connect(m_otp, &Otp::destroyed, this, &OtpEditor::close);
				connect(m_otp, qOverload<OtpType>(&Otp::typeChanged), this, &OtpEditor::setType);
				connect(m_otp, qOverload<QString>(&Otp::issuerChanged), this, &OtpEditor::setIssuer);
				connect(m_otp, qOverload<QString>(&Otp::issuerChanged), this, &OtpEditor::updateWindowTitle);
				connect(m_otp, qOverload<QString>(&Otp::nameChanged), this, &OtpEditor::setName);
				connect(m_otp, qOverload<QString>(&Otp::nameChanged), this, &OtpEditor::updateWindowTitle);
				connect(m_otp, qOverload<int>(&Otp::intervalChanged), m_ui->intervalSpin, &QSpinBox::setValue);
				connect(m_otp, qOverload<QString>(&Otp::displayPluginChanged), this, &OtpEditor::onDisplayPluginChanged);
				connect(m_otp, qOverload<quint64>(&Otp::counterChanged), this, &OtpEditor::setCounter);
				connect(m_otp, &Otp::revealOnDemandChanged, this, &OtpEditor::setRevealOnDemand);
				connect(m_ui->intervalSpin, qOverload<int>(&QSpinBox::valueChanged), m_otp, &Otp::setInterval);
				connect(m_ui->nameEdit, &QLineEdit::textEdited, m_otp, &Otp::setName);
				connect(m_ui->issuerEdit, &QLineEdit::textEdited, m_otp, &Otp::setIssuer);
				connect(m_ui->seedEdit, &QLineEdit::editingFinished, this, &OtpEditor::onCodeSeedEditingFinished);
				connect(m_ui->displayPlugin, qOverload<int>(&QComboBox::currentIndexChanged), this, &OtpEditor::onDisplayPluginChanged);
				connect(m_ui->baseTimeEdit, &QDateTimeEdit::editingFinished, this, &OtpEditor::setCodeBaseTimeFromWidget);
				connect(m_ui->revealOnDemand, &QCheckBox::toggled, m_otp, &Otp::setRevealOnDemand);
				connect(this, &OtpEditor::typeChanged, m_otp, &Otp::setType);
				connect(this, &OtpEditor::counterChanged, m_otp, &Otp::setCounter);
			}
			else {
				m_ui->issuerEdit->setText({});
				m_ui->nameEdit->setText({});
				m_ui->seedEdit->setText({});
				m_ui->icon->setIcon(QIcon());
				m_ui->displayPlugin->setCurrentText(QStringLiteral(""));
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
		if(m_otp) {
			QString title = otpLabel(m_otp);
			setWindowTitle(title.isEmpty() ? tr("Untitled") : otpLabel(m_otp));
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


	void OtpEditor::dragEnterEvent(QDragEnterEvent * ev) {
		if(((Qt::CopyAction | Qt::MoveAction) & ev->proposedAction()) && ev->mimeData()->hasUrls()) {
			const auto urls = ev->mimeData()->urls();

			for(const QUrl & url : urls) {
				if(QStringLiteral("file") == url.scheme()) {
					ev->acceptProposedAction();
					return;
				}
			}
		}
	}


	void OtpEditor::dropEvent(QDropEvent * ev) {
		const auto urls = ev->mimeData()->urls();

		for(const QUrl & url : urls) {
			if(QStringLiteral("file") == url.scheme()) {
				qonvinceApp->readQrCodeFrom(url.path());
			}
		}

		QMessageBox::critical(this, tr("%1 error").arg(QApplication::applicationName()), tr("The dropped file was not recognised as a QR code."));
	}


	void OtpEditor::readBarcode(const QString & fileName) {
		/* otpauth://{totp|hotp}/{domain}:{user}?params */
		OtpQrCodeReader reader(fileName, this);

		if(reader.decode()) {
			m_otp->setName(reader.name());
			m_otp->setSeed(reader.seed(), Otp::SeedType::Base32);
		}
		else {
			QMessageBox::critical(this, tr("%1: error").arg(QApplication::applicationName()), tr("The image could not be decoded. Is it really a QR code image?"));
		}
	}


	bool OtpEditor::createBarcode() {
		if(!QrCodeCreator::isAvailable()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: creation of QR code images is not available\n";
			return false;
		}

		QString fileName = QFileDialog::getSaveFileName(this, tr("%1: Save QR Code").arg(Application::applicationName()));

		if(fileName.isEmpty()) {
			return false;
		}

		if(QFileInfo::exists(fileName) && QMessageBox::No == QMessageBox::question(this, tr("%1: Overwrite file?").arg(Application::applicationName()), tr("The file %1 already exists.\n\nDo you want to replace it?").arg(fileName), QMessageBox::Yes | QMessageBox::No)) {
			return false;
		}

		return createBarcode(fileName);
	}


	bool OtpEditor::createBarcode(const QString & fileName) {
		if(!QrCodeCreator::isAvailable()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: creation of QR code images is not available\n";
			return false;
		}

		QString seed(m_ui->seedEdit->text());

		if(!seed.isEmpty()) {
			QString uri = QStringLiteral("otpauth://%1/%2:%3?secret=%4").arg((OtpType::Hotp == m_otp->type() ? QStringLiteral("hotp") : QStringLiteral("totp")), QString::fromUtf8(m_ui->issuerEdit->text().toUtf8().toPercentEncoding()), QString::fromUtf8(m_ui->nameEdit->text().toUtf8().toPercentEncoding()), seed);

			if(OtpType::Hotp == m_otp->type()) {
				if(0 != m_ui->counterSpin->value()) {
					uri += QStringLiteral("&counter=") + QString::number(m_ui->counterSpin->value());
				}
			}
			else {
				if(30 != m_ui->intervalSpin->value()) {
					uri += QStringLiteral("&period=") + QString::number(m_ui->intervalSpin->value());
				}
			}

			// this detection of digits URL param is not entirely satisfactory
			auto pluginName = m_ui->displayPlugin->currentData().toString();

			if(QStringLiteral("SixDigitsPlugin") == pluginName) {
				uri += QStringLiteral("&digits=6");
			}
			else if(QStringLiteral("EightDigitsPlugin") == pluginName) {
				uri += QStringLiteral("&digits=8");
			}

			QrCodeCreator creator(uri);
			QImage img(creator.image(QSize(128, 128)));

			if(!img.save(fileName)) {
				QMessageBox::warning(this, tr("%1: Error").arg(Application::applicationName()), tr("Failed to save the QR code image to %1.").arg(fileName));
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to save qr code image to \"" << qPrintable(fileName) << "\"\n";
				return false;
			}

			qonvinceApp->showNotification(Application::applicationName(), tr("QR code created successfully."));
			return true;
		}

		QMessageBox::warning(this, tr("%1: Error").arg(Application::applicationName()), tr("You must enter a seed in order to create a QR code image.").arg(fileName));
		std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no code for which to create a qr-code\n";
		return false;
	}


	void OtpEditor::updateHeading() {
		QString issuer(m_ui->issuerEdit->text());
		QString name(m_ui->nameEdit->text());
		QString heading;

		switch(qonvinceApp->settings().codeLabelDisplayStyle()) {
			case CodeLabelDisplayStyle::NameOnly:
				heading = name;
				break;

			case CodeLabelDisplayStyle::IssuerOnly:
				heading = issuer;
				break;

			case CodeLabelDisplayStyle::IssuerAndName:
				if(!name.isEmpty()) {
					if(!issuer.isEmpty()) {
						heading = issuer % ": " % name;
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
			m_otp->setSeed(m_originalSeed);
		}
		else {
			m_otp->setSeed(m_ui->seedEdit->text().toUtf8(), Otp::SeedType::Base32);
		}
	}


	void OtpEditor::setCodeBaseTimeFromWidget() {
		m_otp->setBaselineTime(m_ui->baseTimeEdit->dateTime());
	}


	void OtpEditor::setCounter(quint64 value) {
		if(static_cast<int>(value) != m_ui->counterSpin->value()) {
			m_ui->counterSpin->setValue(static_cast<int>(value));
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
		if(!m_otp) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no code for which to set plugin\n";
			return;
		}

		auto pluginName = m_ui->displayPlugin->currentData().toString();

		if(!qonvinceApp->otpDisplayPluginByName(m_ui->displayPlugin->currentData().toString())) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: display plugin \"" << qPrintable(m_ui->displayPlugin->currentData().toString()) << "\" (" << qPrintable(m_ui->displayPlugin->currentText()) << ") not found\n";
			return;
		}

		m_otp->setDisplayPluginName(pluginName);
	}


	void OtpEditor::onIconSelected(const QIcon & ic) {
		m_otp->setIcon(ic);
		Q_EMIT iconChanged(ic);
	}


	void OtpEditor::onIconCleared() {
		m_otp->setIcon({});
		Q_EMIT iconChanged({});
	}


	void OtpEditor::setType(OtpType codeType) {
		if(codeType != type()) {
			QSignalBlocker hBlocker(m_ui->hotpButton);
			QSignalBlocker tBlocker(m_ui->totpButton);

			if(OtpType::Hotp == codeType) {
				m_ui->hotpButton->setChecked(true);
				m_ui->totpButton->setChecked(false);
				m_ui->intervalSpin->setEnabled(false);
				m_ui->baseTimeEdit->setEnabled(false);
				m_ui->counterSpin->setEnabled(true);
			}
			else if(OtpType::Totp == codeType) {
				m_ui->hotpButton->setChecked(false);
				m_ui->totpButton->setChecked(true);
				m_ui->intervalSpin->setEnabled(true);
				m_ui->baseTimeEdit->setEnabled(true);
				m_ui->counterSpin->setEnabled(false);
			}
		}

		if(m_otp && codeType != m_otp->type()) {
			m_otp->setType(codeType);
		}
	}


	void OtpEditor::setRevealOnDemand(bool onlyOnDemand) {
		if(onlyOnDemand != revealOnDemand()) {
			QSignalBlocker b(m_ui->revealOnDemand);
			m_ui->revealOnDemand->setChecked(onlyOnDemand);
		}

		if(m_otp && m_otp->revealCodeOnDemand() != onlyOnDemand) {
			m_otp->setRevealOnDemand(onlyOnDemand);
		}
	}


}  // namespace Qonvince
