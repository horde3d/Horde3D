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

#include "PipelineTreeModel.h"
#include "QPipelineNode.h"
#include "QPipelineClearTargetNode.h"
#include "QPipelineDrawGeometryNode.h"
#include "QPipelineDoForwardLightLoopNode.h"
#include "QPipelineDrawQuadNode.h"

#include <QtCore/QStringList>
#include <QTreeView>
#include <QMessageBox>
#include <QUndoStack>

PipelineTreeModel::PipelineTreeModel(unsigned int pipelineID, const QDomElement &node, QTreeView* parent) : QXmlTreeModel()
{			
	m_rootItem = new QPipelineNode(pipelineID, node, 0, this);
	QAbstractItemModel::setParent(parent);
	m_rootItem->childs();
}


PipelineTreeModel::~PipelineTreeModel()
{
}

QVariant PipelineTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	QXmlTreeNode *item = static_cast<QXmlTreeNode*>(index.internalPointer());
	switch(role)
	{
	case Qt::BackgroundColorRole:
		if( item->property("Type").toInt() ==  QPipelineNode::PIPELINESTAGENODE )
			return QColor( 240, 240, 240 );
		else
			return QVariant();
	case Qt::ToolTipRole:
		return item->xmlNode().tagName() + " Node";
	case Qt::DisplayRole:
	case Qt::EditRole:
		{		
			if (index.column() == 0) 
				return item->xmlNode().tagName();

			switch (item->property("Type").toInt())
			{
			case QPipelineNode::PIPELINESETUPNODE:
				return item->xmlNode().tagName();
				break;
			case QPipelineNode::PIPELINERENDERTARGETNODE:
				return item->xmlNode().attribute("id", "Main Buffer");				
			case QPipelineNode::PIPELINESWITCHTARGETNODE:
				return item->xmlNode().attribute("target", "Main Buffer");				
			case QPipelineNode::PIPELINESTAGENODE:
				return item->xmlNode().attribute("id");				
			case QPipelineNode::PIPELINECLEARTARGETNODE:
				return ((QPipelineClearTargetNode*)item)->description();
			case QPipelineNode::PIPELINEDRAWGEOMETRYNODE:
				return ((QPipelineDrawGeometryNode*)item)->description();
			case QPipelineNode::PIPELINEDOFORWARDLIGHTLOOPNODE:
				return ((QPipelineDoForwardLightLoopNode*)item)->description();
			case QPipelineNode::PIPELINEDODEFERREDLIGHTLOOPNODE:
				return item->xmlNode().attribute( "context", tr("Default Light Context") );
			case QPipelineNode::PIPELINEDRAWOVERLAYSNODE:
				return item->xmlNode().attribute( "context", tr("None") );
			case QPipelineNode::PIPELINEDRAWQUADNODE:
				return ((QPipelineDrawQuadNode*)item)->description();
			default:
				return QVariant();
			}			
		}
	case Qt::CheckStateRole:
		if (index.column() == 0 && item->property("Type").toInt() == QPipelineNode::PIPELINESTAGENODE)
			return item->property("Enabled").toBool() ? Qt::Checked : Qt::Unchecked;
		else
			return QVariant();
	default:
		return QVariant();
	}
}

bool PipelineTreeModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
	if (index.isValid() && role == Qt::CheckStateRole) 
	{
		QXmlTreeNode *item = static_cast<QXmlTreeNode*>(index.internalPointer());		
		item->setProperty("Enabled", value.toBool());
		emit dataChanged(index, index);
		return true;	
	}
	return false;
}

Qt::ItemFlags PipelineTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	QXmlTreeNode *item = static_cast<QXmlTreeNode*>(index.internalPointer());
	if (item->property("Type").toInt() == QPipelineNode::PIPELINESTAGENODE && index.column() == 0)
		return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable; 
	else
		return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable; 
}

QVariant PipelineTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
	{
		switch (section) {
			 case 0:
				 return tr("Name");
			 case 1:
				 return tr("Type");
			 default:
				 return QVariant();
		}
	}
	return QVariant();
}

int PipelineTreeModel::columnCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
	return 2;
}
