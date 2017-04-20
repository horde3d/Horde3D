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

#include "QXmlTreeView.h"

#include "QXmlTreeModel.h"
#include "QXmlTreeNode.h"
#include "QXmlTreeUndoCommands.h"

#include <QTextStream>
#include <QApplication>
#include <QClipboard>
#include <qmimedata.h>
#include <QMessageBox>

QXmlTreeView::QXmlTreeView(QWidget* parent /*= 0*/) : QTreeView(parent)
{
	
}


QXmlTreeView::~QXmlTreeView()
{
}


void QXmlTreeView::setModel(QAbstractItemModel *model)
{
	QTreeView::setModel(model);
	connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(currentNodeChanged(const QModelIndex&, const QModelIndex&)));
	currentNodeChanged(currentIndex(), QModelIndex());
}

QXmlTreeNode* QXmlTreeView::currentNode()
{
	return static_cast<QXmlTreeNode*>(currentIndex().internalPointer());
}

void QXmlTreeView::currentNodeChanged(const QModelIndex& current, const QModelIndex& previous)
{
	if (current.isValid() && current.internalPointer() != previous.internalPointer())
	{
		QXmlTreeNode* node(static_cast<QXmlTreeNode*>(current.internalPointer()));
		emit currentNodeChanged(node);
	}
	else if (!current.isValid() && previous.isValid())
	{
		emit currentNodeChanged(0);//qobject_cast<SceneTreeModel*>(model())->m_rootItem);
		clearSelection();
	}
}


void QXmlTreeView::copyCurrentNode()
{
	QXmlTreeModel* treeModel = static_cast<QXmlTreeModel*>(model());
	if (!treeModel)	return;
	QString data;
	QTextStream stream(&data);	
	QDomDocument document("Paste");		
	QXmlTreeNode* xmlNode = treeModel->rootNode();
	if (currentIndex().isValid())
		xmlNode = static_cast<QXmlTreeNode*>(currentIndex().internalPointer());		
	document.appendChild(xmlNode->xmlNode().cloneNode());					
	document.save(stream,4);
	QClipboard* clipboard = QApplication::clipboard();
	QMimeData* mime = new QMimeData();
	mime->setData("text/plain", data.toUtf8());	
	clipboard->setMimeData(mime);
}

void QXmlTreeView::pasteNode()
{
	QXmlTreeModel* treeModel = static_cast<QXmlTreeModel*>(model());
	if( !treeModel ) return;

	QXmlTreeNode* xmlNode = treeModel->rootNode();
	if (currentIndex().isValid())
		xmlNode = static_cast<QXmlTreeNode*>(currentIndex().internalPointer());		

	const QMimeData* mime = QApplication::clipboard()->mimeData();;
	if (mime->hasFormat("text/plain"))
	{
		QDomDocument dom("Paste");
		if (dom.setContent(mime->data("text/plain")))
			treeModel->undoStack()->push(createAddUndoCommand(dom.documentElement(), xmlNode->xmlNode(), treeModel, tr("Paste nodes")));
	}
}

void QXmlTreeView::removeCurrentNode(bool confirm /*= true*/)
{	
	QModelIndex index = currentIndex();
	if (!index.isValid())
	{
		QMessageBox::information(this, tr("Attention"), tr("Removing the root node is not possible, please select a childnode first"));
		return;
	}
	if (confirm && QMessageBox::question(
		this, 
		tr("Remove nodes?"), 
		tr("Do you really want to remove the selected node\nand all of it's subnodes?"), 
		QMessageBox::Ok | QMessageBox::Cancel, 
		QMessageBox::Ok) != QMessageBox::Ok)
		return;
	QXmlTreeModel* treeModel = static_cast<QXmlTreeModel*>(model());
	QXmlTreeNode* node = static_cast<QXmlTreeNode*>(index.internalPointer());
		treeModel->undoStack()->push(createRemoveUndoCommand(node, tr("Remove node")));
}

QAddXmlNodeUndoCommand* QXmlTreeView::createAddUndoCommand(const QDomElement& node, const QDomElement& parent, QXmlTreeModel* model, const QString& text) const
{
	return new QAddXmlNodeUndoCommand(node, parent, model, text);
}

QRemoveXmlNodeUndoCommand* QXmlTreeView::createRemoveUndoCommand(QXmlTreeNode* node, const QString& text) const
{
	return new QRemoveXmlNodeUndoCommand(node, text);
}
