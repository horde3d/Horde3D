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
#include "QAttachmentNode.h"

#include "AttachmentTreeModel.h"
#include "QComponentNode.h"
#include "QPositionNode.h"

#include <QXmlTree/QXmlTreeNode.h>
#include <QXmlTree/QXmlTreeModel.h>

#include <GameEngine/GameEngine.h>

/**
 * Undo Command to change an attachment property
 */
class AttachmentPropertyCommand : public QUndoCommand
{

public:
	AttachmentPropertyCommand(const QString& commandName, QXmlTreeNode* node, const QString& propertyName, const QVariant& propertyValue) : QUndoCommand(commandName), 
		m_propertyName(propertyName), m_newValue(propertyValue), m_xmlNode(node->xmlNode())
	{
		m_oldValue = node->property(qPrintable(propertyName));
		QXmlTreeNode* sceneNode = qobject_cast<AttachmentTreeModel*>(node->model())->sceneNodeParent();
		m_sceneXmlNode = sceneNode->xmlNode();
		m_sceneModel = static_cast<QXmlTreeModel*>(sceneNode->model());
	}

	void undo()
	{		
		// since the == operator of QDomElement only returns true when the instance of the node is equal this should be safe
		QModelIndex index = m_sceneModel->index(m_sceneXmlNode); 
		QXmlTreeNode* node = static_cast<QXmlTreeNode*>(index.internalPointer());
		AttachmentTreeModel* model = static_cast<AttachmentTreeModel*>(node->property("__AttachmentModel").value<void*>());
		index = model->index(m_xmlNode);
		node = static_cast<QXmlTreeNode*>(index.internalPointer());
		node->blockSignals(true);			
		node->setProperty(qPrintable(m_propertyName), m_oldValue);
		node->blockSignals(false);
		model->updateIndex(index);
	}

	void redo()
	{
		QModelIndex index = m_sceneModel->index(m_sceneXmlNode); 
		QXmlTreeNode* node = static_cast<QXmlTreeNode*>(index.internalPointer());
		AttachmentTreeModel* model = static_cast<AttachmentTreeModel*>(node->property("__AttachmentModel").value<void*>());
		index = model->index(m_xmlNode);
		node = static_cast<QXmlTreeNode*>(index.internalPointer());		
		node->blockSignals(true);
		node->setProperty(qPrintable(m_propertyName), m_newValue);
		node->blockSignals(false);
		node->model()->updateIndex(index);
	}

private:
	// Model containing the node	
	QDomElement				m_xmlNode;
	QDomElement				m_sceneXmlNode;
	QXmlTreeModel*			m_sceneModel;
	QString					m_propertyName;
	QVariant				m_newValue;
	QVariant				m_oldValue;
};



QAttachmentNode::QAttachmentNode(const QDomElement& xmlNode, int row, AttachmentTreeModel* model) : QXmlTreeNode(xmlNode, row, model)
{
	size_t size = GameEngine::componentNames(0, 0);
	char* components = new char[size];
	GameEngine::componentNames(components, size);
	m_knownNodeNames = QString(components).split(';', QString::SkipEmptyParts);
	delete[] components;
}
	

QAttachmentNode::~QAttachmentNode()
{
}

void QAttachmentNode::setName(const QString& name)
{
	if (signalsBlocked())
	{
		unsigned int entityID = GameEngine::entityWorldID(qPrintable(QAttachmentNode::name()));
		GameEngine::renameGameEntity(entityID, qPrintable(name));
		m_xmlNode.setAttribute("name", name);
	}
	else if (name!= QAttachmentNode::name())
		m_model->undoStack()->push(new AttachmentPropertyCommand("Set Name", this, "Name", name));
}

QXmlTreeNode* QAttachmentNode::createChild(const QDomElement& childNode, int row)
{
	//qDebug("Creating %s Child for %s", qPrintable(childNode.tagName()), qPrintable(childNode.parentNode().toElement().attribute("name"))); 
	QXmlTreeNode* childItem = 0;
	if (childNode.tagName() == "CrowdParticle")
		childItem = new QPositionNode(childNode, row, m_model, this);
	else if (childNode.tagName() == "CrowdVisNode")
		childItem = new QPositionNode(childNode, row, m_model, this);
	else if (childNode.tagName() == "Sound3D")
		childItem = new QPositionNode(childNode, row, m_model, this);
	else
	{
		QComponentNode* node = new QComponentNode(childNode, row, m_model, this);
		if( !node->isValid() )	delete node;		
		else childItem = node;
	}
	if( childItem ) childItem->childs();	
	return childItem;
}