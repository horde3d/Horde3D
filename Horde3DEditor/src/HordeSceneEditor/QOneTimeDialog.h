// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor.
//
// The Horde3D Scene Editor is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The Horde3D Scene Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************
#ifndef QONETIMEDIALOG_H_
#define QONETIMEDIALOG_H_

#include "./ui_QOneTimeDialog.h"

class QOneTimeDialog : public QDialog, protected Ui_QOneTimeDialog
{
	Q_OBJECT
public:
	
	static const int ApplyAlways;
	static int question(QWidget* parent, const QString& title, const QString& text, const QDialogButtonBox::StandardButton& button1,  const QDialogButtonBox::StandardButton& button2,  const QDialogButtonBox::StandardButton& button3 = QDialogButtonBox::NoButton);

private:
	QOneTimeDialog(QWidget* parent = 0, Qt::WindowFlags flags = (Qt::WindowFlags) 0);
	virtual ~QOneTimeDialog();

	QDialogButtonBox::StandardButton	m_button;

private slots:
	void buttonPressed(QAbstractButton* button);

};
#endif
