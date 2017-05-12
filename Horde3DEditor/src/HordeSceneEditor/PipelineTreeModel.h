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

#ifndef PIPELINETREEMODEL_H_
#define PIPELINETREEMODEL_H_

#include <QXmlTree/QXmlTreeModel.h>

class QTreeView;
class QExtrasNode;

/**
 * The scene tree model manages the exposure, adding and removing of extra nodes for the attachment plugin
 */
class PipelineTreeModel : public QXmlTreeModel
{

	Q_OBJECT
public:
	/**
	 * \brief Constructor
	 * @parent node root node for the extra tree model
	 * @parent parent associated QTreeView of this model
	 */
	PipelineTreeModel(unsigned int pipelineID, const QDomElement &node, QTreeView* parent);
	virtual ~PipelineTreeModel();

	virtual QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	//QModelIndex addNode(int row, const QDomElement& newChild, const QModelIndex& parent = QModelIndex());

protected:


};
#endif

