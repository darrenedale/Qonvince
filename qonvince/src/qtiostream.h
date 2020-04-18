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
#ifndef QONVINCE_QTIOSTREAM_H
#define QONVINCE_QTIOSTREAM_H

#include <iostream>

class QString;
class QStringRef;
class QUrl;
class QPoint;
class QRect;

namespace Qonvince {
	std::ostream & operator<<(std::ostream & out, const QString &);
	std::ostream & operator<<(std::ostream & out, const QStringRef &);
	std::ostream & operator<<(std::ostream & out, const QUrl &);
	std::ostream & operator<<(std::ostream & out, const QPoint &);
	std::ostream & operator<<(std::ostream & out, const QRect &);
}  // namespace Qonvince

#endif  // QONVINCE_QTIOSTREAM_H
