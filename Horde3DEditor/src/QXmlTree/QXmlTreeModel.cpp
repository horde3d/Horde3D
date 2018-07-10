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

#include "QXmlTreeModel.h"

#include "QXmlTreeNode.h"

QXmlTreeModel::QXmlTreeModel(QObject* parent /*=0*/) : QAbstractItemModel(parent), m_rootItem(0)
{	
	m_undoStack = new QUndoStack(this);
}


QXmlTreeModel::~QXmlTreeModel()
{
	delete m_rootItem;
}



QModelIndex QXmlTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
	QXmlTreeNode *parentItem = m_rootItem;
	if (parent.isValid())
		parentItem = static_cast<QXmlTreeNode*>(parent.internalPointer());

	QXmlTreeNode *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}


QModelIndex QXmlTreeModel::parent(const QModelIndex &child) const
{
	if (!child.isValid()) return QModelIndex();

	QXmlTreeNode *childItem = static_cast<QXmlTreeNode*>(child.internalPointer());
	QXmlTreeNode *parentItem = childItem->parent();

	if (!parentItem || parentItem == m_rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int QXmlTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	QXmlTreeNode *parentItem = m_rootItem;
	if (parent.isValid())
		parentItem = static_cast<QXmlTreeNode*>(parent.internalPointer());	
	return parentItem->childs();
}

void QXmlTreeModel::setRootNode(QXmlTreeNode *node)
{	
	if (m_rootItem)
	{
		beginRemoveRows( QModelIndex(), 0, m_rootItem->childs() );
		delete m_rootItem;
		m_rootItem = 0;
		endRemoveRows();
	}
	if( node )
	{
		beginInsertRows( QModelIndex(), 0, node->childs() );
		m_rootItem = node;
		endInsertRows();
	}
	beginResetModel();
    endResetModel();
}

QModelIndex QXmlTreeModel::addNode(int row, QDomElement& newChild, const QModelIndex& parent /* = QModelIndex()*/)
{
	beginInsertRows(parent, row, row);
	QXmlTreeNode *item = m_rootItem;
	if (parent.isValid())
		item = static_cast<QXmlTreeNode*>(parent.internalPointer());
	// TODO use insert according to the row instead of appending it
	item->xmlNode().appendChild(newChild);	
	endInsertRows();
	// shouldn't be necessary but it is
	beginResetModel();
    endResetModel();
	return index(row, 0, parent);
}

QModelIndex QXmlTreeModel::index(const QDomNode& node, const QModelIndex& parent /*= QModelIndex()*/) const
{
	QXmlTreeNode* parentItem = m_rootItem;
	if (parent.isValid())
		parentItem = static_cast<QXmlTreeNode*>(parent.internalPointer());
	if (parentItem->xmlNode() == node)
		return createIndex(-1, 0, parentItem); // The parent itself creates a special modelindex (is this valid?)
	QXmlTreeNode* childItem = parentItem->child(node);
	if (childItem)
		return createIndex(childItem->row(), 0, childItem);
	else
		return QModelIndex();
}

bool QXmlTreeModel::removeRows(int row, int count, const QModelIndex& parent /*= QModelIndex()*/)
{
	beginRemoveRows(parent, row, row+count-1);
	QXmlTreeNode *item = m_rootItem;
	if (parent.isValid())
		item = static_cast<QXmlTreeNode*>(parent.internalPointer());	
	for (int i=row; i<row+count; ++i)
		item->removeChild(i);	
	endRemoveRows();
	return true;
}

void QXmlTreeModel::updateIndex(const QModelIndex& index)
{
	emit dataChanged(index, index);
}