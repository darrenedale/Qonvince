/*
 * Copyright 2018 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "functions.h"

#include <QStringBuilder>

#include "application.h"
#include "settings.h"
#include "otp.h"


namespace Qonvince {

	QString otpLabel(Otp * otp) {
		switch(qonvinceApp->settings().codeLabelDisplayStyle()) {
			case Settings::NameOnly:
				return otp->name();

			case Settings::IssuerOnly:
				return otp->issuer();

			case Settings::IssuerAndName:
				if(const auto & name = otp->name(); !name.isEmpty()) {
					if(const auto & issuer = otp->issuer(); !issuer.isEmpty()) {
						return issuer % QStringLiteral(": ") % name;
					}

					return name;
				}

				return otp->issuer();
		}

		return QStringLiteral("");
	}

}  // namespace Qonvince
