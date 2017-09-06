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

#ifndef QXMLTREENODE_H_
#define QXMLTREENODE_H_

#include <QtCore/QObject>
#include <QtXml/qdom.h>
#include <QtCore/QList>
#include <QtCore/QStringList>

class QXmlTreeModel;

/**
 * This class represents a node in a QXmlTreeModel
 */
class QXmlTreeNode : public QObject
{
	Q_OBJECT

public:

	/**
	 * \brief Constructor
	 *
	 */
	QXmlTreeNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent = 0);

	/// Destructor
	virtual ~QXmlTreeNode();

	/**
	 * Number of childs
	 * @return number of childs
	 */
	unsigned int childs();

	/**
	 * \brief Returns the ith child QXmlTreeNode of this node
	 *
	 * This method is based on the protected abstract methods createChild() and isKnown().
	 * If isKnown returns true a new QXmlTreeNode* instance will be created and cached by using createChild() 
	 * to represent the XML data of the ith child	 
	 * @param i the ith child of this instance
	 * @return a new item instance to represent the xml node in a model
	 */
	QXmlTreeNode* child(int i);

	/**
	 * \brief Returns the child that represents the given node instance 
	 * @param node the node you search the item for 
	 *        (note: it will be searched for this special node instance not for an XML node that represents the same data)
	 */
	QXmlTreeNode* child(const QDomNode& node);

	/**
	 * The parent QXmlTreeNode of this node if any set
	 */
	QXmlTreeNode* parent() {return static_cast<QXmlTreeNode*>(QObject::parent());}

	/**
	 * The xml representation for this node
	 */
	QDomElement& xmlNode() {return m_xmlNode;} 

	/**
	 * Returns the row of the item in a QXmlTreeModel
	 */
	int row() const {return m_row;}

	/**
	 * Deletes the ith child from the tree
	 * @param i the index of the child to remove
	 */
	void removeChild(int i);


	/**
	 * The scene tree model that contains this node	 
	 */
	QXmlTreeModel* model() {return m_model;} 


protected:
	virtual bool isKnown(const QString& tagName) const {return m_knownNodeNames.isEmpty() || m_knownNodeNames.contains(tagName);}

	virtual QXmlTreeNode* createChild(const QDomElement& childNode, int row) = 0;

        QXmlTreeModel*                  m_model;

        QDomElement                     m_xmlNode;
	QList<QXmlTreeNode*>		m_childItems;	
        int                             m_row;
        QStringList                     m_knownNodeNames;

};
#endif



