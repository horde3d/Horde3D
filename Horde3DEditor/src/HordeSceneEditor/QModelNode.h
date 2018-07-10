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

#ifndef QMODELNODE_H_
#define QMODELNODE_H_

#include "QSceneNode.h"

/**
 * A model node item for the SceneTreeModel
 */
class QModelNode : public QSceneNode
{
	Q_OBJECT
	Q_CLASSINFO("QModelNode", "Model specific");

	Q_PROPERTY(QString Geometry READ geometry DESIGNABLE true USER true)
	Q_PROPERTY(bool Software_Skinning READ softwareSkinning WRITE setSoftwareSkinning DESIGNABLE true USER true)
	Q_PROPERTY(float Lod_Distance_1 READ lodDist1 WRITE setLodDist1 DESIGNABLE true USER true)
	Q_PROPERTY(float Lod_Distance_2 READ lodDist2 WRITE setLodDist2 DESIGNABLE true USER true)
	Q_PROPERTY(float Lod_Distance_3 READ lodDist3 WRITE setLodDist3 DESIGNABLE true USER true)
	Q_PROPERTY(float Lod_Distance_4 READ lodDist4 WRITE setLodDist4 DESIGNABLE true USER true)

public:
	static QSceneNode* loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent);

	QModelNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);
	virtual ~QModelNode();

	QString geometry() const;

	bool softwareSkinning() const;
	void setSoftwareSkinning(bool softwareSkinning);

	float lodDist1() const;
	void setLodDist1( float lodDist1 );

	float lodDist2() const;
	void setLodDist2( float lodDist2 );

	float lodDist3() const;
	void setLodDist3( float lodDist3 );

	float lodDist4() const;
	void setLodDist4( float lodDist4 );

	void activate();

private:
	void addRepresentation();
	unsigned int	m_resourceID;

};
#endif
