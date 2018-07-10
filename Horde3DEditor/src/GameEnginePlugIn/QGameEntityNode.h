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
#ifndef QGAMEENTITYNODE_H_
#define QGAMEENTITYNODE_H_

#include <QXmlTree/QXmlTreeNode.h>

/**
 * \brief An extra node for GameEntities providing no Horde3D representation
 * 
 * Normally if you want to add GameEntities you create an attachment for an existing Horde3D scene graph node. Therefore
 * you can use the "Add Attachment" command in the editor. But if you want to add a GameEntity with some components but not
 * a SceneGraphComponent, you can add an GameEntiy extra node. There you can add all the components you are also able to add
 * to attachment nodes.
 * 
 * @author Volker Wiendl
 * @date Jun 2008
 */ 
class QGameEntityNode : public QXmlTreeNode
{
	Q_OBJECT

	Q_PROPERTY(unsigned int Entity_ID READ ID DESIGNABLE true USER true)
	Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)
	Q_CLASSINFO("QGameEntityNode", "Game Entity");
public:
	virtual ~QGameEntityNode();
	
	/**
	 * \brief Static factory method
	 * 
	 * To be able to use a factory, we need a static method as a callback that can be registered in the factory. 
	 * This method will be registerd in the PlugInManager as an Extra node factory by the GameControllerAttachment using
	 * the plugin managers's registerExtraNode method.
	 * 
	 * @param xmlNode the XML representation for the GameEntity
	 * @param row index in the ExtraTreeModel
	 * @param model the ExtraTreeModel instance managing this node
	 * @param parent the parent node within the ExtraTreeModel
	 * @return a valid pointer to an QGameEntityNode
	 */ 
	static QXmlTreeNode* loadNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent);

	/**
	 * \brief Creates a new GameEntity
	 * 
	 * Calling this method will show an input dialog to let the user specify a new GameEntity name. If the user entered a valid 
	 * name, a new GameEntity XML representation will be created and returned
	 * @param parent a widget playing the parent role for the input dialog provided to the user
	 * @return a null node or a valid XML representation of a GameEntity node
	 */ 
	static QDomElement createNode(QWidget* parent);

	/**
	 * \brief GameEntity ID within the GameEngine's world
	 * 
	 * Each GameEntity has a unique dynamic world ID that can be requested with this method
	 * @return the GameEntity's world id.
	 */ 
	unsigned int ID() const;

	/**
	 * \brief GameEntity ID within the GameEngine
	 * 
	 * Each GameEntity has a unique static ID that can be used within scripts. It is defined in the data
	 * loaded by the GameEngine and such is always constant between different instances (unlike the world ID available through the ID() method). 
	 * @return the unique name of the GameEntity
	 */ 
	QString name() const;

	/**
	 * \brief Set the GameEntity ID within the GameEngine
	 * 
	 * @param name the new name of the GameEntity
	 */ 
	void setName(const QString& name);

protected:
	/**
	 * \brief Private Constructor
	 * 
	 * Creating new instances is only allowed by using loadNode()
	 * @param xmlNode 
	 * @param row 
	 * @param model 
	 * @param parent
	 */ 
	QGameEntityNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent = 0);

	/**
	 * \brief Returns false because QGameEntityNode can not have child nodes
	 * 
	 * @param QString name of a child node in the XML representation of this node
	 * @return false
	 */ 
	bool isKnown(const QString&) const {return false;}

	/**
	 * \brief Returns 0 because QGameEntityNode can not have child nodes
	 * 
	 * @param childNode the XML node that stores the component's configuration
	 * @param row child row index in the model
	 * @return always 0
	 */ 
	QXmlTreeNode* createChild(const QDomElement& childNode, int row) {return 0;}


};
#endif