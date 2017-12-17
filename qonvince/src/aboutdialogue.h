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

#ifndef QONVINCE_ABOUTDIALOGUE_H
#define QONVINCE_ABOUTDIALOGUE_H

#include <memory>

#include <QDialog>

namespace Qonvince {

	namespace Ui {
		class AboutDialogue;
	}

	class AboutDialogue
	: public QDialog {
			// Q_OBJECT macro technically only necessary if signals/slots/properties defined
			// Q_OBJECT

	public:
		explicit AboutDialogue(QWidget * parent = nullptr);
		~AboutDialogue();

	private:
		std::unique_ptr<Ui::AboutDialogue> m_ui;
	};

}  // namespace Qonvince

#endif  // QONVINCE_ABOUTDIALOGUE_H
