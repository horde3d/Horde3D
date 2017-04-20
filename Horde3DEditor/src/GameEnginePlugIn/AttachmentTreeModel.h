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
#ifndef ATTACHMENTTREEMODEL_H_
#define ATTACHMENTTREEMODEL_H_

#include <QXmlTree/QXmlTreeModel.h>
#include <QXmlTree/QXmlTreeNode.h>

class AttachmentTreeModel : public QXmlTreeModel
{
	Q_OBJECT
public:
	AttachmentTreeModel(QXmlTreeNode* sceneGraphNode, const QDomElement& attachmentNode);

	virtual ~AttachmentTreeModel();

	QVariant data(const QModelIndex &index, int role) const;

	Qt::ItemFlags flags(const QModelIndex &index) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const;

	int columnCount(const QModelIndex &parent /*= QModelIndex()*/) const;

	/** 
	 * Returns the scene node the attachments are child of
	 */
	QXmlTreeNode* sceneNodeParent() {return static_cast<QXmlTreeNode*>(QObject::parent());}
};
#endif