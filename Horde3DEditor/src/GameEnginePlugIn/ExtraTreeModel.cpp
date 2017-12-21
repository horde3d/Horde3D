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
// the Free Software Foundation version 2 of the License 
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

#include "ExtraTreeModel.h"

#include "QExtraNode.h"
#include "QGameEntityNode.h"

#include <QtCore/QStringList>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QUndoStack>

ExtraTreeModel::ExtraTreeModel(PlugInManager* factory, const QDomElement& extrasRoot, QObject* parent /*=0*/) : QXmlTreeModel(parent),
m_extraNodeFactory(factory)
{				
	m_rootItem = new QExtraNode(extrasRoot, 0, this, 0);
	// Populate tree
	m_rootItem->childs();
}


ExtraTreeModel::~ExtraTreeModel()
{
	//if (m_rootItem) qDebug("Warning rootitem of ExtraTreeModel will be not deleted");
	//m_rootItem = 0;
}

QVariant ExtraTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	QXmlTreeNode *item = static_cast<QXmlTreeNode*>(index.internalPointer());
	switch(role)
	{
	case Qt::ToolTipRole:
		return item->property("__ToolTip");
	case Qt::StatusTipRole:
		return item->property("__StatusTip");
	case Qt::DisplayRole:
	case Qt::EditRole:		
		{
			QGameEntityNode* node = qobject_cast<QGameEntityNode*>(item);
			if( node )
			{
				switch( index.column() )
				{
				case 0:
					return node->name();
				case 1:
					return item->xmlNode().tagName();
				case 2:
					return node->ID();
				}
			}
			else return item->xmlNode().tagName();
		}
	case Qt::CheckStateRole:		
		if( index.column() == 0 && item->property("Enabled").isValid() )
			return item->property("Enabled").toBool() ? Qt::Checked : Qt::Unchecked;
	default:
		return QVariant();
	};
}

Qt::ItemFlags ExtraTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid()) return Qt::ItemIsEnabled;
	if( index.column() == 0 && qobject_cast<QGameEntityNode*>( static_cast<QXmlTreeNode*>(index.internalPointer()) ) )
		return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	// don't allow change of any other attribute
	return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ExtraTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
	{
		switch( section )
		{
		case 0:
			return "Name";
		case 1:
			return "Type";
		case 2:
			return "ID";
		}
	}
	return QVariant();
}


int ExtraTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return 3;
}

