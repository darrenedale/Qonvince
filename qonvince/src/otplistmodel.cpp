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

/**
 * @file otplistmodel.cpp
 * @author Darren Edale
 * @date December 2017
 *
 * @brief Implementation of the OtpListModel class.
 */

#include "otplistmodel.h"

#include <numeric>
#include <QString>
#include <QStringBuilder>
#include <QMimeData>
#include "application.h"
#include "otp.h"
#include "settings.h"
#include "functions.h"
#include "otpmimedata.h"

namespace Qonvince
{
	OtpListModel::OtpListModel()
    : QAbstractListModel()
	{
		Q_ASSERT_X(qonvinceApp, __PRETTY_FUNCTION__, "it is not possible to create an OtpListModel without a Qonvince::Application instance");

		// adding/removing Otp is done directly on Application object
		// these connections ensure that the model emits the appropriate
		// signals when this occurs
		// the model itself is read-only
		connect(qonvinceApp, qOverload<int, Otp *>(&Application::otpAdded), this, [this](int index) {
			beginInsertRows({}, index, index);
			endInsertRows();
		});

		connect(qonvinceApp, qOverload<int>(&Application::otpRemoved), this, [this](int index) {
			beginRemoveRows({}, index, index);
			endRemoveRows();
		});

		connect(qonvinceApp, qOverload<int>(&Application::otpChanged), this, [this](int otpIndex) {
			const auto itemIndex = index(otpIndex, 0);
			Q_EMIT dataChanged(itemIndex, itemIndex);
		});
	}

	QVariant OtpListModel::headerData(int section, Qt::Orientation, int role) const
	{
		if(0 == section && role == Qt::DisplayRole) {
			return tr("Otp");
		}

		return {};
	}

	QVariant OtpListModel::data(const QModelIndex & index, int role) const
	{
		auto * otp = qonvinceApp->otp(index.row());

		if(!otp) {
			return {};
		}

		switch(role) {
			case Qt::EditRole:
				return data(index, LabelRole);

			case Qt::DisplayRole:
				return static_cast<QString>(data(index, LabelRole).toString() % ' ' % data(index, CodeRole).toString());

			case OtpRole:
				return QVariant::fromValue(otp);

			case TypeRole:
				return static_cast<int>(otp->type());

			case NameRole:
				return otp->name();

			case IssuerRole:
				return otp->issuer();

			case LabelRole:
				return otpLabel(otp);

			case CodeRole:
				return otp->code();

			case IconRole:
				return otp->icon();

			case TimeToNextCodeRole:
				return otp->timeToNextCode();

			case TimeSinceLastCodeRole:
				return otp->timeSinceLastCode();

			case IntervalRole:
				return otp->interval();

			case CountdownRole:
				return otp->counter();

			case RevealOnDemandRole:
				return otp->revealCodeOnDemand();

			case IsRevealedRole:
				return otp->codeIsVisible();

			default:
				// deliberately empty default case label
				break;
		}

		return {};
	}

	int OtpListModel::rowCount(const QModelIndex &) const
	{
		return qonvinceApp->otpCount();
	}

	Qt::DropActions OtpListModel::supportedDropActions() const
	{
		return Qt::DropAction::MoveAction | Qt::DropAction::CopyAction;
	}

	Qt::ItemFlags OtpListModel::flags(const QModelIndex & idx) const
	{
        auto flags = QAbstractListModel::flags(idx) | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsSelectable;

        if (idx.isValid()) {
            flags |= Qt::ItemFlag::ItemIsDragEnabled;
        }

        return flags | Qt::ItemFlag::ItemIsDropEnabled;
	}

	QStringList OtpListModel::mimeTypes() const
	{
        return {OtpMimeData::OtpJsonMimeType, OtpMimeData::OtpIndicesMimeType,};
	}

	QMimeData * OtpListModel::mimeData(const QModelIndexList & indices) const
	{
        std::vector<int> rows;
        std::vector<Otp *> otps;

        rows.reserve(indices.size());
        otps.reserve(indices.size());

        for (const QModelIndex & index : indices) {
            rows.push_back(index.row());
            otps.push_back(qonvinceApp->otp(index.row()));
        }

		return new OtpMimeData(otps, rows);
	}

	bool OtpListModel::canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int col, const QModelIndex & parent) const
	{
		return 0 == col && (data->hasFormat(OtpMimeData::OtpJsonMimeType) || data->hasFormat(OtpMimeData::OtpIndicesMimeType));
	}

	bool OtpListModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
	{
		if(!canDropMimeData(data, action, row, column, parent)) {
			return false;
		}

        auto * otpMimeData = qobject_cast<const OtpMimeData *>(data);

        if (otpMimeData) {
            if (otpMimeData->origin() == qonvinceApp && Qt::DropAction::MoveAction == action && otpMimeData->hasIndices()) {
                for (const auto indices = *(otpMimeData->indices()); int idx : indices) {
                    if (idx < row) {
                        --row;
                    }

                    qonvinceApp->moveOtp(idx, row);
                }
            } else {
                for (auto otps = *(otpMimeData->otpList()); auto & otpJson : otps) {
                    qonvinceApp->insertOtp(row, std::move(otpJson));
                }
            }
        }
        else if (data->hasFormat(OtpMimeData::OtpJsonMimeType)){
            // extract the dragged OTPs from the MIME data
            for (const auto & otpJson : json::parse(data->data(OtpMimeData::OtpJsonMimeType).toStdString())) {
                qonvinceApp->insertOtp(row, Otp::fromJson(otpJson));
            }
        }

		return true;
	}
}	// namespace Qonvince
