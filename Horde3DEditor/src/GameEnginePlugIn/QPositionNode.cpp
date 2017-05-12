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
#include "QPositionNode.h"

#include "AttachmentTreeModel.h"
#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEngine_SceneGraph.h>

#include <CustomTypes.h>

#include <QtCore/qtextstream.h>
#include <QtCore/qvariant.h>
#include <QtGui/qevent.h>

QPositionNode::QPositionNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent) : QXmlTreeNode(xmlNode, row, model, parent),
m_dynamicProperty(false)
{
	QString data;
	QTextStream stream(&data);
	xmlNode.save(stream, 4);
	m_entityID = GameEngine::entityWorldID( qPrintable(parent->property("Name").toString()) );
	GameEngine::setComponentData(m_entityID, qPrintable( xmlNode.tagName() ), qPrintable(data));
	QXmlTreeNode* root = static_cast<AttachmentTreeModel*>(model)->sceneNodeParent();
	// If the entity has no position we have to create one for the crowd particle
	if( !root->property("Position").isValid() )
	{
		QVec3f pos( m_xmlNode.attribute("x").toFloat(), m_xmlNode.attribute("y").toFloat(), m_xmlNode.attribute("z").toFloat() );
		// Since the parent doesn't have a position yet we create one dynamically (not necessary if the particle is a child of 
		// an entity created by an attachment of a Horde3D scene graph node )
		m_dynamicProperty = true;
		root->setProperty( "Position", QVariant::fromValue(pos) );
		// We have to add scale too, otherwise the transformation changes in the editor are not possible
		root->setProperty( "Scale", QVariant::fromValue( QVec3f(1, 1, 1) ) );
		root->installEventFilter(this);
		QVariant transProp = root->property("__AbsoluteTransformation");
		if( !transProp.isValid() )
		{
			root->setProperty("__AbsoluteTransformation", 
				QVariant::fromValue(QMatrix4f::TransMat( pos.X, pos.Y, pos.Z) ) );
			root->setProperty("__RelativeTransformation", 
				QVariant::fromValue(QMatrix4f::TransMat( pos.X, pos.Y, pos.Z) ) );
		}
	}
	else // Remove any position attribute, since the position should be used from the scene node
	{
		m_xmlNode.removeAttribute("x");
		m_xmlNode.removeAttribute("y");
		m_xmlNode.removeAttribute("z");
	}
}


QPositionNode::~QPositionNode()
{
	unsigned int entityID = GameEngine::entityWorldID( qPrintable(QObject::parent()->property("Name").toString()) );
	GameEngine::setComponentData(entityID, qPrintable( m_xmlNode.tagName() ), 0);
	if( m_dynamicProperty )
	{		
		QXmlTreeNode* root = static_cast<AttachmentTreeModel*>(m_model)->sceneNodeParent();
		root->setProperty("Position", QVariant());
		root->setProperty("Scale", QVariant());
		root->setProperty("__AbsoluteTransformation", QVariant() );
		root->setProperty("__RelativeTransformation", QVariant() );
	}
}



bool QPositionNode::eventFilter(QObject* obj, QEvent* event)
{
	if( event->type() == QEvent::DynamicPropertyChange )
	{
		QDynamicPropertyChangeEvent* ev = static_cast<QDynamicPropertyChangeEvent*>(event);
		if( ev->propertyName() == "Position" )
		{
			QVec3f pos = obj->property("Position").value<QVec3f>();
			savePosition( pos.X, pos.Y, pos.Z );
			obj->setProperty("__AbsoluteTransformation", QVariant::fromValue(QMatrix4f::TransMat( pos.X, pos.Y, pos.Z) ) );
		}
		if( ev->propertyName() == "Scale" )
		{
			// No scale allowed
			obj->setProperty("Scale", QVariant::fromValue(QVec3f(1, 1, 1)) );
		}
		// Set absolute transformation to the relative one but don't apply it to the xml data
		if( ev->propertyName() == "__RelativeTransformation" )
		{
			QMatrix4f mat = obj->property("__RelativeTransformation").value<QMatrix4f>();
			obj->setProperty("__AbsoluteTransformation", QVariant::fromValue(mat) );

		}
	}
	return QXmlTreeNode::eventFilter(obj, event);
}

void QPositionNode::savePosition( float x, float y, float z )
{
	m_xmlNode.setAttribute("x", x);
	m_xmlNode.setAttribute("y", y);
	m_xmlNode.setAttribute("z", z);
	setPosition( x, y, z );
}

void QPositionNode::setPosition( float x, float y, float z )
{
	GameEngine::setEntityTranslation( m_entityID, x, y, z);
}