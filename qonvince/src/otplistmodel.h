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

#ifndef QONVINCE_OTPLISTMODEL_H
#define QONVINCE_OTPLISTMODEL_H

#include <QAbstractListModel>

namespace Qonvince
{
    class OtpListModel
            : public QAbstractListModel
    {
    public:
        static constexpr const int OtpRole = Qt::UserRole + 1;
        static constexpr const int TypeRole = Qt::UserRole + 2;
        static constexpr const int NameRole = Qt::UserRole + 3;
        static constexpr const int IssuerRole = Qt::UserRole + 4;
        static constexpr const int LabelRole = Qt::UserRole + 5;
        static constexpr const int CodeRole = Qt::UserRole + 6;
        static constexpr const int IconRole = Qt::UserRole + 7;
        static constexpr const int TimeToNextCodeRole = Qt::UserRole + 8;
        static constexpr const int TimeSinceLastCodeRole = Qt::UserRole + 9;
        static constexpr const int IntervalRole = Qt::UserRole + 10;
        static constexpr const int CountdownRole = Qt::UserRole + 11;
        static constexpr const int RevealOnDemandRole = Qt::UserRole + 12;
        static constexpr const int IsRevealedRole = Qt::UserRole + 13;

        OtpListModel();

		[[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		[[nodiscard]] QVariant data(const QModelIndex & idx, int role) const override;
		[[nodiscard]] int rowCount(const QModelIndex & parent) const override;
		[[nodiscard]] Qt::DropActions supportedDropActions() const override;
		[[nodiscard]] Qt::ItemFlags flags(const QModelIndex & idx) const override;
		[[nodiscard]] QStringList mimeTypes() const override;
		[[nodiscard]] QMimeData * mimeData(const QModelIndexList & idx) const override;
		[[nodiscard]] bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int col, const QModelIndex & parent) const override;
		bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;
	};

}  // namespace Qonvince

#endif  // QONVINCE_OTPLISTMODEL_H
