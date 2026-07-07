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

#include "SceneTreeModel.h"
#include "QSceneNode.h"
#include "PlugInManager.h"

#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/qmimedata.h>
#include <QtCore/QSignalMapper>
#include <QTreeView>
#include <QMessageBox>
#include <QApplication>
#include <QtCore/QDir>

#include <Horde3D.h>

SceneTreeModel::SceneTreeModel(PlugInManager* factory, const QDomElement &node, QSceneNode* parentNode) : QXmlTreeModel(), 
m_parentNode(parentNode), m_sceneNodeFactory(factory)
{	
	m_rootItem = m_sceneNodeFactory->loadSceneNode(node, 0, this, parentNode);
	if (m_rootItem)
	{
		// Apply settings from reference node to root node of this model
		// Initialize transformation of reference nodes before creating the child nodes, 
		// otherwise the absolute transformations will be initialized to late for attachments
		QSceneNode* root = static_cast<QSceneNode*>(m_rootItem);
		float x, y, z, rx, ry, rz, sx, sy, sz;
		parentNode->getTransformation(x,y,z,rx,ry,rz,sx,sy,sz);
		h3dSetNodeTransform(root->hordeId(), x, y, z, rx, ry, rz, sx, sy, sz);
		// Set node name
		h3dSetNodeParamStr(root->hordeId(), H3DNodeParams::NameStr, qPrintable(parentNode->name()));

		// Populate tree
		m_rootItem->childs();
	}
}


SceneTreeModel::~SceneTreeModel()
{
}

QVariant SceneTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	QSceneNode *item = static_cast<QSceneNode*>(index.internalPointer());
	switch(role)
	{
	case Qt::ToolTipRole:
	case Qt::StatusTipRole:
		switch (index.column())
		{
		case 0:
			return tr("Double click to rename this node");
		default:		
			return item->toolTip(index.column());			
		}
	case Qt::DisplayRole:
	case Qt::EditRole:
		return item->displayText(index.column());		
	case Qt::CheckStateRole:
		if (index.column() == 0)
			return item->enabled() ? Qt::Checked : Qt::Unchecked;
		else
			return QVariant();
	default:
		return QVariant();
	};
}

bool SceneTreeModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
	if (index.isValid()) 
	{
		QSceneNode *item = static_cast<QSceneNode*>(index.internalPointer());		
		if (role == Qt::EditRole)
			item->setName(value.toString());
		else if (role == Qt::CheckStateRole)
			item->setEnabled(value.toBool());
		emit dataChanged(index, index);
		return true;	
	}
	return false;
}

Qt::ItemFlags SceneTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
	// only allow change of name attribute
	if (index.column() == 0)		
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
	else
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
		
}

QVariant SceneTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
	{
		switch (section) {
			 case 0:
				 return tr("Name");
			 case 1:
				 return tr("Type");
			 case 2:
				 return tr("ID");
			 default:
				 return QVariant();
		}
	}
	return QVariant();
}


int SceneTreeModel::columnCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
	return 3;
}

QStringList SceneTreeModel::mimeTypes() const
{
	return QStringList("application/xml");
}

QMimeData* SceneTreeModel::mimeData(const QModelIndexList &indexes) const
{
	QDomDocument document("Paste");		
	int row = -1;
    foreach (QModelIndex index, indexes) 
	{
        if (index.isValid() && index.row() != row) 
		{
			row = index.row();
			QXmlTreeNode* xmlNode = static_cast<QXmlTreeNode*>(index.internalPointer());
			document.appendChild(xmlNode->xmlNode().cloneNode());					
        }
    }

	QString data;
	QTextStream stream(&data);	
	document.save(stream,4);

	QMimeData* mime = new QMimeData();
	mime->setData("application/xml", data.toUtf8());	
    return mime;
}

bool SceneTreeModel::dropMimeData(const QMimeData *mime, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex & /*parent*/)
 {
     if (action == Qt::IgnoreAction)
         return true;

     if (!mime->hasFormat("application/xml"))
         return false;

	QDomDocument dom("Paste");
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
	return dom.setContent(mime->data("application/xml"));
#else
	return (bool) dom.setContent(mime->data("application/xml"));
#endif
	// Don't add it, since we want to use the view's undo redo mechanism
	//if (dom.setContent(mime->data("application/xml")))
	//{
	//	QXmlTreeNode* node = m_rootItem;
	//	if (parent.isValid())
	//		node = static_cast<QXmlTreeNode*>(parent.internalPointer());		
	//	addNode(rowCount(parent), dom.documentElement(), parent);
	//}
	//return true;
}
