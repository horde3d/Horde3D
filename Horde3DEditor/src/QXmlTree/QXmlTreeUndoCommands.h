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

#ifndef QXMLTREEUNDOCOMMANDS_H_
#define QXMLTREEUNDOCOMMANDS_H_

class QXmlTreeNode;
class QXmlTreeModel;

#include <QUndoStack>
#include <QDomElement>
#include <QAbstractItemModel>

class QRemoveXmlNodeUndoCommand : public QUndoCommand
{
public:
	QRemoveXmlNodeUndoCommand(QXmlTreeNode* node, const QString& text);
	
	virtual void undo();

	virtual void redo();

protected:	
	QDomElement				m_addNode;
	QDomNode				m_parentNode;
	QXmlTreeModel*			m_model;
};

class QAddXmlNodeUndoCommand : public QUndoCommand
{
public:
	QAddXmlNodeUndoCommand(const QDomElement& node, const QDomElement& parent, QXmlTreeModel* model, const QString& text);	

	virtual void undo();

	void redo();

protected:	
	QDomElement				m_addNode;
	QDomElement				m_parentNode;
	QXmlTreeModel*			m_model;
};


#endif
