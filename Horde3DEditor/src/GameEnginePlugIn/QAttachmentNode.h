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
#ifndef QATTACHMENTNODE_H_
#define QATTACHMENTNODE_H_

#include <QXmlTree/QXmlTreeNode.h>

class AttachmentTreeModel;

/**
 * \brief Root node for AttachmentTreeModel
 * 
 * This class builds the root node for the AttachmentTreeModel displayed in the GameControllerWidget.
 * It handles the game components that may be attached to either a QGameEntityNode in the ExtraTreeModel
 * or a Attachment XML child node of a QSceneNode. 
 * 
 * @author Volker Wiendl
 * @date Jun 2008
 */ 
class QAttachmentNode : public QXmlTreeNode
{
	Q_OBJECT

	Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)

public:
	/**
	 * \brief Constructor
	 * 
	 * Creates a new QAttachmentNode instance that builds the root node of an AttachmentTreeModel
	 * 
	 * @param xmlNode the XML node that contains the name attribute for the GameEntity (may be an attachment node in the scenegraph or
	 * a GameEntity node in the ExtraTreeModel)
	 * @param row index in the model containing this node
	 * @param model the model hosting this node
	 */ 
	QAttachmentNode(const QDomElement& xmlNode, int row, AttachmentTreeModel* model);
	virtual ~QAttachmentNode();


	/**
	 * \brief GameEntity name attribute value
	 * @return name attribute of the current GameEntity or Attachment node (based on the xmlNode that was provided to the constructor)
	 */
	QString name() const {return m_xmlNode.attribute("name");}

	/**
	 * \brief Sets the GameEntity name of this node
	 * 
	 * @param name the new entity name
	 */ 
	void setName(const QString& name);

protected:	
	/**
	 * \brief Creates GameComponent node representations
	 * 
	 * @param childNode the XML node that stores the component's configuration
	 * @param row child row index in the model
	 * @return a new GameComponent representation or 0 if no valid representation could be created for the given XML node
	 */ 
	QXmlTreeNode* createChild(const QDomElement& childNode, int row);

};
#endif