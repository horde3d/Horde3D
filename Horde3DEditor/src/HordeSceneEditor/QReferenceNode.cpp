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

#include "QReferenceNode.h"
#include "SceneTreeModel.h"

#include "HordeSceneEditor.h"
#include "GLWidget.h"
#include "AttachmentPlugIn.h"
#include "PlugInManager.h"

#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QMessageBox>

#include "SceneFile.h"

#include <Horde3D.h>
#include <Horde3DUtils.h>

QSceneNode* QReferenceNode::loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent)
{
	return new QReferenceNode(xmlNode, row, model, parent);
}


QReferenceNode::QReferenceNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode) : 
	QSceneNode(xmlNode, row, model, parentNode), m_treeModel(0)
{
	setObjectName("Reference");
	m_hordeID = parentNode ? parentNode->hordeId() : H3DRootNode;
	QReferenceNode::addRepresentation();
}


QReferenceNode::~QReferenceNode()
{	
	// Reset horde id before deleting the tree model, because, tree model destructor already removes it for us
	// so we set it to 0 to avoid doublicate removement in QSceneNode destructor
	if( m_treeModel && m_treeModel->rootNode() )
		m_hordeID = 0;

	delete m_treeModel;
	QDomElement attachment = m_xmlNode.firstChildElement("Attachment");	
	AttachmentPlugIn* plugIn = HordeSceneEditor::instance()->pluginManager()->attachmentPlugIn();
	if( !attachment.isNull() && plugIn )
		plugIn->destroyNodeAttachment(this);
	// Avoid removing of attachment in QSceneNode Destructor
	m_model = 0;
}

void QReferenceNode::save()
{
	// Save this reference node
	QFile sceneGraphFile(m_fileName);
	if (!sceneGraphFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
		QMessageBox::warning(0, tr("Error"), tr("Error opening %1 for writing: %2").arg(m_fileName).arg(sceneGraphFile.errorString()));
	else
	{
		QTextStream stream(&sceneGraphFile);
		m_sceneGraphFile.save(stream, 4);		
	}
	sceneGraphFile.close();
	if (m_treeModel)
	{
		// ... and all sub reference nodes
		QModelIndexList referenceNodes = m_treeModel->match(m_treeModel->index(0,1), Qt::DisplayRole, QString("Reference"), -1, Qt::MatchExactly | Qt::MatchRecursive);
		for (int i=0; i<referenceNodes.size(); ++i)
		{
			QReferenceNode* reference = static_cast<QReferenceNode*>(referenceNodes.at(i).internalPointer());
			reference->save();
		}
	}
}

void QReferenceNode::addRepresentation()
{		
	// Get absolute filename for referenced xml file
	m_fileName = QFileInfo( QDir::current(), m_xmlNode.attribute("sceneGraph") ).filePath();

	// Try opening referenced file
	QFile sceneGraphFile(m_fileName);
	if (!sceneGraphFile.open(QIODevice::ReadOnly))
	{		
		HordeSceneEditor::instance()->updateLog(new QListWidgetItem(tr("Error when opening file %1!\n%2").arg(m_fileName).arg(sceneGraphFile.errorString()), 0, 2));
		m_hordeID = 0;
		return;
	}
	QString errorMsg;
	int line, column;
	// parse xml
	if (!m_sceneGraphFile.setContent(&sceneGraphFile, &errorMsg, &line, &column))
	{
		HordeSceneEditor::instance()->updateLog(new QListWidgetItem(tr("Error in XML file %1, line %2 column %3:\n%4").arg(m_fileName).arg(line).arg(column).arg(errorMsg), 0, 2));		
		m_hordeID = 0;
		return;
	}
	sceneGraphFile.close();
	
	// Create new model for xml file
	m_treeModel = new SceneTreeModel(HordeSceneEditor::instance()->pluginManager(), m_sceneGraphFile.documentElement(), this);
	if( m_treeModel->rootNode() )
	{
		m_hordeID = static_cast<QSceneNode*>(m_treeModel->rootNode())->hordeId();
		setProperty("HordeType", m_treeModel->rootNode()->property("HordeType"));
		// Check for attachments
		QDomElement attachment = m_xmlNode.firstChildElement("Attachment");	
		AttachmentPlugIn* plugIn = HordeSceneEditor::instance()->pluginManager()->attachmentPlugIn();
		if (!attachment.isNull() &&  plugIn != 0)
			plugIn->initNodeAttachment(this);
	}
	else
	{
		delete m_treeModel;
		m_treeModel = 0;
		m_hordeID = 0;
	}		
}

void QReferenceNode::activate()
{
	float minX, minY, minZ, maxX, maxY, maxZ;
	h3dGetNodeAABB(m_hordeID, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);

	unsigned int cameraID = HordeSceneEditor::instance()->glContext()->activeCam();
	float leftPlane = h3dGetNodeParamF(cameraID, H3DCamera::LeftPlaneF, 0);
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

QString QReferenceNode::toolTip(int /*column*/)
{
	 return tr("Double click to enter the associated scenegraph file '%1'\n"
		 "Hold Control key pressed while clicking to move the camera to\n"
		 "fit the bounding of the scene specified in the reference node!").arg(m_xmlNode.attribute("sceneGraph") );
}
