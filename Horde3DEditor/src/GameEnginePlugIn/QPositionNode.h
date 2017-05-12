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
#ifndef QPOSITIONNODE_H_
#define QPOSITIONNODE_H_

#include <QXmlTree/QXmlTreeNode.h>

class QPositionNode : public QXmlTreeNode
{
	Q_OBJECT
public:
	QPositionNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent);
	virtual ~QPositionNode();

protected:
	// A normal position node don't has childs
	virtual bool isKnown(const QString&) const {return false;}

	virtual QXmlTreeNode* createChild(const QDomElement& childNode, int row) {return 0;}

	bool eventFilter(QObject* obj, QEvent* event);

private:
	void savePosition( float x, float y, float z );
	void setPosition( float x, float y, float z );

	bool			m_dynamicProperty;

	unsigned int	m_entityID;
};
#endif