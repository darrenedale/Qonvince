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

#include "otpdisplaypluginchooser.h"

#include "application.h"

namespace Qonvince
{
    OtpDisplayPluginChooser::OtpDisplayPluginChooser(QWidget * parent)
            : QComboBox(parent)
    {
        Q_ASSERT_X(qonvinceApp, __PRETTY_FUNCTION__, "can't create an OtpDisplayPluginChooser without a Qonvince::Application object");

        refreshPlugins();

        connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int){
            Q_EMIT currentPluginChanged(currentPluginName());
        });
    }

    LibQonvince::OtpDisplayPlugin * OtpDisplayPluginChooser::currentPlugin() const
    {
        return qonvinceApp->otpDisplayPluginByName(currentPluginName());
    }

    void OtpDisplayPluginChooser::setCurrentPluginName(const QString & pluginName)
    {
        setCurrentIndex(findData(pluginName));
    }

    void OtpDisplayPluginChooser::refreshPlugins()
    {
        auto reblock = blockSignals(true);
        auto current = currentPluginName();
        clear();

        for (auto * plugin: qonvinceApp->otpDisplayPlugins()) {
            QComboBox::addItem(QString::fromStdString(plugin->displayName()), QString::fromStdString(plugin->name()));
        }

        if (!current.isEmpty()) {
            setCurrentPluginName(current);
        }

        blockSignals(reblock);

        if (current != currentPluginName()) {
            Q_EMIT currentPluginChanged(currentPluginName());
        }
    }
}  // namespace Qonvince
