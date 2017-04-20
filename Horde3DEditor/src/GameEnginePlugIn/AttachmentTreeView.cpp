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
#include "AttachmentTreeView.h"

#include "AttachmentTreeModel.h"
#include <QXmlTree/QXmlTreeUndoCommands.h>

#include <QtWidgets/qaction.h>
#include <QtXml/qdom.h>


class RemoveNodeUndoCommand : public QRemoveXmlNodeUndoCommand
{
public:
	RemoveNodeUndoCommand(QXmlTreeNode* node, const QString& text) : QRemoveXmlNodeUndoCommand(node, text)
	{
		QXmlTreeNode* sceneNode = qobject_cast<AttachmentTreeModel*>(node->model())->sceneNodeParent();
		m_sceneXmlNode = sceneNode->xmlNode();
		m_model = static_cast<QXmlTreeModel*>(sceneNode->model());
	}

	void undo()
	{
		QModelIndex index = m_model->index(m_sceneXmlNode); 
		Q_ASSERT(index.isValid());
		QXmlTreeNode* node = static_cast<QXmlTreeNode*>(index.internalPointer());
		AttachmentTreeModel* model = static_cast<AttachmentTreeModel*>(node->property("__AttachmentModel").value<void*>());
		QModelIndex parentIndex = m_model->index(m_parentNode);
		model->addNode(model->rowCount(parentIndex), m_addNode, parentIndex);
		// force update
		model->rowCount();
	}	

	void redo()
	{
		QModelIndex index = m_model->index(m_sceneXmlNode); 
		Q_ASSERT(index.isValid());
		QXmlTreeNode* node = static_cast<QXmlTreeNode*>(index.internalPointer());
		AttachmentTreeModel* model = static_cast<AttachmentTreeModel*>(node->property("__AttachmentModel").value<void*>());
		index = model->index(m_addNode);
		Q_ASSERT(index.isValid());
		model->removeRow(index.row(), index.parent());	
	}

private:
	QDomElement		m_sceneXmlNode;
};

class AddNodeUndoCommand : public QAddXmlNodeUndoCommand
{
public:
	AddNodeUndoCommand(const QDomElement& node, const QDomElement& parent, AttachmentTreeModel* model, const QString& text) : QAddXmlNodeUndoCommand(node, parent, model, text)
	{
		QXmlTreeNode* sceneNode = model->sceneNodeParent();
		m_sceneXmlNode = sceneNode->xmlNode();
		m_model = static_cast<QXmlTreeModel*>(sceneNode->model());
	}

	void redo()
	{
		QModelIndex index = m_model->index(m_sceneXmlNode); 
		Q_ASSERT(index.isValid());
		QXmlTreeNode* node = static_cast<QXmlTreeNode*>(index.internalPointer());
		AttachmentTreeModel* model = static_cast<AttachmentTreeModel*>(node->property("__AttachmentModel").value<void*>());
		QModelIndex parentIndex = m_model->index(m_parentNode);
		model->addNode(model->rowCount(parentIndex), m_addNode, parentIndex);
	}	

	void undo()
	{
		QModelIndex index = m_model->index(m_sceneXmlNode); 
		Q_ASSERT(index.isValid());
		QXmlTreeNode* node = static_cast<QXmlTreeNode*>(index.internalPointer());
		AttachmentTreeModel* model = static_cast<AttachmentTreeModel*>(node->property("__AttachmentModel").value<void*>());
		index = model->index(m_addNode);
		Q_ASSERT(index.isValid());
		model->removeRow(index.row(), index.parent());	
	}

private:
	QDomElement		m_sceneXmlNode;
};


AttachmentTreeView::AttachmentTreeView(QWidget* parent /*= 0*/) : QXmlTreeView(parent)
{
}


AttachmentTreeView::~AttachmentTreeView()
{
}


void AttachmentTreeView::addPhysicsComponent()
{
	AttachmentTreeModel* model = qobject_cast<AttachmentTreeModel*>(QAbstractItemView::model());	
	model->undoStack()->push(createAddUndoCommand(QDomDocument().createElement("BulletPhysics"), model->rootNode()->xmlNode(), model, tr("Add Physics Component")));
	setCurrentIndex(model->index( model->rowCount(), 0));
}

void AttachmentTreeView::addTTSComponent()
{
	AttachmentTreeModel* model = qobject_cast<AttachmentTreeModel*>(QAbstractItemView::model());	
	model->undoStack()->push(createAddUndoCommand(QDomDocument().createElement("TTS"), model->rootNode()->xmlNode(), model, tr("Add TTS Component")));
	setCurrentIndex(model->index( model->rowCount(), 0));
}

void AttachmentTreeView::addKeyframeAnimComponent()
{
	AttachmentTreeModel* model = qobject_cast<AttachmentTreeModel*>(QAbstractItemView::model());	
	model->undoStack()->push(createAddUndoCommand(QDomDocument().createElement("KeyframeAnimation"), model->rootNode()->xmlNode(), model, tr("Add Keyframe Animation Component")));		
	setCurrentIndex(model->index( model->rowCount(), 0));
}

void AttachmentTreeView::addDynamidComponent()
{
	AttachmentTreeModel* model = qobject_cast<AttachmentTreeModel*>(QAbstractItemView::model());	
	model->undoStack()->push(createAddUndoCommand(QDomDocument().createElement("Dynamid"), model->rootNode()->xmlNode(), model, tr("Add Dynamid Component")));		
}

void AttachmentTreeView::addCrowdParticleComponent()
{
	AttachmentTreeModel* model = qobject_cast<AttachmentTreeModel*>(QAbstractItemView::model());	
	model->undoStack()->push(createAddUndoCommand(QDomDocument().createElement("CrowdParticle"), model->rootNode()->xmlNode(), model, tr("Add Crowd Particle Component")));
	setCurrentIndex(model->index( model->rowCount(), 0));
}

void AttachmentTreeView::addCrowdVisNodeComponent()
{
	AttachmentTreeModel* model = qobject_cast<AttachmentTreeModel*>(QAbstractItemView::model());	
	model->undoStack()->push(createAddUndoCommand(QDomDocument().createElement("CrowdVisNode"), model->rootNode()->xmlNode(), model, tr("Add Crowd Visibility Node Component")));		
	setCurrentIndex(model->index( model->rowCount(), 0));
}

void AttachmentTreeView::addSound3DComponent()
{
	AttachmentTreeModel* model = qobject_cast<AttachmentTreeModel*>(QAbstractItemView::model());	
	model->undoStack()->push(createAddUndoCommand(QDomDocument().createElement("Sound3D"), model->rootNode()->xmlNode(), model, tr("Add Sound3D Component")));		
	setCurrentIndex(model->index( model->rowCount(), 0));
}

QAddXmlNodeUndoCommand* AttachmentTreeView::createAddUndoCommand(const QDomElement& node, const QDomElement& parent, QXmlTreeModel* model, const QString& text) const
{
	return new AddNodeUndoCommand(node, parent, qobject_cast<AttachmentTreeModel*>(model), text);
}


QRemoveXmlNodeUndoCommand* AttachmentTreeView::createRemoveUndoCommand(QXmlTreeNode* node, const QString& text) const
{
	return new RemoveNodeUndoCommand(node, text);
}



