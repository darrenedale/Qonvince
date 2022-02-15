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

#include "otpmimedata.h"
#include <QtGlobal>
#include "application.h"
#include "otp.h"

using nlohmann::json;

namespace Qonvince
{
    const QString OtpMimeData::OtpJsonMimeType = QStringLiteral("application/vnd.equit.qonvince.otp.json");
    const QString OtpMimeData::OtpIndicesMimeType = QStringLiteral("application/vnd.equit.qonvince.otp.indices");

    OtpMimeData::OtpMimeData(Otp * otp, const std::optional<int> & index) noexcept
    : m_origin(qonvinceApp)
    {
        Q_ASSERT_X(otp, __PRETTY_FUNCTION__ , "constructor expects a non-null Otp pointer.");
        auto otps = json::array();
        otps.push_back(otp->toJson());
        setData(OtpJsonMimeType, QByteArray::fromStdString(otps.dump()));

        if (index) {
            setData(OtpIndicesMimeType, QByteArray::number(*index));
        }
    }

    OtpMimeData::OtpMimeData(const std::vector<Otp *> & otps, const std::optional<std::vector<int>> & indices) noexcept
    : m_origin(qonvinceApp)
    {
        Q_ASSERT_X(!otps.empty(), __PRETTY_FUNCTION__ , "constructor expects at least one Otp.");
        Q_ASSERT_X(std::all_of(otps.cbegin(), otps.cend(), [](Otp * otp) -> bool {
            return otp;
        }), __PRETTY_FUNCTION__ , "constructor expects all Otps to be non-null.");
        Q_ASSERT_X(!indices || indices->size() == otps.size(), __PRETTY_FUNCTION__ , "constructor expects no indices or the same number of indices as Otps.");
        Q_ASSERT_X(!indices || std::all_of(indices->cbegin(), indices->cend(), [](int index) -> bool {
            return 0 <= index;
        }), __PRETTY_FUNCTION__ , "constructor expects no indices or all indices to be >= 0.");

        setOtps(otps);

        if (indices) {
            setIndices(*indices);
        }
    }

    OtpMimeData::OtpMimeData(nlohmann::json && otpJson, const std::optional<std::vector<int>> & indices) noexcept
    : m_origin(qonvinceApp)
    {
        setData(OtpJsonMimeType, QByteArray::fromStdString(otpJson.dump()));

        if (indices) {
            setIndices(*indices);
        }
    }

    OtpMimeData::OtpMimeData(std::string && otpJsonStr, const std::optional<std::vector<int>> & indices) noexcept
    : m_origin(qonvinceApp)
    {
        setData(OtpJsonMimeType, QByteArray::fromStdString(otpJsonStr));

        if (indices) {
            setIndices(*indices);
        }
    }

    OtpMimeData::OtpMimeData(int index) noexcept
            : m_origin(qonvinceApp)
    {
        Q_ASSERT_X(0 <= index, __PRETTY_FUNCTION__ , "constructor expects an index >= 0.");
        setData(OtpIndicesMimeType, QByteArray::number(index));
    }

    OtpMimeData::OtpMimeData(const std::vector<int> & indices) noexcept
            : m_origin(qonvinceApp)
    {
        Q_ASSERT_X(!indices.empty(), __PRETTY_FUNCTION__ , "constructor expects at least one index.");
        Q_ASSERT_X(std::all_of(indices.cbegin(), indices.cend(), [](int index) -> bool {
            return 0 <= index;
        }), __PRETTY_FUNCTION__ , "constructor expects all indices to be >= 0");
        setIndices(indices);
    }

    OtpMimeData::~OtpMimeData() noexcept = default;

    void OtpMimeData::setIndices(const std::vector<int> & indices)
    {
        setData(OtpIndicesMimeType,
        std::transform_reduce(indices.cbegin(), indices.cend(),
              QByteArray(),
              [](const QByteArray & init, QByteArray index) -> QByteArray {
                  if (init.isEmpty()) {
                      return index;
                  }

                  return init + ' ' + index;
              },
              [](int index) -> QByteArray {
                  return QByteArray::number(index);
              })
        );
    }

    void OtpMimeData::setOtps(const std::vector<Otp *> & otps)
    {
        auto otpJson = json::array();

        for (const auto * otp : otps) {
            otpJson.push_back(otp->toJson());
        }

        setData(OtpJsonMimeType, QByteArray::fromStdString(otpJson.dump()));
    }

    std::optional<QByteArray> OtpMimeData::otpListJsonString() const
    {
        if (!hasFormat(OtpJsonMimeType)) {
            return {};
        }

        return data(OtpJsonMimeType);
    }

    std::optional<json> OtpMimeData::otpListJson() const
    {
        if (!hasFormat(OtpJsonMimeType)) {
            return {};
        }

        return json::parse(data(OtpJsonMimeType));
    }

    std::optional<std::vector<std::unique_ptr<Otp>>> OtpMimeData::otpList() const
    {
        const auto otpJson = otpListJson();

        if (!otpJson) {
            return {};
        }

        std::vector<std::unique_ptr<Otp>> otps;
        otps.reserve(otpJson->size());

        std::transform(otpJson->cbegin(), otpJson->cend(), std::back_inserter(otps), [](const json & otpJson) -> std::unique_ptr<Otp>
        {
            return Otp::fromJson(otpJson);
        });

        return otps;
    }

    std::optional<int> OtpMimeData::index() const
    {
        if (!hasFormat(OtpIndicesMimeType)) {
            return {};
        }

        return data(OtpIndicesMimeType).split(' ')[0].toInt();
    }

    std::optional<std::vector<int>> OtpMimeData::indices() const
    {
        if (!hasFormat(OtpIndicesMimeType)) {
            return {};
        }

        std::vector<int> ret;
        auto strIndices = data(OtpIndicesMimeType).split(' ');
        ret.reserve(strIndices.size());

        std::transform(strIndices.begin(), strIndices.end(), std::back_inserter(ret), [](const QByteArray & strIndex) -> int {
            return strIndex.toInt();
        });

        return ret;
    }
} // Qonvince