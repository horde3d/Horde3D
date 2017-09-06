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
#ifndef QXMLTREEMODEL_H_
#define QXMLTREEMODEL_H_

#include <QAbstractItemModel>
#include <QUndoStack>
#include <QDomElement>

class QXmlTreeNode;

class QXmlTreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	QXmlTreeModel(QObject* parent = 0);
	virtual ~QXmlTreeModel();
	
	// QAbstractItemModel Implementations
	virtual QVariant data(const QModelIndex &index, int role) const = 0;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const = 0;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const = 0;

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

	QModelIndex parent(const QModelIndex &child) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
	
	bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

	// Custom implementation
	virtual void setRootNode( QXmlTreeNode* node );

	QModelIndex addNode(int row, QDomElement& newChild, const QModelIndex& parent = QModelIndex());

	void updateIndex(const QModelIndex&);

	QModelIndex index(const QDomNode& node, const QModelIndex& parent = QModelIndex()) const;

	QXmlTreeNode* rootNode() {return m_rootItem;}
	/**
	 * The tree model have its own undo stack
	 * @return QUndoStack* pointer to an undostack for this model
	 */
	QUndoStack*	undoStack() {return m_undoStack;}


protected:
	QXmlTreeNode*			m_rootItem;

	QUndoStack*				m_undoStack;
};
#endif
