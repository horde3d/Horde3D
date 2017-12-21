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

#ifndef QXMLNODEPROPERTYSETCOMMAND_H_
#define QXMLNODEPROPERTYSETCOMMAND_H_

#include "QXmlTreeNode.h"
#include "QXmlTreeModel.h"

#include <QApplication>

/**
 * Undo Command to change a property
 */
class QXmlNodePropertyCommand : public QUndoCommand
{

public:
	QXmlNodePropertyCommand(const QString& commandName, QXmlTreeNode* node, const QString& propertyName, const QVariant& propertyValue, int id) : QUndoCommand(commandName), 
		m_node(node), m_xmlNode(node->xmlNode()), m_model(static_cast<QXmlTreeModel*>(node->model())), m_propertyName(propertyName), m_newValue(propertyValue), m_id(id)
	{
		m_oldValue = node->property(qPrintable(propertyName));				
	}

	void undo()
	{		
		qApp->setOverrideCursor(Qt::BusyCursor);
		QModelIndex index;
		index = m_model->index(m_xmlNode); // since the == operator of QDomElement only returns true when the instance of the node is equal this should be safe
		m_node = static_cast<QXmlTreeNode*>(index.internalPointer());
		m_node->blockSignals(true);			
		m_node->setProperty(qPrintable(m_propertyName), m_oldValue);
		m_node->blockSignals(false);
		m_node->model()->updateIndex(index);
		qApp->restoreOverrideCursor();
	}

	void redo()
	{
		qApp->setOverrideCursor(Qt::BusyCursor);
		QModelIndex index;
		index = m_model->index(m_xmlNode);
		m_node = static_cast<QXmlTreeNode*>(index.internalPointer());
		m_node->blockSignals(true);
		m_node->setProperty(qPrintable(m_propertyName), m_newValue);
		m_node->blockSignals(false);
		m_node->model()->updateIndex(index);
		qApp->restoreOverrideCursor();
	}

	int id() const
	{
		return m_id;
	}

	bool mergeWith(const QUndoCommand* other)
	{
		if (other->id() != m_id || other->id() == -1) return false;
		const QXmlNodePropertyCommand* command = static_cast<const QXmlNodePropertyCommand*>(other);
		if (command->m_model != m_model || command->m_xmlNode != m_xmlNode || command->m_propertyName != m_propertyName) return false;
		m_newValue = command->m_newValue;
		return true;
	}

private:
	// Model containing the node
	QXmlTreeNode*			m_node;
	QDomElement				m_xmlNode;
	QXmlTreeModel*			m_model;
	QString					m_propertyName;
	QVariant				m_newValue;
	QVariant				m_oldValue;
	const int				m_id;
};

#endif
