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
#include "AttachmentTreeModel.h"

#include "QAttachmentNode.h"

AttachmentTreeModel::AttachmentTreeModel(QXmlTreeNode* sceneGraphNode, const QDomElement& attachmentNode) : QXmlTreeModel(sceneGraphNode)
{
	delete m_undoStack;
	m_undoStack = sceneGraphNode->model()->undoStack();
	m_rootItem = new QAttachmentNode(attachmentNode, 0, this);
	m_rootItem->childs();
}


AttachmentTreeModel::~AttachmentTreeModel()
{
}



QVariant AttachmentTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	QAttachmentNode *item = static_cast<QAttachmentNode *>(index.internalPointer());	
	switch(role)
	{
	case Qt::ToolTipRole:
		return item->xmlNode().tagName() + " Component";
	case Qt::DisplayRole:
		return item->xmlNode().tagName();
	default:
		return QVariant();
	};
}


Qt::ItemFlags AttachmentTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	else
		return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;		
}

QVariant AttachmentTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
	{
		switch (section) {
			 case 0:
				 return tr("Game Component");			
			 default:
				 return QVariant();
		}
	}
	return QVariant();
}


int AttachmentTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return 1;
}