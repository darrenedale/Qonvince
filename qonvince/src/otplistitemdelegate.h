/*
 * Copyright 2015 - 2020 Darren Edale
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

#ifndef QONVINCE_OTPLISTITEMDELEGATE_H
#define QONVINCE_OTPLISTITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace Qonvince
{
    // Q_OBJECT macro is omitted because currently it's only required if using signals/slots/properties
    class OtpListItemDelegate
            : public QStyledItemDelegate
    {
    public:
        // doesn't take parent arg because we only ever use objects of this class as
        // smart pointers
        OtpListItemDelegate();

        void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
        [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

        inline const QColor & countdownWarningColour() const
        {
            return m_countdownWarningColour;
        }

        inline const QColor & countdownCriticalColour() const
        {
            return m_countdownCriticalColour;
        }

        inline void setCountdownWarningColour(const QColor & colour)
        {
            if (colour.isValid()) {
                m_countdownWarningColour = colour;
                return;
            }

            // invalid means no custom colour
            m_countdownWarningColour = {-1, -1, -1};
        }

        inline void setCountdownCriticalColour(const QColor & colour)
        {
            if (colour.isValid()) {
                m_countdownCriticalColour = colour;
                return;
            }

            // invalid means no custom colour
            m_countdownCriticalColour = {-1, -1, -1};
        }

        [[nodiscard]] inline constexpr int actionIconAreaWidth() const
        {
            return m_actionIconAreaWidth;
        }

        inline constexpr void setActionIconAreaWidth(int width)
        {
            m_actionIconAreaWidth = width;
        }

    private:
        QColor m_countdownWarningColour;
        QColor m_countdownCriticalColour;
        int m_actionIconAreaWidth;
    };
}  // namespace Qonvince

#endif  // QONVINCE_OTPLISTITEMDELEGATE_H
