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

#ifndef QONVINCE_OTPEDITORDIALOGUE_H
#define QONVINCE_OTPEDITORDIALOGUE_H

#include <memory>

#include <QDialog>

#include "otp.h"

namespace Qonvince {

	class OtpEditor;

	namespace Ui {
		class OtpEditorDialogue;
	}

	class OtpEditorDialogue
	: public QDialog {
		Q_OBJECT

	public:
		explicit OtpEditorDialogue(QWidget * = nullptr);
		explicit OtpEditorDialogue(Otp *, QWidget * = nullptr);
		~OtpEditorDialogue() override;

		OtpEditor * editor();
		Otp * otp();

	private:
		std::unique_ptr<Ui::OtpEditorDialogue> m_ui;
	};

}  // namespace Qonvince

#endif  // QONVINCE_OTPEDITORDIALOGUE_H
