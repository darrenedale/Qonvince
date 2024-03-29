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

/** \file settingswidget.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the SettingsWidget class.
  */

#include "settingswidget.h"
#include "ui_settingswidget.h"

#include <iostream>

#include "types.h"
#include "settings.h"

namespace Qonvince
{
    SettingsWidget::SettingsWidget(Settings & settings, QWidget * parent)
            : QWidget(parent),
              m_ui{std::make_unique<Ui::SettingsWidget>()},
              m_settings(settings)
    {
        m_ui->setupUi(this);
        resyncWithSettings();
        connect(m_ui->singleInstance, &QCheckBox::toggled, &m_settings, &Settings::setSingleInstance);
        connect(m_ui->quitOnMainWindowClose, &QCheckBox::toggled, &m_settings, &Settings::setQuitOnMainWindowClosed);
        connect(m_ui->startMinimised, &QCheckBox::toggled, &m_settings, &Settings::setStartMinimised);
        connect(m_ui->copyCodeOnClick, &QCheckBox::toggled, &m_settings, &Settings::setCopyCodeOnClick);
        connect(m_ui->hideOnCodeCopyClick, &QCheckBox::toggled, &m_settings, &Settings::setHideOnCodeCopyClick);
        connect(m_ui->clearClipboardAfterInterval, &QCheckBox::toggled, &m_settings, &Settings::setClearClipboardAfterInterval);
        connect(m_ui->clipboardClearInterval, qOverload<int>(&QSpinBox::valueChanged), &m_settings, &Settings::setClipboardClearInterval);
        connect(m_ui->codeLabelDisplayStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingsWidget::onDisplayStyleWidgetChanged);
        connect(m_ui->revealTimeout, qOverload<int>(&QSpinBox::valueChanged), &m_settings, &Settings::setCodeRevealTimeout);
        connect(&m_settings, &Settings::changed, this, &SettingsWidget::resyncWithSettings);
    }

    SettingsWidget::~SettingsWidget() = default;

    void SettingsWidget::resyncWithSettings()
    {
        QSignalBlocker blocker(this);
        setSingleInstance(m_settings.singleInstance());
        setQuitOnMainWindowClose(m_settings.quitOnMainWindowClosed());
        setStartMinimised(m_settings.startMinimised());
        setCopyCodeOnClick(m_settings.copyCodeOnClick());
        setHideOnCodeCopyClick(m_settings.hideOnCodeCopyClick());
        setClearClipboardAfterInterval(m_settings.clearClipboardAfterInterval());
        setClipboardClearInterval(m_settings.clipboardClearInterval());
        setCodeLabelDisplayStyle(m_settings.codeLabelDisplayStyle());
        setCodeRevealTimeout(m_settings.codeRevealTimeout());
    }

    void SettingsWidget::onDisplayStyleWidgetChanged()
    {
        m_settings.setCodeLabelDisplayStyle(codeLabelDisplayStyle());
    }

    bool SettingsWidget::singleInstance() const
    {
        return m_ui->singleInstance->isChecked();
    }

    bool SettingsWidget::quitOnMainWindowClose() const
    {
        return m_ui->quitOnMainWindowClose->isChecked();
    }

    bool SettingsWidget::startMinimised() const
    {
        return m_ui->startMinimised->isChecked();
    }

    bool SettingsWidget::copyCodeOnClick() const
    {
        return m_ui->copyCodeOnClick->isChecked();
    }

    bool SettingsWidget::hideOnCodeCopyClick() const
    {
        return m_ui->hideOnCodeCopyClick->isChecked();
    }

    bool SettingsWidget::clearClipboardAfterInterval() const
    {
        return m_ui->clearClipboardAfterInterval->isChecked();
    }

    int SettingsWidget::clipboardClearInterval() const
    {
        return m_ui->clipboardClearInterval->value();
    }

    void SettingsWidget::setSingleInstance(bool single)
    {
        m_ui->singleInstance->setChecked(single);
    }

    void SettingsWidget::setQuitOnMainWindowClose(bool close)
    {
        m_ui->quitOnMainWindowClose->setChecked(close);
    }

    void SettingsWidget::setStartMinimised(bool minimised)
    {
        m_ui->startMinimised->setChecked(minimised);
    }

    void SettingsWidget::setCopyCodeOnClick(bool copy)
    {
        m_ui->copyCodeOnClick->setChecked(copy);
    }

    void SettingsWidget::setHideOnCodeCopyClick(bool hide)
    {
        m_ui->hideOnCodeCopyClick->setChecked(hide);
    }

    void SettingsWidget::setClearClipboardAfterInterval(bool clear)
    {
        m_ui->clearClipboardAfterInterval->setChecked(clear);
    }

    void SettingsWidget::setClipboardClearInterval(int interval)
    {
        m_ui->clipboardClearInterval->setValue(interval);
    }

    CodeLabelDisplayStyle SettingsWidget::codeLabelDisplayStyle() const
    {
        static CodeLabelDisplayStyle transTable[] = {CodeLabelDisplayStyle::IssuerAndName, CodeLabelDisplayStyle::NameOnly, CodeLabelDisplayStyle::IssuerOnly};
        int i = m_ui->codeLabelDisplayStyle->currentIndex();

        if (0 <= i && i < static_cast<int>(sizeof transTable)) {
            return transTable[i];
        }

        std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: unexpected display style current index (" << i << "), returning default style\n";
        return CodeLabelDisplayStyle::IssuerAndName;
    }

    int SettingsWidget::codeRevealTimeout() const
    {
        return m_ui->revealTimeout->value();
    }

    void SettingsWidget::setCodeLabelDisplayStyle(CodeLabelDisplayStyle style)
    {
        switch (style) {
            case CodeLabelDisplayStyle::IssuerAndName:
                m_ui->codeLabelDisplayStyle->setCurrentIndex(0);
                break;

            case CodeLabelDisplayStyle::NameOnly:
                m_ui->codeLabelDisplayStyle->setCurrentIndex(1);
                break;

            case CodeLabelDisplayStyle::IssuerOnly:
                m_ui->codeLabelDisplayStyle->setCurrentIndex(2);
                break;
        }
    }

    void SettingsWidget::setCodeRevealTimeout(int timeout)
    {
        if (m_ui->revealTimeout->minimum() > timeout || timeout > m_ui->revealTimeout->maximum()) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: attempt to set reveal timeout to invalid value\n";
            return;
        }

        m_ui->revealTimeout->setValue(timeout);
    }
}  // namespace Qonvince
