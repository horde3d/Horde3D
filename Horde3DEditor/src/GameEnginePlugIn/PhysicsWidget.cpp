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
#include "PhysicsWidget.h"
#include "PhysicConstraintWidget.h"

#include <QtXml/qdom.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qtextstream.h>

#include <QXmlTree/QXmlTreeNode.h>
#include <QSceneNode.h>

#include <Horde3D/Horde3D.h>
#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEngine_BulletPhysics.h>

PhysicsWidget::PhysicsWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags), m_updateRequested(false)
{
	setupUi(this);
	connect(m_mass, SIGNAL(valueChanged(double)), this, SLOT(massChanged(double)));
	connect(m_mass, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_kinematic, SIGNAL(toggled(bool)), this, SLOT(kinematicChanged(bool)));
	connect(m_kinematic, SIGNAL(toggled(bool)), this, SLOT(updatePhysics()));
	connect(m_shape, SIGNAL(currentIndexChanged(int)), this, SLOT(shapeChanged(int)));
	connect(m_shape, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePhysics()));
	connect(m_width, SIGNAL(valueChanged(double)), this, SLOT(updateBox()));
	connect(m_width, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_height, SIGNAL(valueChanged(double)), this, SLOT(updateBox()));
	connect(m_height, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_depth, SIGNAL(valueChanged(double)), this, SLOT(updateBox()));
	connect(m_depth, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_radius, SIGNAL(valueChanged(double)), this, SLOT(updateSphere(double)));
	connect(m_radius, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_cylRadius, SIGNAL(valueChanged(double)), this, SLOT(updateCylinder()));
	connect(m_cylRadius, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_cylHeight, SIGNAL(valueChanged(double)), this, SLOT(updateCylinder()));
	connect(m_cylHeight, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_restitution, SIGNAL(valueChanged(double)), this, SLOT(updateRestitution(double)));
	connect(m_restitution, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_friction, SIGNAL(valueChanged(double)), this, SLOT(updateFriction(double)));
	connect(m_rollingFriction, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_rollingFriction, SIGNAL(valueChanged(double)), this, SLOT(updateRollingFriction(double)));
	connect(m_friction, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_noContactResponse, SIGNAL(toggled(bool)), this, SLOT(noContactResponseChanged(bool)));
	connect(m_noContactResponse, SIGNAL(toggled(bool)), this, SLOT(updatePhysics()));
	connect(m_offsetX, SIGNAL(valueChanged(double)), this, SLOT(updateOffset()));
	connect(m_offsetX, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_offsetY, SIGNAL(valueChanged(double)), this, SLOT(updateOffset()));
	connect(m_offsetY, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_offsetZ, SIGNAL(valueChanged(double)), this, SLOT(updateOffset()));
	connect(m_offsetZ, SIGNAL(valueChanged(double)), this, SLOT(updatePhysics()));
	connect(m_resetWorld, SIGNAL(clicked()), this, SLOT(resetPhysics()));	
	connect(m_debug, SIGNAL(toggled(bool)), this, SLOT(toggleDebugRendering(bool)));
	connect(m_enabled, SIGNAL(toggled(bool)), this, SLOT(togglePhysics(bool)));
	connect(m_addConstraint, SIGNAL(clicked()), this, SLOT(addConstraint()));
	
	m_debug->setChecked(GameEngine::physicsShapeRenderingEnabled());
	m_enabled->setChecked(GameEngine::physicsEnabled());
}


PhysicsWidget::~PhysicsWidget()
{
}

