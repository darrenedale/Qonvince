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

#ifndef QONVINCE_QRCODECREATOR_H
#define QONVINCE_QRCODECREATOR_H

#include <QString>
#include <QImage>
#include <QColor>

#include "libqrencode.h"

class QPainter;

namespace Qonvince
{
    class QrCodeCreator
    {
    public:
        static const QColor DefaultForeground;
        static const QColor DefaultBackground;

        explicit QrCodeCreator(const QString & data);
        ~QrCodeCreator();

        inline static bool isAvailable()
        {
            return s_libQrEncode.isOpen();
        }

        inline void setData(const QString & data)
        {
            m_data = data;
        }

        [[nodiscard]] inline const QString & data() const
        {
            return m_data;
        }

        QImage image(const QSize & size);

        void paint(QPainter & painter, const QSize & size, const QColor & fg = DefaultForeground, const QColor & bg = DefaultBackground);

    private:
        static LibQrEncode s_libQrEncode;
        QString m_data;
    };
}  // namespace Qonvince

#endif  // QONVINCE_QRCODECREATOR_H
