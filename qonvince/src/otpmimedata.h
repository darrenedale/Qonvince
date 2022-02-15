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

#ifndef QONVINCE_OTPMIMEDATA_H
#define QONVINCE_OTPMIMEDATA_H

#include <optional>
#include <vector>
#include <QMimeData>
#include <QByteArray>
#include <nlohmann/json.hpp>

namespace Qonvince
{
    class Application;
    class Otp;

    /**
     * @brief Specialisation of QMimeData for Otp export/drag-drop.
     *
     *
     */
    class OtpMimeData
    : public QMimeData
    {
        Q_OBJECT

    public:
        static const QString OtpJsonMimeType;
        static const QString OtpIndicesMimeType;

        /**
         * Initialise OtpMimeData with the JSON representation of a single Otp and optionally its index.
         */
        explicit OtpMimeData(Otp *, const std::optional<int> & index = {}) noexcept;

        /**
         * Initialise OtpMimeData with the JSON representation of one or more Otps.
         */
        explicit OtpMimeData(const std::vector<Otp *> & otps, const std::optional<std::vector<int>> & indices = {}) noexcept;

        /**
         * Initialise OtpMimeData with the pre-built JSON representation of one or more Otps and optionally their indices.
         */
        explicit OtpMimeData(nlohmann::json && otpJson, const std::optional<std::vector<int>> & indices = {}) noexcept;

        /**
         * Initialise OtpMimeData with the pre-built stringified JSON representation of one or more Otps and optionally their indices.
         */
        explicit OtpMimeData(std::string && otpJsonStr, const std::optional<std::vector<int>> & indices = {}) noexcept;

        /**
         * Initialise OtpMimeData with a single Otp index.
         */
        explicit OtpMimeData(int index) noexcept;

        /**
         * Initialise OtpMimeData with one or more Otp indices.
         */
        explicit OtpMimeData(const std::vector<int> & indices) noexcept;

        /**
         * Destroy the MIME data.
         */
        ~OtpMimeData() noexcept override;

        /**
         * Fetch the stringified JSON representation of the Otps in the MIME data, if present.
         *
         * @return The JSON as a QByteArray, or an empty optional if not present.
         */
        [[nodiscard]] std::optional<QByteArray> otpListJsonString() const;

        /**
         * Fetch the JSON representation of the Otps in the MIME data, if present.
         *
         * @return The JSON, or an empty optional if not present.
         */
        [[nodiscard]] std::optional<nlohmann::json> otpListJson() const;

        /**
         * Fetch the Otps in the MIME data, if present.
         *
         * @return A vector of the Otps, or an empty optional if not present.
         */
        [[nodiscard]] std::optional<std::vector<std::unique_ptr<Otp>>> otpList() const;

        /**
         * Fetch the index of the OTP, if present.
         *
         * If the MIME data contains more than one index, only the first index is returned.
         *
         * @return The index, or an empty optional if the MIME data does not contain an index/indices.
         */
        [[nodiscard]] std::optional<int> index() const;

        /**
         * Fetch the indices of the OTPs, if present.
         *
         * @return The indices, or an empty optional if the MIME data does not contain indices.
         */
        [[nodiscard]] std::optional<std::vector<int>> indices() const;

        /**
         * Check whether the MIME data contains indices of Otps.
         *
         * @return true if the MIME data contains indices, false otherwise.
         */
        [[nodiscard]] inline bool hasIndices() const
        {
            return hasFormat(OtpIndicesMimeType);
        }

        /**
         * Check whether the MIME data contains the JSON representation of Otps.
         *
         * @return true if the MIME data contains JSON Otp details, false otherwise.
         */
        [[nodiscard]] inline bool hasOtpList() const
        {
            return hasFormat(OtpJsonMimeType);
        }

        /**
         * The Application that's managing the list of Otps.
         *
         * This is useful when Otps are reordered - the indices can be used to swap the Otps in the Application's list rather
         * than removing Otps and inserting new ones.
         *
         * @return
         */
        [[nodiscard]] inline Application * origin() const
        {
            return m_origin;
        }

    private:
        // helpers for the constructors
        void setIndices(const std::vector<int> & indices);
        void setOtps(const std::vector<Otp *> & otps);

        // the application from which the Otps originated. if the receiver is in the same application, it can use the
        // indices to just re-order the Otps on drop
        Application * m_origin;
    };
} // Qonvince

#endif //QONVINCE_OTPMIMEDATA_H
