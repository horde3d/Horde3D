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

#ifndef SCENETREEMODEL_H_
#define SCENETREEMODEL_H_

#include <QXmlTree/QXmlTreeModel.h>
#include <QtXml/qdom.h>

class QSceneNode;
class QTreeView;
class QUndoStack;
class PlugInManager;

/**
 * The scene tree model manages the exposure, adding and removing of Horde3D elements to the scene graph
 */
class SceneTreeModel : public QXmlTreeModel
{
	friend class SceneTreeView;

	Q_OBJECT
public:

	SceneTreeModel(PlugInManager* factory, const QDomElement &node, QSceneNode* parentNode);
	virtual ~SceneTreeModel();

	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	Qt::ItemFlags flags(const QModelIndex &index) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const;

	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	// Custom implementation
	
	QSceneNode*	parentNode() const {return m_parentNode;}	

	PlugInManager* nodeFactory() const {return m_sceneNodeFactory;}

	Qt::DropActions supportedDragActions() const {return Qt::CopyAction | Qt::MoveAction;}
	Qt::DropActions supportedDropActions() const {return Qt::CopyAction | Qt::MoveAction;}

	QStringList mimeTypes() const;

	QMimeData* mimeData(const QModelIndexList &indexes) const;

	bool dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column, const QModelIndex &parent);

protected:

	QSceneNode*			m_parentNode;
	PlugInManager*	m_sceneNodeFactory;

};
#endif


