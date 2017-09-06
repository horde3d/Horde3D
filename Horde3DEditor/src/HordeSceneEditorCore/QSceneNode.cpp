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

#include "QSceneNode.h"
#include "SceneTreeModel.h"
#include "PlugInManager.h"
#include "AttachmentPlugIn.h"
#include "CustomTypes.h"

#include <QXmlTree/QXmlNodePropertyCommand.h>


#include <QApplication>

#include "SceneFile.h"

#include <Horde3D.h>

QSceneNode::QSceneNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode) : 
QXmlTreeNode(xmlNode, row, model, parentNode), m_hordeID(0), m_active(true)
{			
	if( model )
		m_knownNodeNames = model->nodeFactory()->sceneNodeNames();
	else // Only support basic types if this node has no model 
		// (kind of hack to allow root reference node, because there is no real Reference node in Horde3D)
		m_knownNodeNames << "Group"<< "Model"<< "Mesh"<< "Joint"<< "Reference"<< "Light" << "Camera"<< "Emitter";

	setProperty("__Horde3D", true);
	if (m_xmlNode.hasAttribute("px"))
	{
		m_xmlNode.setAttribute("tx", m_xmlNode.attribute("px").remove('f'));
		m_xmlNode.removeAttribute("px");
	}
	if (m_xmlNode.hasAttribute("py"))
	{
		m_xmlNode.setAttribute("ty", m_xmlNode.attribute("py").remove('f'));
		m_xmlNode.removeAttribute("py");
	}
	if (m_xmlNode.hasAttribute("pz"))
	{
		m_xmlNode.setAttribute("tz", m_xmlNode.attribute("pz").remove('f'));
		m_xmlNode.removeAttribute("pz");
	}
}


QSceneNode::~QSceneNode()
{ 
	qDeleteAll(m_childItems);
	if (m_hordeID != 0 && h3dGetNodeType(m_hordeID) != H3DNodeTypes::Undefined)
		h3dRemoveNode(m_hordeID);
	m_hordeID = 0;
	QDomElement attachment = m_xmlNode.firstChildElement("Attachment");	
	SceneTreeModel* model = static_cast<SceneTreeModel*>(m_model);
	if( model && !attachment.isNull())
	{
		AttachmentPlugIn* plugIn = model->nodeFactory()->attachmentPlugIn();
		if (plugIn != 0) plugIn->destroyNodeAttachment(this);
	}
	h3dReleaseUnusedResources();
	m_childItems.clear();
}


QXmlTreeNode* QSceneNode::createChild(const QDomElement& childNode, int row)
{
	SceneTreeModel* model = static_cast<SceneTreeModel*>(m_model);
	QSceneNode* childItem = model->nodeFactory()->loadSceneNode(childNode, row, model, this);
	// Populate Tree
	if (childItem) childItem->childs();
	return childItem;
}

void QSceneNode::getTransformation(float& x, float& y, float& z, float& rx, float& ry, float& rz, float& sx, float& sy, float& sz)
{
	x = roundIt( m_xmlNode.attribute("tx", "0.0").remove('f').toFloat(), 5 );
	y = roundIt( m_xmlNode.attribute("ty", "0.0").remove('f').toFloat(), 5 );
	z = roundIt( m_xmlNode.attribute("tz", "0.0").remove('f').toFloat(), 5 );
	rx = roundIt( m_xmlNode.attribute("rx", "0.0").remove('f').toFloat(), 5 );
	ry = roundIt( m_xmlNode.attribute("ry", "0.0").remove('f').toFloat(), 5 );
	rz = roundIt( m_xmlNode.attribute("rz", "0.0").remove('f').toFloat(), 5 );
	sx = roundIt( m_xmlNode.attribute("sx", "1.0").remove('f').toFloat(), 5 );
	sy = roundIt( m_xmlNode.attribute("sy", "1.0").remove('f').toFloat(), 5 );
	sz = roundIt( m_xmlNode.attribute("sz", "1.0").remove('f').toFloat(), 5 );
}


void QSceneNode::setName(const QString& name)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("name", name);
		h3dSetNodeParamStr(m_hordeID, H3DNodeParams::NameStr, qPrintable(name));
	}
	else if (name!= QSceneNode::name())
		m_model->undoStack()->push(new QXmlNodePropertyCommand("Set Name", this, "Name", name, -1));
}

QVec3f QSceneNode::position() const
{

	return QVec3f(
		roundIt( m_xmlNode.attribute("tx", "0.0").remove('f').toFloat(), 5 ), 
		roundIt( m_xmlNode.attribute("ty", "0.0").remove('f').toFloat(), 5 ), 
		roundIt( m_xmlNode.attribute("tz", "0.0").remove('f').toFloat(), 5 ));
}

void QSceneNode::setPosition(const QVec3f& position)
{
	if (signalsBlocked())
	{
		const float* mat = 0;
		h3dGetNodeTransMats(m_hordeID, &mat, 0);
		// Normally this should never be occure, but in case of an invalid camera this may happen
		if (mat == 0) return;
		const_cast<float*>(mat)[12] = roundIt( position.X, 5 );
		const_cast<float*>(mat)[13] = roundIt( position.Y, 5 );
		const_cast<float*>(mat)[14] = roundIt( position.Z, 5 );
		h3dSetNodeTransMat(m_hordeID, mat);
		m_xmlNode.setAttribute("tx", QString::number( position.X, 'f', 5 ) );
		m_xmlNode.setAttribute("ty", QString::number( position.Y, 'f', 5 ));
		m_xmlNode.setAttribute("tz", QString::number( position.Z, 'f', 5 ));
	}
	else if (position != QSceneNode::position())
	{
		static const QString undoText = tr("Set Position");
		if (m_model->undoStack()->undoText() == undoText)
			m_model->undoStack()->push(new QXmlNodePropertyCommand(undoText, this, "Position", QVariant::fromValue(position), -1));
		else
			m_model->undoStack()->push(new QXmlNodePropertyCommand(undoText, this, "Position", QVariant::fromValue(position), TransformationID));
	}
}

