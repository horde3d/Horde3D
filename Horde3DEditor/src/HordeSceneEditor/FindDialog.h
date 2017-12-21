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

#ifndef FINDDIALOG_H_
#define FINDDIALOG_H_

#include "ui_FindDialog.h"

class FindDialog : public QDialog, protected Ui_FindDialog
{
	Q_OBJECT
public:
	static FindDialog* showFindDialog(QWidget* parent = 0);
	static void releaseDialog();

	virtual ~FindDialog();

	void accept();
	
signals:
	void find(const QString&);

private:
	FindDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	
	static FindDialog* m_instance;


};
#endif

