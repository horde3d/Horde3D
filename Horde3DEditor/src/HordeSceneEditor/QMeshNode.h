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

#ifndef QMESHNODE_H_
#define QMESHNODE_H_

#include "QSceneNode.h"

class QMeshNode : public QSceneNode
{
	Q_OBJECT
	Q_CLASSINFO("QMeshNode", "Mesh specific");

	Q_PROPERTY(Material Material READ material WRITE setMaterial DESIGNABLE true USER true)
	Q_PROPERTY(int BatchStart READ batchStart DESIGNABLE true USER true)
	Q_PROPERTY(int BatchCount READ batchCount DESIGNABLE true USER true)
	Q_PROPERTY(int VerticeRStart READ vertRStart DESIGNABLE true USER true)
	Q_PROPERTY(int VerticeREnd READ vertREnd DESIGNABLE true USER true)
	Q_PROPERTY(int Lod_Level READ lodLevel WRITE setLodLevel DESIGNABLE true USER true)


public:
	static QSceneNode* loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent);

	QMeshNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);
	virtual ~QMeshNode();

	Material material() const;
	void setMaterial(const Material& material);

	int batchStart() const;

	int batchCount() const;

	int vertRStart() const;

	int vertREnd() const;

	int lodLevel() const;
	void setLodLevel( int lodLevel );

	void activate();

signals:
	void materialChanged( const QString& materialFile );

private:
	void addRepresentation();

	unsigned int	m_resourceID;
};
#endif
