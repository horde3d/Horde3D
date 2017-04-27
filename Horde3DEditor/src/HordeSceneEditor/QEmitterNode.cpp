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
#include "QEmitterNode.h"

#include <QXmlTree/QXmlNodePropertyCommand.h>
#include "HordeSceneEditor.h"
#include "AttachmentPlugIn.h"
#include "SceneTreeModel.h"
#include "PlugInManager.h"

#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "SceneFile.h"

#include <Horde3D.h>
#include <Horde3DUtils.h>


QSceneNode* QEmitterNode::loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent)
{
	return new QEmitterNode(xmlNode, row, model, parent);
}

QEmitterNode::QEmitterNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode) : QSceneNode(xmlNode, row, model, parentNode)
{
	setObjectName("Emitter");
	QEmitterNode::addRepresentation();	
}


QEmitterNode::~QEmitterNode()
{
	if (m_matResource != 0)
		h3dRemoveResource(m_matResource);
	if (m_effectResource != 0)
		h3dRemoveResource(m_effectResource);
}


void QEmitterNode::addRepresentation()
{	
	m_matResource = h3dAddResource(H3DResTypes::Material, qPrintable(m_xmlNode.attribute("material")), 0);

	// Load resource immediately since a later call to loadResourceFromDisk results in a bad behaviour of the Horde3D engine
	QString resourceName = h3dutGetResourcePath(H3DResTypes::Material);
	if( !resourceName.isEmpty() && !resourceName.endsWith('/') && !resourceName.endsWith('\\') )
		resourceName += '/';
	resourceName += m_xmlNode.attribute("material");

	QFile matFile(QFileInfo(resourceName).absoluteFilePath());
	if (!matFile.open(QIODevice::ReadOnly))
		qWarning("Error opening resource %s", qPrintable(m_xmlNode.attribute("material")));

	// Stupid return value, if false it can also be the case that the resource was loaded before instead of that their was an error
	h3dLoadResource(m_matResource, matFile.readAll().append('\0').constData(), matFile.size() + 1);
	matFile.close();

	m_effectResource = h3dAddResource(H3DResTypes::ParticleEffect, qPrintable(m_xmlNode.attribute("particleEffect")), 0);

	// Load resource immediately since a later call to loadResourceFromDisk results in a bad behaviour of the Horde3D engine
	resourceName = h3dutGetResourcePath(H3DResTypes::ParticleEffect);
	if (!resourceName.isEmpty() && !resourceName.endsWith('/') && !resourceName.endsWith('\\'))
		resourceName += '/';
	resourceName += m_xmlNode.attribute("particleEffect");

	QFile effectFile(QFileInfo(resourceName).absoluteFilePath());
	if (!effectFile.open(QIODevice::ReadOnly))
		qWarning("Error opening resource %s", qPrintable(m_xmlNode.attribute("particleEffect")));

	// Stupid return value, if false it can also be the case that the resource was loaded before instead of that their was an error
	h3dLoadResource(m_effectResource, effectFile.readAll().append('\0').constData(), effectFile.size()+1);
	effectFile.close();

	QSceneNode* parentNode = static_cast<QSceneNode*>(parent());
	unsigned int rootID = parentNode ? parentNode->hordeId() : H3DRootNode;

	m_hordeID = h3dAddEmitterNode(
		rootID, 
		qPrintable(m_xmlNode.attribute("name", "ATTENTION No Node Name")), 
		m_matResource, 
		m_effectResource, 
		m_xmlNode.attribute("maxCount").toUInt(),
		m_xmlNode.attribute("respawnCount").toInt()
	);

	h3dSetNodeParamF(m_hordeID, H3DEmitter::ForceF3, 0, m_xmlNode.attribute("forceX", "0.0").toFloat());
	h3dSetNodeParamF(m_hordeID, H3DEmitter::ForceF3, 1, m_xmlNode.attribute("forceY", "0.0").toFloat());
	h3dSetNodeParamF(m_hordeID, H3DEmitter::ForceF3, 2, m_xmlNode.attribute("forceZ", "0.0").toFloat());
	h3dSetNodeParamF(m_hordeID, H3DEmitter::DelayF, 0, m_xmlNode.attribute("delay", "0.0").toFloat());
	h3dSetNodeParamF(m_hordeID, H3DEmitter::SpreadAngleF, 0, m_xmlNode.attribute("spreadAngle", "0.0").toFloat());
	h3dSetNodeParamF(m_hordeID, H3DEmitter::EmissionRateF, 0, m_xmlNode.attribute("emissionRate", "0.0").toFloat());

	// load transformation from file...
	float x, y, z, rx, ry, rz, sx, sy, sz;
	getTransformation(x,y,z,rx,ry,rz,sx,sy,sz);
	// ...and update scene representation
	h3dSetNodeTransform(m_hordeID, x, y, z, rx, ry, rz, sx, sy, sz);
	
	// Attachment
	QDomElement attachment = m_xmlNode.firstChildElement("Attachment");	
	SceneTreeModel* model = static_cast<SceneTreeModel*>(m_model);
	AttachmentPlugIn* plugIn = model->nodeFactory()->attachmentPlugIn();
	if (!attachment.isNull() &&  plugIn != 0)
		plugIn->initNodeAttachment(this);

	startTimer(100);
}