QVec3f QSceneNode::rotation() const
{
	return QVec3f(
		roundIt( m_xmlNode.attribute("rx", "0.0").remove('f').toFloat(), 5 ), 
		roundIt( m_xmlNode.attribute("ry", "0.0").remove('f').toFloat(), 5 ), 
		roundIt( m_xmlNode.attribute("rz", "0.0").remove('f').toFloat(), 5 ));
}

void QSceneNode::setRotation(const QVec3f& rotation)
{
	if (signalsBlocked())
	{		
		QVec3f pos = position();
		QVec3f scaling = scale();
		h3dSetNodeTransform(m_hordeID, pos.X, pos.Y, pos.Z, rotation.X, rotation.Y, rotation.Z, scaling.X, scaling.Y, scaling.Z);
		m_xmlNode.setAttribute( "rx", QString::number( rotation.X, 'f', 5 ) );
		m_xmlNode.setAttribute( "ry", QString::number( rotation.Y, 'f', 5 ) );
		m_xmlNode.setAttribute( "rz", QString::number( rotation.Z, 'f', 5 ) );
	}
	else if (rotation != QSceneNode::rotation())
	{
		static const QString undoText = tr("Set Rotation");
		if (m_model->undoStack()->undoText() == undoText)
			m_model->undoStack()->push(new QXmlNodePropertyCommand(undoText, this, "Rotation", QVariant::fromValue(rotation), -1));
		else
			m_model->undoStack()->push(new QXmlNodePropertyCommand(undoText, this, "Rotation", QVariant::fromValue(rotation), TransformationID));
	
	}
}

QVec3f QSceneNode::scale() const
{
	return QVec3f(
		roundIt( m_xmlNode.attribute("sx", "1.0").remove('f').toFloat(), 5 ), 
		roundIt( m_xmlNode.attribute("sy", "1.0").remove('f').toFloat(), 5 ), 
		roundIt( m_xmlNode.attribute("sz", "1.0").remove('f').toFloat(), 5 ));
}

void QSceneNode::setScale(const QVec3f& scale)
{
	if (signalsBlocked())
	{		
		QVec3f pos = position();
		QVec3f rot = rotation();
		h3dSetNodeTransform(m_hordeID, pos.X, pos.Y, pos.Z, rot.X, rot.Y, rot.Z, scale.X, scale.Y, scale.Z);
		m_xmlNode.setAttribute( "sx", QString::number( scale.X, 'f', 5 ) );
		m_xmlNode.setAttribute( "sy", QString::number( scale.Y, 'f', 5 ) );
		m_xmlNode.setAttribute( "sz", QString::number( scale.Z, 'f', 5 ) );
	}
	else if (scale != QSceneNode::scale())
	{
		static const QString undoText = tr("Set Scale");
		if (m_model->undoStack()->undoText() == undoText)
			m_model->undoStack()->push(new QXmlNodePropertyCommand(undoText, this, "Scale", QVariant::fromValue(scale), -1));
		else
			m_model->undoStack()->push(new QXmlNodePropertyCommand(undoText, this, "Scale", QVariant::fromValue(scale), TransformationID));
	
	}
}


QVec3f QSceneNode::aabbMin() const
{
	if( m_hordeID == 0) return QVec3f();

	QVec3f min;	
	h3dGetNodeAABB(m_hordeID, &min.X, &min.Y, &min.Z, 0, 0, 0);
	return min;
}

QVec3f QSceneNode::aabbMax() const
{
	if( m_hordeID == 0) return QVec3f();

	QVec3f max;	
	h3dGetNodeAABB(m_hordeID, 0, 0, 0, &max.X, &max.Y, &max.Z);
	return max;
}

QMatrix4f QSceneNode::absTrans() const
{
	if( m_hordeID == 0) return QMatrix4f();
	const float* mat = 0;
	h3dGetNodeTransMats(m_hordeID, 0, &mat);
	if( mat ) return QMatrix4f(mat);
	else return QMatrix4f();
}

QMatrix4f QSceneNode::relTrans() const
{
	if( m_hordeID == 0) return QMatrix4f();
	const float* mat = 0;
	h3dGetNodeTransMats(m_hordeID, &mat, 0);
	if( mat ) return QMatrix4f(mat);
	else return QMatrix4f();
}

void QSceneNode::setRelTrans( const QMatrix4f& relTrans)
{
	// Does not update the XML representation
	h3dSetNodeTransMat(m_hordeID, relTrans.x);
}

bool QSceneNode::enabled() const
{
	return m_active;
}

void QSceneNode::setEnabled(bool enabled)
{
	h3dSetNodeFlags(m_hordeID, enabled ? 0 : H3DNodeFlags::Inactive, true );
	m_active = enabled;
}

QVariant QSceneNode::displayText( int column )
{
	switch( column )
	{
	case 0:
		return m_xmlNode.attribute("name", "ATTENTION no name set");
	case 1:
		return m_xmlNode.tagName();
	case 2: 
		return m_hordeID;
	default:
		return QVariant();
	}
}
