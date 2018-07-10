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
#ifndef PHYSICSWIDGET_H_
#define PHYSICSWIDGET_H_

#include "Ui_PhysicsWidget.h"
class QXmlTreeNode;

class PhysicsWidget : public QWidget, protected Ui_PhysicsWidget
{
	Q_OBJECT
public:
	PhysicsWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~PhysicsWidget();

	bool setCurrentNode(QXmlTreeNode* node);

signals:
	void modified(bool);

private slots:
	void shapeChanged(int index);
	void massChanged(double value);	
	void kinematicChanged(bool checked);
	void noContactResponseChanged(bool enabled);
	void updateOffset();
	void updateBox();
	void updateSphere(double value);
	void updateCylinder();
	void updateFriction(double value);
	void updateRollingFriction(double value);
	void updateRestitution(double value);
	void updatePhysics();
	void resetPhysics();
	void toggleDebugRendering(bool enabled);
	void togglePhysics(bool enabled);
	void addConstraint();
	void deleteConstraint(class PhysicConstraintWidget* wdg);
	void updateFinished();

private:
	QXmlTreeNode*	m_currentNode;
	bool			m_updateRequested;

};
#endif