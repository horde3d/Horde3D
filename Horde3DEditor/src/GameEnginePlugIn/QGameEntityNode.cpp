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
#include "QGameEntityNode.h"

#include "AttachmentTreeModel.h"

#include <QXmlTree/QXmlNodePropertyCommand.h>
#include <QtWidgets/QInputDialog>

#include <GameEngine/GameEngine.h>

QXmlTreeNode* QGameEntityNode::loadNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent)
{
	return new QGameEntityNode(xmlNode, row, model, parent);
}

QDomElement QGameEntityNode::createNode(QWidget* parent)
{
	QDomElement entityNode;
	QString name = QInputDialog::getText(parent, tr("GameEntity name"), tr("Specify new GameEntity name"));
	while(!name.isEmpty() && GameEngine::entityWorldID(qPrintable(name)) != 0)
	{
		name = QInputDialog::getText(parent, tr("GameEntity name"), tr("%1 already exists. Specify a new GameEntity name:").arg(name));
		if(name.isEmpty()) break;
	}
	if( !name.isEmpty() )
	{
		entityNode = QDomDocument().createElement("GameEntity");
		entityNode.setAttribute("name", name);
	}
	return entityNode;
}


QGameEntityNode::QGameEntityNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : QXmlTreeNode(xmlNode, row, model, parent)
{
	// Create only game entity not the attached components since they should be created separately by the corresponding Q classes
	GameEngine::createGameEntity( qPrintable( QString("<GameEntity name=\"%1\" />").arg(xmlNode.attribute("name")) ) );
	// Create a treemodel for the Attachment Widget
	AttachmentTreeModel* attachmentModel = new AttachmentTreeModel(this, m_xmlNode);
	// Store treemodel as dynamic property
	setProperty("__AttachmentModel", QVariant::fromValue<void*>(attachmentModel));	
}


QGameEntityNode::~QGameEntityNode()
{
	GameEngine::removeGameEntity(ID());
}

unsigned int QGameEntityNode::ID() const
{
	return GameEngine::entityWorldID( qPrintable(name()) );
}


QString QGameEntityNode::name() const
{
	return m_xmlNode.attribute("name");
}

void QGameEntityNode::setName(const QString &name)
{
	if (signalsBlocked())
	{
		if( GameEngine::entityWorldID( qPrintable( name ) ) == 0 )
		{
			unsigned int entityID = GameEngine::entityWorldID(qPrintable(QGameEntityNode::name()));
			GameEngine::renameGameEntity(entityID, qPrintable(name));
			m_xmlNode.setAttribute("name", name);
		}
	}
	else if (name != QGameEntityNode::name())
		m_model->undoStack()->push(new QXmlNodePropertyCommand("Set Name", this, "Name", name, -1));
}


