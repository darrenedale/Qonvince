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
#include "qtiostream.h"

#include <QString>
#include <QUrl>
#include <QRect>
#include <QPoint>


namespace Qonvince {


	std::ostream & operator<<(std::ostream & out, const QString & str) {
		out << qPrintable(str);
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QStringRef & str) {
		out << str.toString();
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QUrl & url) {
		out << url.toString();
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QRect & rect) {
		out << rect.topLeft() << '-' << rect.bottomRight();
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QPoint & point) {
		out << '(' << point.x() << ", " << point.y() << ')';
		return out;
	}


}  // namespace Qonvince
