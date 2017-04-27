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


#include "QModelNode.h"

#include "HordeSceneEditor.h"
#include "GLWidget.h"
#include "AttachmentPlugIn.h"
#include "SceneTreeModel.h"
#include "PlugInManager.h"

#include <QXmlTree/QXmlNodePropertyCommand.h>

#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <float.h>

#include "SceneFile.h"

#include <Horde3D.h>
#include <Horde3DUtils.h>

QSceneNode* QModelNode::loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent)
{
	return new QModelNode(xmlNode, row, model, parent);
}

QModelNode::QModelNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode) :
	QSceneNode(xmlNode, row, model, parentNode)
{
	setObjectName("Model");	
	addRepresentation();
}

QModelNode::~QModelNode()
{
	if (m_resourceID != 0)
		h3dRemoveResource(m_resourceID);
}

QString QModelNode::geometry() const
{
	return m_xmlNode.attribute("geometry");
}

bool QModelNode::softwareSkinning() const
{
	return m_xmlNode.attribute("softwareSkinning", "false").compare("true", Qt::CaseInsensitive) == 0 ||
		m_xmlNode.attribute("softwareSkinning", "0").compare("1", Qt::CaseInsensitive) == 0;
}

void QModelNode::setSoftwareSkinning(bool softwareSkinning)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("softwareSkinning", softwareSkinning);
		h3dSetNodeParamI(m_hordeID, H3DModel::SWSkinningI, softwareSkinning);
	}
	else if( softwareSkinning != QModelNode::softwareSkinning() )
		m_model->undoStack()->push(new QXmlNodePropertyCommand("Set Software Skinning", this, "Software_Skinning", QVariant::fromValue(softwareSkinning), SoftwareSkinningID));
}

float QModelNode::lodDist1() const
{
	return m_xmlNode.attribute("lodDist1", QString::number( FLT_MAX ) ).toFloat();
}

void QModelNode::setLodDist1( float lodDist1 ) 
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("lodDist1", lodDist1);
		h3dSetNodeParamI(m_hordeID, H3DModel::LodDist1F, lodDist1);
	}
	else if( lodDist1 != QModelNode::lodDist1() )
		m_model->undoStack()->push( new QXmlNodePropertyCommand("Set Lod Distance 1", this, "Lod_Distance_1", QVariant::fromValue(lodDist1), LodDistID) );
}

float QModelNode::lodDist2() const
{
	return m_xmlNode.attribute("lodDist2", QString::number( FLT_MAX ) ).toFloat();
}

void QModelNode::setLodDist2( float lodDist2 ) 
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("lodDist2", lodDist2);
		h3dSetNodeParamI(m_hordeID, H3DModel::LodDist2F, lodDist2);
	}
	else if( lodDist2 != QModelNode::lodDist2() )
		m_model->undoStack()->push( new QXmlNodePropertyCommand("Set Lod Distance 2", this, "Lod_Distance_2", QVariant::fromValue(lodDist2), LodDistID) );
}

float QModelNode::lodDist3() const
{
	return m_xmlNode.attribute("lodDist3", QString::number( FLT_MAX ) ).toFloat();
}

void QModelNode::setLodDist3( float lodDist3 ) 
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("lodDist3", lodDist3 );
		h3dSetNodeParamI(m_hordeID, H3DModel::LodDist3F, lodDist3);
	}
	else if( lodDist3 != QModelNode::lodDist3() )
		m_model->undoStack()->push( new QXmlNodePropertyCommand("Set Lod Distance 3", this, "Lod_Distance_3", QVariant::fromValue(lodDist3), LodDistID) );
}

float QModelNode::lodDist4() const
{
	return m_xmlNode.attribute("lodDist4", QString::number( FLT_MAX ) ).toFloat();
}

void QModelNode::setLodDist4( float lodDist4 ) 
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("lodDist4", lodDist4);
		h3dSetNodeParamI(m_hordeID, H3DModel::LodDist4F, lodDist4);
	}
	else if( lodDist4 != QModelNode::lodDist4() )
		m_model->undoStack()->push( new QXmlNodePropertyCommand("Set Lod Distance 4", this, "Lod_Distance_4", QVariant::fromValue(lodDist4), LodDistID) );
}

