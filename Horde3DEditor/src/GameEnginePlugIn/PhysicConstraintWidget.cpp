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
#include "PhysicConstraintWidget.h"

#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEvent.h>
#include <math.h>

PhysicConstraintWidget::PhysicConstraintWidget(QDomElement node, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
	: m_currentNode(node), QWidget(parent, flags)
{
	setupUi(this);
	
	// General callbacks
	connect(m_deleteConstraint, SIGNAL(clicked()), this, SLOT(deleteConstraint()));

	// Load xml values to ui and set callback methods for changes
	// general attributes
	m_x->setValue(node.attribute("x","0").toFloat());
	connect(m_x, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	m_y->setValue(node.attribute("y","0").toFloat());
	connect(m_y, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	m_z->setValue(node.attribute("z","0").toFloat());
	connect(m_z, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	m_breakImpulseThres->setValue(node.attribute("breakImpulseThres","0").toFloat());
	connect(m_breakImpulseThres, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	m_dbgDrawSize->setValue(node.attribute("dbgDrawSize","5").toFloat());
	connect(m_dbgDrawSize, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	m_target->setText(node.attribute("target"));
	connect(m_target, SIGNAL(textChanged(const QString &)), this, SLOT(updateGeneralValues()));
	m_targetX->setValue(node.attribute("targetX","0").toFloat());
	connect(m_targetX, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	m_targetY->setValue(node.attribute("targetY","0").toFloat());
	connect(m_targetY, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	m_targetZ->setValue(node.attribute("targetZ","0").toFloat());
	connect(m_targetZ, SIGNAL(valueChanged(double)), this, SLOT(updateGeneralValues()));
	QString typeString = node.attribute("type", "point2point");
	if (typeString.compare("hinge",Qt::CaseInsensitive) == 0)
	{
		m_type->setCurrentIndex(1);
		m_constraintTypeProperties->setCurrentIndex(1);
	}
	else if (typeString.compare("slider",Qt::CaseInsensitive) == 0)
	{
		m_type->setCurrentIndex(2);
		m_constraintTypeProperties->setCurrentIndex(2);
	}
	else if (typeString.compare("coneTwist",Qt::CaseInsensitive) == 0)
	{
		m_type->setCurrentIndex(3);
		m_constraintTypeProperties->setCurrentIndex(3);
	}
	else if (typeString.compare("6DofSpring",Qt::CaseInsensitive) == 0)
	{
		m_type->setCurrentIndex(4);
		m_constraintTypeProperties->setCurrentIndex(4);
	}
	else
	{
		m_type->setCurrentIndex(0);
		m_constraintTypeProperties->setCurrentIndex(0);
	}
	connect(m_type, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
	//hinge attributes
	m_axisX->setValue(node.attribute("axisX","0").toFloat());
	connect(m_axisX, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_axisY->setValue(node.attribute("axisY","0").toFloat());
	connect(m_axisY, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_axisZ->setValue(node.attribute("axisZ","0").toFloat());
	connect(m_axisZ, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_targetAxisX->setValue(node.attribute("targetAxisX","0").toFloat());
	connect(m_targetAxisX, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_targetAxisY->setValue(node.attribute("targetAxisY","0").toFloat());
	connect(m_targetAxisY, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_targetAxisZ->setValue(node.attribute("targetAxisZ","0").toFloat());
	connect(m_targetAxisZ, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_lowLimit->setValue(node.attribute("lowLimit","-180").toFloat());
	connect(m_lowLimit, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_highLimit->setValue(node.attribute("highLimit","180").toFloat());
	connect(m_highLimit, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_hingeSoftness->setValue(node.attribute("softness","0.9").toFloat());
	connect(m_hingeSoftness, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_hingeBiasFactor->setValue(node.attribute("biasFactor","0.3").toFloat());
	connect(m_hingeBiasFactor, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	m_hingeRelaxationFactor->setValue(node.attribute("relaxationFactor","1").toFloat());
	connect(m_hingeRelaxationFactor, SIGNAL(valueChanged(double)), this, SLOT(updateHingeValues()));
	//slider attributes
	m_sliderRotX->setValue(node.attribute("rotX","0").toFloat());
	connect(m_sliderRotX, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_sliderRotY->setValue(node.attribute("rotY","0").toFloat());
	connect(m_sliderRotY, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_sliderRotZ->setValue(node.attribute("rotZ","0").toFloat());
	connect(m_sliderRotZ, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_sliderTargetRotX->setValue(node.attribute("targetRotX","0").toFloat());
	connect(m_sliderTargetRotX, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_sliderTargetRotY->setValue(node.attribute("targetRotY","0").toFloat());
	connect(m_sliderTargetRotY, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_sliderTargetRotZ->setValue(node.attribute("targetRotZ","0").toFloat());
	connect(m_sliderTargetRotZ, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_lowerLinLimit->setValue(node.attribute("lowerLinLimit","-999999").toFloat());
	connect(m_lowerLinLimit, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_upperLinLimit->setValue(node.attribute("upperLinLimit","999999").toFloat());
	connect(m_upperLinLimit, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_lowerAngLimit->setValue(node.attribute("lowerAngLimit","-180").toFloat());
	connect(m_lowerAngLimit, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	m_upperAngLimit->setValue(node.attribute("upperAngLimit","180").toFloat());
	connect(m_upperAngLimit, SIGNAL(valueChanged(double)), this, SLOT(updateSliderValues()));
	//coneTwist attributes
	m_coneTwistRotX->setValue(node.attribute("rotX","0").toFloat());
	connect(m_coneTwistRotX, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistRotY->setValue(node.attribute("rotY","0").toFloat());
	connect(m_coneTwistRotY, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistRotZ->setValue(node.attribute("rotZ","0").toFloat());
	connect(m_coneTwistRotZ, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistTargetRotX->setValue(node.attribute("targetRotX","0").toFloat());
	connect(m_coneTwistTargetRotX, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistTargetRotY->setValue(node.attribute("targetRotY","0").toFloat());
	connect(m_coneTwistTargetRotY, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistTargetRotZ->setValue(node.attribute("targetRotZ","0").toFloat());
	connect(m_coneTwistTargetRotZ, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_swing1Limit->setValue(node.attribute("swing1Limit","-180").toFloat());
	connect(m_swing1Limit, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_swing2Limit->setValue(node.attribute("swing2Limit","180").toFloat());
	connect(m_swing2Limit, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_twistLimit->setValue(node.attribute("twistLimit","180").toFloat());
	connect(m_twistLimit, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistSoftness->setValue(node.attribute("softness","1").toFloat());
	connect(m_coneTwistSoftness, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistBiasFactor->setValue(node.attribute("biasFactor","0.3").toFloat());
	connect(m_coneTwistBiasFactor, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	m_coneTwistRelaxationFactor->setValue(node.attribute("relaxationFactor","1").toFloat());
	connect(m_coneTwistRelaxationFactor, SIGNAL(valueChanged(double)), this, SLOT(updateConeTwistValues()));
	// 6DofSpring attributes
	m_springRotX->setValue(node.attribute("rotX","0").toFloat());
	connect(m_springRotX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_springRotY->setValue(node.attribute("rotY","0").toFloat());
	connect(m_springRotY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_springRotZ->setValue(node.attribute("rotZ","0").toFloat());
	connect(m_springRotZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_springTargetRotX->setValue(node.attribute("targetRotX","0").toFloat());
	connect(m_springTargetRotX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_springTargetRotY->setValue(node.attribute("targetRotY","0").toFloat());
	connect(m_springTargetRotY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_springTargetRotZ->setValue(node.attribute("targetRotZ","0").toFloat());
	connect(m_springTargetRotZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_lowerLinLimitX->setValue(node.attribute("lowerLinLimitX","-999999").toFloat());
	connect(m_lowerLinLimitX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_lowerLinLimitY->setValue(node.attribute("lowerLinLimitY","-999999").toFloat());
	connect(m_lowerLinLimitY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_lowerLinLimitZ->setValue(node.attribute("lowerLinLimitZ","-999999").toFloat());
	connect(m_lowerLinLimitZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_upperLinLimitX->setValue(node.attribute("upperLinLimitX","999999").toFloat());
	connect(m_upperLinLimitX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_upperLinLimitY->setValue(node.attribute("upperLinLimitY","999999").toFloat());
	connect(m_upperLinLimitY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_upperLinLimitZ->setValue(node.attribute("upperLinLimitZ","999999").toFloat());
	connect(m_upperLinLimitZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_lowerAngLimitX->setValue(node.attribute("lowerAngLimitX","-180").toFloat());
	connect(m_lowerAngLimitX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_lowerAngLimitY->setValue(node.attribute("lowerAngLimitY","-180").toFloat());
	connect(m_lowerAngLimitY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_lowerAngLimitZ->setValue(node.attribute("lowerAngLimitZ","-180").toFloat());
	connect(m_lowerAngLimitZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_upperAngLimitX->setValue(node.attribute("upperAngLimitX","180").toFloat());
	connect(m_upperAngLimitX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_upperAngLimitY->setValue(node.attribute("upperAngLimitY","180").toFloat());
	connect(m_upperAngLimitY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_upperAngLimitZ->setValue(node.attribute("upperAngLimitZ","180").toFloat());
	connect(m_upperAngLimitZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_dampingX->setValue(node.attribute("dampingX","1").toFloat());
	connect(m_dampingX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_dampingY->setValue(node.attribute("dampingY","1").toFloat());
	connect(m_dampingY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_dampingZ->setValue(node.attribute("dampingZ","1").toFloat());
	connect(m_dampingZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_dampingRX->setValue(node.attribute("dampingRX","1").toFloat());
	connect(m_dampingRX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_dampingRY->setValue(node.attribute("dampingRY","1").toFloat());
	connect(m_dampingRY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_dampingRZ->setValue(node.attribute("dampingRZ","1").toFloat());
	connect(m_dampingRZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_stiffnessX->setValue(node.attribute("stiffnessX","0").toFloat());
	connect(m_stiffnessX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_stiffnessY->setValue(node.attribute("stiffnessY","0").toFloat());
	connect(m_stiffnessY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_stiffnessZ->setValue(node.attribute("stiffnessZ","0").toFloat());
	connect(m_stiffnessZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_stiffnessRX->setValue(node.attribute("stiffnessRX","0").toFloat());
	connect(m_stiffnessRX, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_stiffnessRY->setValue(node.attribute("stiffnessRY","0").toFloat());
	connect(m_stiffnessRY, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_stiffnessRZ->setValue(node.attribute("stiffnessRZ","0").toFloat());
	connect(m_stiffnessRZ, SIGNAL(valueChanged(double)), this, SLOT(updateSpringValues()));
	m_disableSpringX->setChecked(node.attribute("disableSpringX", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node.attribute("disableSpringX", "0").compare("1",Qt::CaseInsensitive) == 0);
	connect(m_disableSpringX, SIGNAL(stateChanged(int)), this, SLOT(updateSpringValues()));
	m_disableSpringY->setChecked(node.attribute("disableSpringY", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node.attribute("disableSpringY", "0").compare("1",Qt::CaseInsensitive) == 0);	
	connect(m_disableSpringY, SIGNAL(stateChanged(int)), this, SLOT(updateSpringValues()));
	m_disableSpringZ->setChecked(node.attribute("disableSpringZ", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node.attribute("disableSpringZ", "0").compare("1",Qt::CaseInsensitive) == 0);
	connect(m_disableSpringZ, SIGNAL(stateChanged(int)), this, SLOT(updateSpringValues()));
	m_disableSpringRX->setChecked(node.attribute("disableSpringRX", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node.attribute("disableSpringRX", "0").compare("1",Qt::CaseInsensitive) == 0);
	connect(m_disableSpringRX, SIGNAL(stateChanged(int)), this, SLOT(updateSpringValues()));
	m_disableSpringRY->setChecked(node.attribute("disableSpringRY", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node.attribute("disableSpringRY", "0").compare("1",Qt::CaseInsensitive) == 0);
	connect(m_disableSpringRY, SIGNAL(stateChanged(int)), this, SLOT(updateSpringValues()));
	m_disableSpringRZ->setChecked(node.attribute("disableSpringRZ", "0").compare("true",Qt::CaseInsensitive) == 0 ||
			node.attribute("disableSpringRZ", "0").compare("1",Qt::CaseInsensitive) == 0);
	connect(m_disableSpringRZ, SIGNAL(stateChanged(int)), this, SLOT(updateSpringValues()));
}

PhysicConstraintWidget::~PhysicConstraintWidget()
{
}

void PhysicConstraintWidget::deleteConstraint()
{
	emit deleting(this);
}

void PhysicConstraintWidget::typeChanged(int typeIndex)
{
	// TODO: remove unneeded xml attributes from current node?
	switch (typeIndex)
	{
	case 0: // point2point
		m_currentNode.setAttribute("type", "point2point");
		break;
	case 1: // hinge
		m_currentNode.setAttribute("type", "hinge");
		break;
	case 2: // slider
		m_currentNode.setAttribute("type", "slider");
		break;
	case 3: // coneTwist
		m_currentNode.setAttribute("type", "coneTwist");
		break;
	case 4: // 6DofSpring
		m_currentNode.setAttribute("type", "6DofSpring");
		break;
	}
	m_constraintTypeProperties->setCurrentIndex(typeIndex);
	emit modified(true);
}

void PhysicConstraintWidget::updateGeneralValues()
{	
	m_currentNode.setAttribute("x", m_x->value());	
	m_currentNode.setAttribute("y", m_y->value());	
	m_currentNode.setAttribute("z", m_z->value());
	m_currentNode.setAttribute("breakImpulseThres", m_breakImpulseThres->value());
	m_currentNode.setAttribute("dbgDrawSize", m_dbgDrawSize->value());
	m_currentNode.setAttribute("target", m_target->text());
	m_currentNode.setAttribute("targetX", m_targetX->value());
	m_currentNode.setAttribute("targetY", m_targetY->value());
	m_currentNode.setAttribute("targetZ", m_targetZ->value());
	emit modified(true);
}

void PhysicConstraintWidget::updateHingeValues()
{
	m_currentNode.setAttribute("axisX", m_axisX->value());
	m_currentNode.setAttribute("axisY", m_axisY->value());
	m_currentNode.setAttribute("axisZ", m_axisZ->value());
	m_currentNode.setAttribute("targetAxisX", m_targetAxisX->value());
	m_currentNode.setAttribute("targetAxisY", m_targetAxisY->value());
	m_currentNode.setAttribute("targetAxisZ", m_targetAxisZ->value());
	m_currentNode.setAttribute("lowLimit", m_lowLimit->value());
	m_currentNode.setAttribute("highLimit", m_highLimit->value());
	m_currentNode.setAttribute("softness", m_hingeSoftness->value());
	m_currentNode.setAttribute("biasFactor", m_hingeBiasFactor->value());
	m_currentNode.setAttribute("relaxationFactor", m_hingeRelaxationFactor->value());
	emit modified(true);
}

void PhysicConstraintWidget::updateSliderValues()
{
	m_currentNode.setAttribute("rotX", m_sliderRotX->value());
	m_currentNode.setAttribute("rotY", m_sliderRotY->value());
	m_currentNode.setAttribute("rotZ", m_sliderRotZ->value());
	m_currentNode.setAttribute("targetRotX", m_sliderTargetRotX->value());
	m_currentNode.setAttribute("targetRotY", m_sliderTargetRotY->value());
	m_currentNode.setAttribute("targetRotZ", m_sliderTargetRotZ->value());
	m_currentNode.setAttribute("lowerLinLimit", m_lowerLinLimit->value());
	m_currentNode.setAttribute("upperLinLimit", m_upperLinLimit->value());
	m_currentNode.setAttribute("lowerAngLimit", m_lowerAngLimit->value());
	m_currentNode.setAttribute("upperAngLimit", m_upperAngLimit->value());
	emit modified(true);
}

void PhysicConstraintWidget::updateConeTwistValues()
{
	m_currentNode.setAttribute("rotX", m_coneTwistRotX->value());
	m_currentNode.setAttribute("rotY", m_coneTwistRotY->value());
	m_currentNode.setAttribute("rotZ", m_coneTwistRotZ->value());
	m_currentNode.setAttribute("targetRotX", m_coneTwistTargetRotX->value());
	m_currentNode.setAttribute("targetRotY", m_coneTwistTargetRotY->value());
	m_currentNode.setAttribute("targetRotZ", m_coneTwistTargetRotZ->value());
	m_currentNode.setAttribute("swing1Limit", m_swing1Limit->value());
	m_currentNode.setAttribute("swing2Limit", m_swing2Limit->value());
	m_currentNode.setAttribute("twistLimit", m_twistLimit->value());
	m_currentNode.setAttribute("softness", m_coneTwistSoftness->value());
	m_currentNode.setAttribute("biasFactor", m_coneTwistBiasFactor->value());
	m_currentNode.setAttribute("relaxationFactor", m_coneTwistRelaxationFactor->value());
	emit modified(true);
}

void PhysicConstraintWidget::updateSpringValues()
{
	m_currentNode.setAttribute("rotX", m_springRotX->value());
	m_currentNode.setAttribute("rotY", m_springRotY->value());
	m_currentNode.setAttribute("rotZ", m_springRotZ->value());
	m_currentNode.setAttribute("targetRotX", m_springTargetRotX->value());
	m_currentNode.setAttribute("targetRotY", m_springTargetRotY->value());
	m_currentNode.setAttribute("targetRotZ", m_springTargetRotZ->value());
	m_currentNode.setAttribute("lowerLinLimitX", m_lowerLinLimitX->value());
	m_currentNode.setAttribute("lowerLinLimitY", m_lowerLinLimitY->value());
	m_currentNode.setAttribute("lowerLinLimitZ", m_lowerLinLimitZ->value());
	m_currentNode.setAttribute("upperLinLimitX", m_upperLinLimitX->value());
	m_currentNode.setAttribute("upperLinLimitY", m_upperLinLimitY->value());
	m_currentNode.setAttribute("upperLinLimitZ", m_upperLinLimitZ->value());
	m_currentNode.setAttribute("lowerAngLimitX", m_lowerAngLimitX->value());
	m_currentNode.setAttribute("lowerAngLimitY", m_lowerAngLimitY->value());
	m_currentNode.setAttribute("lowerAngLimitZ", m_lowerAngLimitZ->value());
	m_currentNode.setAttribute("upperAngLimitX", m_upperAngLimitX->value());
	m_currentNode.setAttribute("upperAngLimitY", m_upperAngLimitY->value());
	m_currentNode.setAttribute("upperAngLimitZ", m_upperAngLimitZ->value());
	m_currentNode.setAttribute("dampingX", m_dampingX->value());
	m_currentNode.setAttribute("dampingY", m_dampingY->value());
	m_currentNode.setAttribute("dampingZ", m_dampingZ->value());
	m_currentNode.setAttribute("dampingRX", m_dampingRX->value());
	m_currentNode.setAttribute("dampingRY", m_dampingRY->value());
	m_currentNode.setAttribute("dampingRZ", m_dampingRZ->value());
	m_currentNode.setAttribute("stiffnessX", m_stiffnessX->value());
	m_currentNode.setAttribute("stiffnessY", m_stiffnessY->value());
	m_currentNode.setAttribute("stiffnessZ", m_stiffnessZ->value());
	m_currentNode.setAttribute("stiffnessRX", m_stiffnessRX->value());
	m_currentNode.setAttribute("stiffnessRY", m_stiffnessRY->value());
	m_currentNode.setAttribute("stiffnessRZ", m_stiffnessRZ->value());
	m_currentNode.setAttribute("disableSpringX", m_disableSpringX->isChecked() ? 1 : 0);
	m_currentNode.setAttribute("disableSpringY", m_disableSpringY->isChecked() ? 1 : 0);
	m_currentNode.setAttribute("disableSpringZ", m_disableSpringZ->isChecked() ? 1 : 0);
	m_currentNode.setAttribute("disableSpringRX", m_disableSpringRX->isChecked() ? 1 : 0);
	m_currentNode.setAttribute("disableSpringRY", m_disableSpringRY->isChecked() ? 1 : 0);
	m_currentNode.setAttribute("disableSpringRZ", m_disableSpringRZ->isChecked() ? 1 : 0);
	emit modified(true);
}