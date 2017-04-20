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

#ifndef QTERRAINNODE_H_
#define QTERRAINNODE_H_

#include "QSceneNode.h"


/**
 * A terrain node item for the SceneTreeModel
 */
class QTerrainNode : public QSceneNode
{
	Q_OBJECT
	Q_CLASSINFO("QTerrainNode", "Terrain specific");

	Q_PROPERTY(Material Material READ material WRITE setMaterial DESIGNABLE true USER true)
	Q_PROPERTY(Texture HeightMap READ heightMap WRITE setHeightMap DESIGNABLE true USER true)
	Q_PROPERTY(int Blocksize READ blockSize WRITE setBlockSize DESIGNABLE true USER true)
	Q_CLASSINFO("Blocksize", "minimum=5;maximum=65;");

	Q_PROPERTY(float Mesh_Quality READ meshQuality WRITE setMeshQuality DESIGNABLE true USER true)
	Q_CLASSINFO("Mesh_Quality", "minimum=0.0001;maximum=1000.0;decimals=2;singleStep=10;");

	Q_PROPERTY(float Skirt_Height READ skirtHeight WRITE setSkirtHeight DESIGNABLE true USER true)
	Q_CLASSINFO("Skirt_Height", "minimum=0.0001;maximum=100.0;decimals=3;singleStep=0.01;");


public:
	static QDomElement createNode(QWidget* parent);
	static QSceneNode* loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);

	QTerrainNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);
	virtual ~QTerrainNode();

	Material material() const;
	void setMaterial(const Material& material);

	Texture heightMap() const;
	void setHeightMap(const Texture& heightMap);

	int blockSize() const;
	void setBlockSize(const int blockSize);

	float meshQuality() const;
	void setMeshQuality(const float quality);

	float skirtHeight() const;
	void setSkirtHeight(const float height);

private:
	void addRepresentation();
	unsigned int	m_heightMapID;
	unsigned int	m_materialID;

};
#endif
