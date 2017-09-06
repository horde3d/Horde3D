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

#ifndef ABOUTDIALOG_H_
#define ABOUTDIALOG_H_

#include "ui_AboutDialog.h"

/**
 * Simple dialog to show some information about the application
 */
class AboutDialog : public QDialog, protected Ui_AboutDialog
{
	Q_OBJECT
public:
	/**
	 * Constructor
	 * @param message optional string to change the dialogs information message 
	 * @param parent parent widget for the dialog
	 * @param flags optional flags to change dialog settings
	 */
	AboutDialog(const QString& message =QString(), QWidget* parent = 0, Qt::WindowFlags flags = Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
	/// Destructor
	virtual ~AboutDialog();


};
#endif
