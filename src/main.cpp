/*
 * Copyright 2015 - 2017 Darren Edale
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

#include "application.h"

#include <QDebug>

#include "crypt.h"


using namespace Qonvince;


int main( int argc, char *argv[] ) {
//	Crypt c("mykeyislongenough");
//	QString in("my secret text");
//	QString out = c.encrypt(in);

//	qDebug() << in << "encrypts to" << out;

//	in = c.decrypt(out).toUtf8();
//	qDebug() << out << "decrypts to" << in;
//	return 0;

	Application q(argc, argv);
	return q.exec();
}
