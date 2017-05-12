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

#include "QComponentNode.h"

#include <GameEngine/GameEngine.h>

#include <QtCore/qtextstream.h>
#include <QtCore/qvariant.h>

QComponentNode::QComponentNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent ) : QXmlTreeNode(xmlNode, row, model, parent)
{
	QString data;
	QTextStream stream(&data);
	xmlNode.save(stream, 4);
	unsigned int entityID = GameEngine::entityWorldID( qPrintable(parent->property("Name").toString()) );
	m_valid = GameEngine::setComponentData(entityID, qPrintable( xmlNode.tagName() ), qPrintable(data));
}


QComponentNode::~QComponentNode()
{
	unsigned int entityID = GameEngine::entityWorldID( qPrintable(QObject::parent()->property("Name").toString()) );
	GameEngine::setComponentData(entityID, qPrintable( m_xmlNode.tagName() ), 0);
}



