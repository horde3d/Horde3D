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

#include "QXmlTreeUndoCommands.h"

#include "QXmlTreeNode.h"
#include "QXmlTreeModel.h"

#include <QApplication>

QAddXmlNodeUndoCommand::QAddXmlNodeUndoCommand(const QDomElement &node, const QDomElement &parent, QXmlTreeModel *model, const QString &text) : QUndoCommand(text), 
		m_addNode(node), m_parentNode(parent), m_model(model)
{

}

void QAddXmlNodeUndoCommand::undo()
{	
	qApp->setOverrideCursor(Qt::BusyCursor);
	QModelIndex index = m_model->index(m_addNode);
	Q_ASSERT(index.isValid());
	m_model->removeRow(index.row(), index.parent());	
	qApp->restoreOverrideCursor();
}

void QAddXmlNodeUndoCommand::redo()
{
	qApp->setOverrideCursor(Qt::BusyCursor);
	QModelIndex parentIndex = m_model->index(m_parentNode);
	m_model->addNode(m_model->rowCount(parentIndex), m_addNode, parentIndex);
	qApp->restoreOverrideCursor();
}


QRemoveXmlNodeUndoCommand::QRemoveXmlNodeUndoCommand(QXmlTreeNode *node, const QString &text) : QUndoCommand(text), 
	m_addNode(node->xmlNode()), m_parentNode(node->xmlNode().parentNode()), m_model(static_cast<QXmlTreeModel*>(node->model()))	
{
}

void QRemoveXmlNodeUndoCommand::undo()
{
	qApp->setOverrideCursor(Qt::BusyCursor);
	QModelIndex parentIndex = m_model->index(m_parentNode);
	m_model->addNode(m_model->rowCount(parentIndex), m_addNode, parentIndex);
	qApp->restoreOverrideCursor();
}

void QRemoveXmlNodeUndoCommand::redo()
{
	qApp->setOverrideCursor(Qt::BusyCursor);
	QModelIndex index = m_model->index(m_addNode);
	Q_ASSERT(index.isValid());
	m_model->removeRow(index.row(), index.parent());	
	qApp->restoreOverrideCursor();
}
