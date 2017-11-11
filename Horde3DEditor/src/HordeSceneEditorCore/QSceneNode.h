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

#ifndef QSCENENODE_H_
#define QSCENENODE_H_

#include <QXmlTree/QXmlTreeNode.h>

#include "CustomTypes.h"

#include <QtCore/QStringList>

class SceneTreeModel;

/**
 * QSceneNode is the parent node of every Horde3D node in the scene graph
 * It will create ReferenceNodes, CameraNodes, LightNodes and GroupNodes
 * from their Xml representation in the scene graph. Other Horde3D nodes like
 * Mesh, Model and Joint are represented by a TreeItem but not by a Horde3D representation.
 * It is also used for extra nodes although these nodes
 * are not used for the SceneTreeModel but for the ExtraTreeModel. 
 */
class QSceneNode : public QXmlTreeNode
{
	Q_OBJECT
	Q_CLASSINFO("QSceneNode", "Basic");

	Q_PROPERTY(int ID READ hordeId DESIGNABLE true USER true)
	Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)
	Q_PROPERTY(bool Render READ enabled WRITE setEnabled DESIGNABLE true USER true)
	Q_PROPERTY(QVec3f Position READ position WRITE setPosition DESIGNABLE true USER true)
	Q_CLASSINFO("Position", "decimalsX=5;decimalsY=5;decimalsZ=5;");
	Q_PROPERTY(QVec3f Rotation READ rotation WRITE setRotation DESIGNABLE true USER true)
	Q_CLASSINFO("Rotation", "decimalsX=5;decimalsY=5;decimalsZ=5;");
	Q_PROPERTY(QVec3f Scale READ scale WRITE setScale DESIGNABLE true USER true)
	Q_CLASSINFO("Scale", "minimumX=0;minimumY=0;minimumZ=0;decimalsX=5;decimalsY=5;decimalsZ=5;");
	Q_PROPERTY(QVec3f AABB_Min READ aabbMin DESIGNABLE true USER true)
	Q_PROPERTY(QVec3f AABB_Max READ aabbMax DESIGNABLE true USER true)

	Q_PROPERTY(QMatrix4f __AbsoluteTransformation READ absTrans)
	Q_PROPERTY(QMatrix4f __RelativeTransformation READ relTrans WRITE setRelTrans)

public:
	
	/**
	 * \brief Constructor
	 *
	 * @param xmlNode the xml representation for this node
	 * @param row the row of the new QSceneNode within the QTreeView
	 * @param type the type of the newly created node
	 * @param model the model the new node will be part of
	 * @param parentNode if set this QSceneNode indicates the parent of the newly created node
	 */
	QSceneNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);
	/// Destructor
	virtual ~QSceneNode();

	/// The internal scene graph id in Horde3d
	unsigned int hordeId() const {return m_hordeID;}

	/**
	 * Node name attribute value
	 * @return name attribute of the current node
	 */
	QString name() const {return m_xmlNode.attribute("name");}
	void setName(const QString& name);

	/**
	 * Removes the ith child from the tree but don't deletes it
	 * @param i the index of the child to be taken
	 * @return QSceneNode* pointer to the item that will be removed from the child list
	 */
	QSceneNode* takeChild(int i);

	/**
	 * Appends an existing child to the childlist
	 */
	void appendChild(QSceneNode* child);

	QDomElement attachment() {return m_xmlNode.firstChildElement("Attachment");}

	/// every node can have it's own transformation
	void getTransformation(float& x, float& y, float& z, float& rx, float& ry, float& rz, float& sx, float& sy, float& sz);

	QVec3f position() const;
	void setPosition(const QVec3f& position);

	QVec3f rotation() const;
	void setRotation(const QVec3f& rotation);

	QVec3f scale() const;
	void setScale(const QVec3f& scale);

	QVec3f aabbMin() const;
	QVec3f aabbMax() const;


	QMatrix4f absTrans() const;

	QMatrix4f relTrans() const;
	/**
	 * \brief  Updates the Horde3D transformation without affecting the XML data
	 * 
	 * If there should be dynamic changes that should not be stored to the XML file you can
	 * set the RelativeTransformation property using this method.
	 * @param relTrans matrix containing the transformation of this node relative to it's parent
	 */ 
	void setRelTrans( const QMatrix4f& relTrans );

	bool enabled() const;
	void setEnabled(bool enabled);

	/**
	 * Can be reimplemented for custom behavior when a scene node gets activated (e.g. double clicked)
	 */
	virtual void activate() {}

	/**
	 * Provides a tooltip for the tree view
	 * Can be reimplemented in sub nodes
	 */
        virtual QString toolTip( int /*column*/ = -1) { return QString(); }

	/**
	 * Returns the text displayed by a tree view for the given column
	 * @return text for column in tree model
	 */
	virtual QVariant displayText( int column );

protected:	
	QXmlTreeNode* createChild(const QDomElement& childNode, int row);

	unsigned int				m_hordeID;
	bool						m_active;

	enum UndoIDs {
		TransformationID = 1, 
		FrustumID, 
		KeepAspectID,
		PipelineID,
		EmitterMaterialID, 
		EmitterMaxCountID, 
		EmitterRespawnCountID, 
		EmitterForceID, 
		EmitterEmissionRateID, 
		EmitterSpreadAngleID,
		EmitterDelayID,
		GroupMinDistID,
		GroupMaxDistID,
		LightShadowMapBiasID,
		LightFoVID,
		LightColorID,
		LightShadowMapCountID,
		LightRadiusID,
		LightMaterialID,
		LightShadowMapLambdaID,
		LightIntensityID,
		MeshMaterialID,
		LodLevelID,
		SoftwareSkinningID,
		LodDistID,
		TerrainMaterialID,
		TerrainHeightMapID,
		TerrainBlockSizeID,
		TerrainMeshQualityID,
		TerrainSkirtHeightID,
		OrthoProjectionID,
		OcclusionCullingID
	};
};
#endif


