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

#ifndef OTPDISPLAYPLUGINCHOOSER_H
#define OTPDISPLAYPLUGINCHOOSER_H

#include <QObject>
#include <QComboBox>
#include <QString>

namespace LibQonvince
{
    class OtpDisplayPlugin;
}

namespace Qonvince
{
    class OtpDisplayPluginChooser
            : public QComboBox
    {
    Q_OBJECT
    public:
        explicit OtpDisplayPluginChooser(QWidget * parent = nullptr);

        void addItem() = delete;
        void addItems() = delete;
        void setItemData() = delete;
        void setItemIcon() = delete;
        void setItemText() = delete;

        [[nodiscard]] inline QString currentPluginName() const
        {
            return currentData().toString();
        }

        [[nodiscard]] LibQonvince::OtpDisplayPlugin * currentPlugin() const;

    public Q_SLOTS:
        void setCurrentPluginName(const QString & pluginName);

    Q_SIGNALS:
        void currentPluginChanged(const QString & pluginName);

    private Q_SLOTS:
        void refreshPlugins();
    };
}  // namespace Qonvince

#endif  // OTPDISPLAYPLUGINCHOOSER_H
