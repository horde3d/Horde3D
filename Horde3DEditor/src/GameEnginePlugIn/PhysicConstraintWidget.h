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
// Phoneme Editor
// ---------------------------------------------------------
// Copyright (C) 2014 Felix Kistler
// 
// ****************************************************************************************
#ifndef PHYSICCONSTRAINTWIDGET_H_
#define PHYSICCONSTRAINTWIDGET_H_

#include "Ui_PhysicConstraintWidget.h"

#include <QtXml/qdom.h>

class PhysicConstraintWidget : public QWidget, protected Ui_PhysicConstraintWidget
{
	Q_OBJECT
public:
	PhysicConstraintWidget(QDomElement node, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~PhysicConstraintWidget();

	QDomElement getXmlNode()
	{
		return m_currentNode;
	}

signals:
	void modified(bool);
	void deleting(PhysicConstraintWidget*);

private slots:
	void updateGeneralValues();
	void updateHingeValues();
	void updateSliderValues();
	void updateConeTwistValues();
	void updateSpringValues();
	void typeChanged(int typeIndex);
	void deleteConstraint();
	//TODO rest

private:
	QDomElement	m_currentNode;
};
#endif