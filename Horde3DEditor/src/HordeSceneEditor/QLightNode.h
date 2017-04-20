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

#ifndef QLIGHTNODE_H_
#define QLIGHTNODE_H_

#include "QSceneNode.h"
#include <QtGui/QColor>

/**
 * A light node item for the SceneTreeModel
 */
class QLightNode : public QSceneNode
{
	Q_OBJECT
	Q_CLASSINFO("QLightNode", "Light specific");

	Q_PROPERTY(float Field_Of_View READ fov WRITE setFov DESIGNABLE true USER true)
	Q_CLASSINFO("Field_Of_View", "minimum=0.0;maximum=360.0;");
	Q_PROPERTY(float Radius READ radius WRITE setRadius DESIGNABLE true USER true)	
	Q_PROPERTY(QColor Color READ color WRITE setColor DESIGNABLE true USER true)
	Q_PROPERTY(int Shadow_Map_Count READ shadowMapCount WRITE setShadowMapCount DESIGNABLE true USER true)
	Q_CLASSINFO("Shadow_Map_Count", "minimum=0;maximum=4;");
	Q_PROPERTY(float Shadow_Map_Bias READ shadowMapBias WRITE setShadowMapBias DESIGNABLE true USER true)
	Q_CLASSINFO("Shadow_Map_Bias", "minimum=0.0;maximum=10.0;decimals=4;singleStep=0.0001;");
	Q_PROPERTY(float Shadow_Map_Lambda READ lambda WRITE setLambda DESIGNABLE true USER true)
	Q_CLASSINFO("Shadow_Map_Lambda", "minimum=0.0;maximum=1.0;decimals=4;singleStep=0.0001;");
	Q_PROPERTY(Material Material READ material WRITE setMaterial DESIGNABLE true USER true)
	Q_PROPERTY(QString ShadowContext READ shadowContext WRITE setShadowContext DESIGNABLE true USER true)
	Q_PROPERTY(QString LightContext READ lightContext WRITE setLightContext DESIGNABLE true USER true)

public:
	static QSceneNode* loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent);

	QLightNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);
	virtual ~QLightNode();

	void addRepresentation();

	void activate();

	float fov() const;
	void setFov(float value);

	float shadowMapBias() const;
	void setShadowMapBias(float value);

	QString shadowContext() const;
	void setShadowContext(const QString& value);

	QString lightContext() const;
	void setLightContext(const QString& value);

	QColor color() const;
	void setColor(QColor value);

	int shadowMapCount() const;
	void setShadowMapCount(int value);

	float lambda() const;
	void setLambda(float value);

	float radius() const;
	void setRadius(float value);

	Material material() const;
	void setMaterial(const Material& material);
	
        QString toolTip( int /*column*/ )  { return tr("Double click to move the camera to the lights position"); }

protected:
	
	//void addDebugRepresentation();
	//void removeDebugRepresentation();

private:	
	unsigned int m_resourceID;
	unsigned int m_debugID;
	unsigned int m_debugResourceMat;
	unsigned int m_debugResourceGeo;
	
};

#endif
