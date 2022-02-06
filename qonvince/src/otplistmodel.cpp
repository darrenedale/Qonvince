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

/** \file otplistmodel.cpp
  * \author Darren Edale
  * \date December 2017
  *
  * \brief Implementation of the OtpListModel class.
  */

#include "otplistmodel.h"

#include <QString>
#include <QStringBuilder>

#include "application.h"
#include "otp.h"
#include "settings.h"
#include "functions.h"

namespace Qonvince
{
    OtpListModel::OtpListModel()
    {
        Q_ASSERT_X(qonvinceApp, __PRETTY_FUNCTION__, "it is not possible to create an OtpListModel without a Qonvince::Application instance");

        // adding/removing Otp is done directly on Application object
        // these connections ensure that the model emits the appropriate
        // signals when this occurs
        // the model itself is read-only
        connect(qonvinceApp, qOverload<int, Otp *>(&Application::otpAdded), this, [this](int index){
            beginInsertRows({}, index, index);
            endInsertRows();
        });

        connect(qonvinceApp, qOverload<int>(&Application::otpRemoved), this, [this](int index){
            beginRemoveRows({}, index, index);
            endRemoveRows();
        });

        connect(qonvinceApp, qOverload<int>(&Application::otpChanged), this, [this](int otpIndex){
            const auto itemIndex = index(otpIndex, 0);
            Q_EMIT dataChanged(itemIndex, itemIndex);
        });
    }

    QVariant OtpListModel::headerData(int section, Qt::Orientation, int role) const
    {
        if (0 == section && role == Qt::DisplayRole) {
            return tr("Otp");
        }

        return {};
    }

    QVariant OtpListModel::data(const QModelIndex & index, int role) const
    {
        auto * otp = qonvinceApp->otp(index.row());

        if (!otp) {
            return {};
        }

        switch (role) {
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
}  // namespace Qonvince