Material QEmitterNode::material() const
{
	return m_xmlNode.attribute("material");
}

void QEmitterNode::setMaterial(const Material& material)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("material", material.FileName);
		if (m_matResource != 0)
			h3dRemoveResource(m_matResource);
		m_matResource = h3dAddResource( H3DResTypes::Material, qPrintable(material.FileName), 0 );
		h3dutLoadResourcesFromDisk(".");
		h3dSetNodeParamI(m_hordeID, H3DEmitter::MatResI, m_matResource);
	}
	else if (material != QEmitterNode::material())
		m_model->undoStack()->push(new QXmlNodePropertyCommand("Set Material", this, "Material", QVariant::fromValue(material), EmitterMaterialID));
}

Effect QEmitterNode::effect() const
{
	return Effect(m_xmlNode.attribute("particleEffect"));
}

void QEmitterNode::setEffect(const Effect& effect)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("particleEffect", effect.FileName);
		if (m_effectResource != 0)
			h3dRemoveResource(m_effectResource);
		m_effectResource = h3dAddResource( H3DResTypes::ParticleEffect, qPrintable(effect.FileName), 0 );
		h3dutLoadResourcesFromDisk(".");
		h3dSetNodeParamI(m_hordeID, H3DEmitter::PartEffResI, m_effectResource);
	}
	else if (effect != QEmitterNode::effect())
		m_model->undoStack()->push(new QXmlNodePropertyCommand("Set Effect", this, "Effect", QVariant::fromValue(effect), EmitterMaterialID));
}

unsigned int QEmitterNode::maxCount() const
{
	return m_xmlNode.attribute("maxCount").toUInt();
}

void QEmitterNode::setMaxCount(unsigned int value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("maxCount", value);
		h3dSetNodeParamI(m_hordeID, H3DEmitter::MaxCountI, value);
	}
	else if (value != QEmitterNode::maxCount())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Max Count"), this, "Max_Count", value, EmitterMaxCountID));

}


int QEmitterNode::respawnCount() const
{
	return m_xmlNode.attribute("respawnCount").toInt();
}

void QEmitterNode::setRespawnCount(int value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("respawnCount", value);
		h3dSetNodeParamI(m_hordeID, H3DEmitter::RespawnCountI, value);
	}
	else if (value != QEmitterNode::respawnCount())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Respawn Count"), this, "Respawn_Count", value, EmitterRespawnCountID));

}


QVec3f QEmitterNode::force() const
{
	return QVec3f(m_xmlNode.attribute("forceX").toFloat(), m_xmlNode.attribute("forceY").toFloat(), m_xmlNode.attribute("forceZ").toFloat());
}

void QEmitterNode::setForce(const QVec3f &value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("forceX", value.X);
		m_xmlNode.setAttribute("forceY", value.Y);
		m_xmlNode.setAttribute("forceZ", value.Z);
		h3dSetNodeParamF(m_hordeID, H3DEmitter::ForceF3, 0, value.X);
		h3dSetNodeParamF(m_hordeID, H3DEmitter::ForceF3, 1, value.Y);
		h3dSetNodeParamF(m_hordeID, H3DEmitter::ForceF3, 2, value.Z);
	}
	else if (value != QEmitterNode::force())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Force"), this, "Force", QVariant::fromValue(value), EmitterForceID));
}

int QEmitterNode::emissionRate() const
{
	return m_xmlNode.attribute("emissionRate").toInt();
}

void QEmitterNode::setEmissionRate(int value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("emissionRate", value);
		h3dSetNodeParamF(m_hordeID, H3DEmitter::EmissionRateF, 0, value);
	}
	else if (value != QEmitterNode::emissionRate())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Emission Rate"), this, "Emission_Rate", value, EmitterEmissionRateID));
}

float QEmitterNode::spreadAngle() const
{
	return m_xmlNode.attribute("spreadAngle").toFloat();
}

void QEmitterNode::setSpreadAngle(float value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("spreadAngle", value);
		h3dSetNodeParamF(m_hordeID, H3DEmitter::SpreadAngleF, 0, value);
	}
	else if (value != QEmitterNode::spreadAngle())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Spread Angle"), this, "Spread_Angle", value, EmitterSpreadAngleID));
}

float QEmitterNode::delay() const
{
	return m_xmlNode.attribute("delay").toFloat();
}

void QEmitterNode::setDelay(float value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("delay", value);
		h3dSetNodeParamF(m_hordeID, H3DEmitter::DelayF, 0, value);
	}
	else if (value != QEmitterNode::delay())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Delay"), this, "Delay", value, EmitterDelayID));
}

void QEmitterNode::timerEvent(QTimerEvent* /*event*/)
{
	if (m_hordeID != 0)
		h3dUpdateEmitter(m_hordeID, 0.1f);
}