void QModelNode::addRepresentation()
{
	m_resourceID = h3dAddResource(H3DResTypes::Geometry, qPrintable(m_xmlNode.attribute("geometry")), 0);

	// Load resource immediately since a later call to loadResourceFromDisk results in a bad behaviour of the Horde3D engine
	QString resourceName = h3dutGetResourcePath(H3DResTypes::Geometry);
	if( !resourceName.isEmpty() && !resourceName.endsWith('/') && !resourceName.endsWith('\\'))
		resourceName += '/';
	resourceName += geometry();

	QFile file(QFileInfo(resourceName).absoluteFilePath());
	if (!file.open(QIODevice::ReadOnly))
		qWarning("Error opening resource %s (%s)", qPrintable(m_xmlNode.attribute("geometry")), qPrintable( QFileInfo(resourceName).absoluteFilePath() ));
	
	// Stupid return value, if false it can also be the case that the resource was loaded before instead of that their was an error
    if( !h3dIsResLoaded(m_resourceID) )
        h3dLoadResource(m_resourceID, file.readAll().constData(), file.size());
	file.close();

	QSceneNode* parentNode = static_cast<QSceneNode*>(parent());
	unsigned int rootID = parentNode ? parentNode->hordeId() : H3DRootNode;

	m_hordeID = h3dAddModelNode(rootID, qPrintable(m_xmlNode.attribute("name", "ATTENTION No Node Name")), m_resourceID);
	
	float x, y, z, rx, ry, rz, sx, sy, sz;
	getTransformation(x,y,z,rx,ry,rz,sx,sy,sz);
	h3dSetNodeTransform(m_hordeID, x, y, z, rx, ry, rz, sx, sy, sz);
	
	h3dSetNodeParamI(m_hordeID, H3DModel::SWSkinningI, softwareSkinning());
	h3dSetNodeParamF(m_hordeID, H3DModel::LodDist1F, 0, lodDist1());
	h3dSetNodeParamF(m_hordeID, H3DModel::LodDist2F, 0, lodDist2());
	h3dSetNodeParamF(m_hordeID, H3DModel::LodDist3F, 0, lodDist3());
	h3dSetNodeParamF(m_hordeID, H3DModel::LodDist4F, 0, lodDist4());

	// Attachment
	QDomElement attachment = m_xmlNode.firstChildElement("Attachment");	
	SceneTreeModel* model = static_cast<SceneTreeModel*>(m_model);
	AttachmentPlugIn* plugIn = model->nodeFactory()->attachmentPlugIn();
	if (!attachment.isNull() &&  plugIn != 0)
		plugIn->initNodeAttachment(this);
}

void QModelNode::activate()
{
	float minX, minY, minZ, maxX, maxY, maxZ;
	h3dGetNodeAABB(m_hordeID, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);

    unsigned int cameraID = static_cast<HordeSceneEditor*>(qApp->property("SceneEditorInstance").value<void*>())->glContext()->activeCam();
	float leftPlane = h3dGetNodeParamF(cameraID, H3DCamera::LeftPlaneF, 0 );
	float rightPlane = h3dGetNodeParamF(cameraID, H3DCamera::RightPlaneF, 0);
	float bottomPlane = h3dGetNodeParamF(cameraID, H3DCamera::BottomPlaneF, 0);
	float topPlane = h3dGetNodeParamF(cameraID, H3DCamera::TopPlaneF, 0);
	float nearPlane = h3dGetNodeParamF(cameraID, H3DCamera::NearPlaneF, 0);

	const float* camera = 0;
	H3DNode parentNode = h3dGetNodeParent(cameraID);
	h3dGetNodeTransMats(parentNode, 0, &camera); 
	if ( !camera ) return;
	
	/** 
	 * (maxX - minX)                           =   width of the bounding box 
	 * rightPlane / (rightPlane - leftPlane)   =   right fraction of the viewing frustum
	 */
	float offsetX = (maxX - minX) * rightPlane / (rightPlane - leftPlane);
	float offsetY = (maxY - minY) * topPlane / (topPlane - bottomPlane);
	QMatrix4f newCamTrans = QMatrix4f::TransMat(maxX - offsetX, maxY - offsetY, maxZ);
	newCamTrans.translate(0, 0, qMax(nearPlane * offsetX / rightPlane, nearPlane * offsetY / topPlane));		
	h3dSetNodeTransMat(cameraID, (QMatrix4f(camera).inverted() * newCamTrans).x);	
}


