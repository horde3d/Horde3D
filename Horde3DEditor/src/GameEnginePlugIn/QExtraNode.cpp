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
#include "QExtraNode.h"

#include "PlugInManager.h"

#include "ExtraTreeModel.h"

QExtraNode::QExtraNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : QXmlTreeNode(xmlNode, row, model, parent)
{
	m_knownNodeNames << "GameEntity";	
}


QExtraNode::~QExtraNode()
{
}

QXmlTreeNode* QExtraNode::createChild(const QDomElement &childNode, int row)
{	
	QXmlTreeNode* childItem = static_cast<ExtraTreeModel*>(m_model)->nodeFactory()->loadExtraNode(childNode, row, m_model, this);
	// Populate Tree
	if (childItem) childItem->childs();
	return childItem;
}

void QExtraNode::setModel(QXmlTreeModel* model)
{
	m_model = model;
}

QXmlTreeModel* QExtraNode::getModel()
{
	return m_model;
}