// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the GameEngine developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// The GameEngine is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The GameEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************


// ****************************************************************************************
//
// GameEngine Horde3D Editor Plugin of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#ifndef PATHPAGE_H_
#define PATHPAGE_H_

#include "Ui_PathPage.h"

#include <QtWidgets/qwizard.h>
class QSignalMapper;

/**
 * Class for configuring the GameEngine specific pathes
 */
class PathPage : public QWizardPage, protected Ui_PathPage
{
	Q_OBJECT
public:
	PathPage(QWidget* parent = 0);
	virtual ~PathPage();

	bool validatePage();
	void initializePage();

	void setDirectories( const QString& mediaDir, const QString& scriptDir );

private slots:
	void setDirectory(QWidget* edit);

private:
	QSignalMapper*	m_mapper;


};
#endif