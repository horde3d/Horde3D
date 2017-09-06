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

#ifndef SCENEFILEPAGE_H_
#define SCENEFILEPAGE_H_

#include "ui_SceneFilePage.h"
#include <QWizard>

/**
 * Wizard Page for creating a new SceneFile
 */
class SceneFilePage : public QWizardPage, protected Ui_SceneFilePage
{
	Q_OBJECT
public:
	SceneFilePage(QWidget* parent = 0);
	virtual ~SceneFilePage();

	bool validatePage();

	void initializePage();

private slots:
	void setScenePath();

};
#endif