bool PhysicsWidget::setCurrentNode(QXmlTreeNode *node)
{
	// First remove all current constraints, the other values aren't that important to reset...
	while(m_constraintContainer->count() != 0)
	{
		QLayoutItem* childItem = m_constraintContainer->takeAt(0);
		if (childItem->widget() != 0)
			delete childItem->widget();
		delete childItem;
	}

	if (node && (node->xmlNode().tagName() == "BulletPhysics" || node->xmlNode().tagName() == "Physics"))
	{				
		m_currentNode = 0;
		m_mass->setValue(node->xmlNode().attribute("mass","0.0").toFloat());
		m_width->setValue(node->xmlNode().attribute("x", "1").toFloat());
		m_height->setValue(node->xmlNode().attribute("y","1").toFloat());
		m_depth->setValue(node->xmlNode().attribute("z", "1").toFloat());
		m_radius->setValue(node->xmlNode().attribute("radius", "1").toFloat());
		m_cylRadius->setValue(node->xmlNode().attribute("radius", "1").toFloat());
		m_cylHeight->setValue(node->xmlNode().attribute("height", "1").toFloat());

		int index = m_shape->findText(node->xmlNode().attribute("shape"));
		m_shape->setCurrentIndex(index);
		m_restitution->setValue(node->xmlNode().attribute("restitution", "0").toFloat());
		m_friction->setValue(node->xmlNode().attribute("friction", "0.5").toFloat());
		m_rollingFriction->setValue(node->xmlNode().attribute("rollingFriction", "0").toFloat());
		m_kinematic->setChecked(
			node->xmlNode().attribute("kinematic", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node->xmlNode().attribute("kinematic", "0").compare("1",Qt::CaseInsensitive) == 0);		
		m_noContactResponse->setChecked(
			node->xmlNode().attribute("noContactResponse", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node->xmlNode().attribute("noContactResponse", "0").compare("1",Qt::CaseInsensitive) == 0);	
		m_offsetX->setValue(node->xmlNode().attribute("offsetX","0").toFloat());
		m_offsetY->setValue(node->xmlNode().attribute("offsetY","0").toFloat());
		m_offsetZ->setValue(node->xmlNode().attribute("offsetZ","0").toFloat());
		m_currentNode = node;
		shapeChanged(index);

		for (int i = 0; i < node->xmlNode().childNodes().count(); ++i)
		{
			QDomNode constNode = node->xmlNode().childNodes().at(i);
			if (constNode.isElement() && constNode.nodeName().compare("Constraint") == 0)
			{
				PhysicConstraintWidget* wdg = new PhysicConstraintWidget(constNode.toElement(), this);
				connect(wdg, SIGNAL(modified(bool)), this, SLOT(updatePhysics()));
				connect(wdg, SIGNAL(deleting(PhysicConstraintWidget*)), this, SLOT(deleteConstraint(PhysicConstraintWidget*)));
				m_constraintContainer->addWidget(wdg);
			}
		}
		return true;
	}
	else
		return false;
}

void PhysicsWidget::shapeChanged(int index)
{
	if (m_currentNode == 0)
		return;

	switch(index)
	{
	case 0: // Box
		m_currentNode->xmlNode().removeAttribute("radius");
		m_currentNode->xmlNode().removeAttribute("height");
		updateBox();
		m_shapeWidget->setCurrentIndex(0);	
		break;
	case 1: // Sphere
		m_currentNode->xmlNode().removeAttribute("x");
		m_currentNode->xmlNode().removeAttribute("y");
		m_currentNode->xmlNode().removeAttribute("z");
		m_currentNode->xmlNode().removeAttribute("height");
		updateSphere(m_radius->value());
		m_shapeWidget->setCurrentIndex(1);	
		break;
	case 2: case 3: case 4: // Cylinder, Capsule, Cone
		m_currentNode->xmlNode().removeAttribute("x");
		m_currentNode->xmlNode().removeAttribute("y");
		m_currentNode->xmlNode().removeAttribute("z");
		updateCylinder();
		m_shapeWidget->setCurrentIndex(2);
		break;
	case 5: // Mesh
		m_currentNode->xmlNode().removeAttribute("radius");
		m_currentNode->xmlNode().removeAttribute("x");
		m_currentNode->xmlNode().removeAttribute("y");
		m_currentNode->xmlNode().removeAttribute("z");
		m_currentNode->xmlNode().removeAttribute("height");
		m_shapeWidget->setCurrentIndex(3);
		break;
	}
	m_currentNode->xmlNode().setAttribute("shape", m_shape->currentText());
}

void PhysicsWidget::massChanged(double value)
{	
	m_kinematic->setEnabled(value == 0.0);
	if (m_currentNode == 0)
		return;
	m_currentNode->xmlNode().setAttribute("mass", value);
}

void PhysicsWidget::kinematicChanged(bool enabled)
{
	m_mass->setDisabled(enabled);
	if (m_currentNode == 0)	return;
	if (enabled)
		m_currentNode->xmlNode().setAttribute("kinematic", enabled);
	else
		m_currentNode->xmlNode().removeAttribute("kinematic");
}

void PhysicsWidget::noContactResponseChanged(bool enabled)
{
	if (m_currentNode == 0)	return;
	if (enabled)
		m_currentNode->xmlNode().setAttribute("noContactResponse", enabled);
	else
		m_currentNode->xmlNode().removeAttribute("noContactResponse");
}

void PhysicsWidget::updateOffset()
{
	if (m_currentNode == 0)
		return;
	if (abs(m_offsetX->value()) > 0.0001)
		m_currentNode->xmlNode().setAttribute("offsetX", m_offsetX->value());
	else
		m_currentNode->xmlNode().removeAttribute("offsetX");
	if (abs(m_offsetY->value()) > 0.0001)
		m_currentNode->xmlNode().setAttribute("offsetY", m_offsetY->value());
	else
		m_currentNode->xmlNode().removeAttribute("offsetY");
	if (abs(m_offsetZ->value()) > 0.0001)
		m_currentNode->xmlNode().setAttribute("offsetZ", m_offsetZ->value());
	else
		m_currentNode->xmlNode().removeAttribute("offsetZ");
}

void PhysicsWidget::updateBox()
{
	if (m_currentNode == 0)
		return;
	m_currentNode->xmlNode().setAttribute("x", m_width->value());
	m_currentNode->xmlNode().setAttribute("y", m_height->value());
	m_currentNode->xmlNode().setAttribute("z", m_depth->value());	
}

void PhysicsWidget::updateSphere(double value)
{	
	if (m_currentNode == 0)	return;
	m_currentNode->xmlNode().setAttribute("radius", value);	
}


void PhysicsWidget::updateCylinder()
{	
	if (m_currentNode == 0) return;
	m_currentNode->xmlNode().setAttribute("radius", m_cylRadius->value());	
	m_currentNode->xmlNode().setAttribute("height", m_cylHeight->value());	
}

void PhysicsWidget::updateRestitution(double value)
{
	if (m_currentNode == 0)
		return;
	m_currentNode->xmlNode().setAttribute("restitution", value);	

}

void PhysicsWidget::updateFriction(double value)
{
	if (m_currentNode == 0)
		return;
	m_currentNode->xmlNode().setAttribute("friction", value);	
}

void PhysicsWidget::updateRollingFriction(double value)
{
	if (m_currentNode == 0)
		return;
	m_currentNode->xmlNode().setAttribute("rollingFriction", value);	
}

void PhysicsWidget::updatePhysics()
{
	if (m_currentNode != 0 && !m_updateRequested)
	{
		// Temporarily disable physics so that the current update is finished before we set component data
		// and we don't cause any conflicts
		GameEngine::setPhysicsEnabled(false);
		m_updateRequested = true;
	}
}

void PhysicsWidget::updateFinished()
{
	if (m_updateRequested)
	{
		QString entityName = m_currentNode->xmlNode().parentNode().toElement().attribute("name");
		QString xmlData;
		QTextStream stream(&xmlData);
		m_currentNode->xmlNode().save(stream, 4);	
		unsigned int entityID = GameEngine::entityWorldID(qPrintable(entityName));
		GameEngine::setComponentData(entityID, "BulletPhysics", qPrintable(xmlData));	
		GameEngine::setPhysicsEnabled(m_enabled->isChecked());
		emit modified(true);
		m_updateRequested = false;
	}
}

void PhysicsWidget::resetPhysics()
{
	GameEngine::resetPhysics();
}

void PhysicsWidget::toggleDebugRendering(bool enabled)
{
	GameEngine::setPhysicsShapeRenderingEnabled( enabled );
}

void PhysicsWidget::togglePhysics(bool enabled)
{
	GameEngine::setPhysicsEnabled(enabled);
}

void PhysicsWidget::addConstraint()
{
	if (m_currentNode != 0)
	{
		QDomElement newNode = m_currentNode->xmlNode().ownerDocument().createElement(QString("Constraint"));
		m_currentNode->xmlNode().appendChild(newNode);
		PhysicConstraintWidget* wdg = new PhysicConstraintWidget(newNode, this);
		connect(wdg, SIGNAL(modified(bool)), this, SLOT(updatePhysics()));
		connect(wdg, SIGNAL(deleting(PhysicConstraintWidget*)), this, SLOT(deleteConstraint(PhysicConstraintWidget*)));
		m_constraintContainer->addWidget(wdg);
		updatePhysics();
	}
}

void PhysicsWidget::deleteConstraint(PhysicConstraintWidget* wdg)
{
	if (m_currentNode != 0)
	{
		m_currentNode->xmlNode().removeChild(wdg->getXmlNode());
		m_constraintContainer->removeWidget(wdg);
		delete wdg;
		updatePhysics();
	}
}