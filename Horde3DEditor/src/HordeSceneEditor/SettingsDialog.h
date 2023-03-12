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

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include "ui_SettingsDialog.h"

class QSignalMapper;

/**
 * Configuration dialog for some options of the HordeSceneEditor application
 *
 * The configuration dialog configures the repository and plug in pathes as well as the
 * editor used when clicking on the edit button of a shader attribute
 */
class SettingsDialog : public QDialog, protected Ui_SettingsDialog
{
	Q_OBJECT
public:
	SettingsDialog(QWidget* parent = 0, Qt::WindowFlags flags = (Qt::WindowFlags) 0);
	virtual ~SettingsDialog();

private slots:
	void setPath(QWidget* path);
	void restart() {m_restart = true;}
	void saveSettings();
	void setShaderEditor();
	void updateButtons();

private:
	QSignalMapper*		m_mapper;
	bool				m_restart;

};
#endif
