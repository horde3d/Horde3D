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
#ifndef CROWDVISIBILITYWIDGET_H_
#define CROWDVISIBILITYWIDGET_H_

#include "Ui_CrowdVisibilityWidget.h"

class QXmlTreeNode;

class CrowdVisibilityWidget : public QWidget, protected Ui_CrowdVisibilityWidget
{
	Q_OBJECT
public:
	CrowdVisibilityWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~CrowdVisibilityWidget();

	bool setCurrentNode(QXmlTreeNode* node);

signals:
	void modified(bool);

private slots:
	void updateTag( int tag );
	void updateTimes( );
	void updateCrowdSim();

private:
	QXmlTreeNode*	m_currentNode;


};
#endif