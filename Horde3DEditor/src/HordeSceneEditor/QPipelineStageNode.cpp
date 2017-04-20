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

#include "QPipelineStageNode.h"
#include "QPipelineNode.h"
#include "QPipelineSwitchTargetNode.h"
#include "QPipelineBindBufferNode.h"
#include "QPipelineUnbindBuffersNode.h"
#include "QPipelineDoForwardLightLoopNode.h"
#include "QPipelineDoDeferredLightLoopNode.h"
#include "QPipelineDrawGeometryNode.h"
#include "QPipelineDrawQuadNode.h"
#include "QPipelineDrawOverlaysNode.h"
#include "QPipelineClearTargetNode.h"
#include "QPipelineSetUniformNode.h"

#include <QXmlTree/QXmlTreeModel.h>
#include <Horde3D.h>


QPipelineStageNode::QPipelineStageNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : 
QXmlTreeNode(xmlNode, row, model, parent), m_active(true)
{
	setProperty("Type", QPipelineNode::PIPELINESTAGENODE);
}


QPipelineStageNode::~QPipelineStageNode()
{
}

bool QPipelineStageNode::enabled() const
{
	return m_active;
}

void QPipelineStageNode::setEnabled(bool enabled)
{
	m_active = enabled;
	QPipelineNode* root = qobject_cast<QPipelineNode*>(m_model->rootNode());	
	h3dSetResParamI(root->hordeId(), H3DPipeRes::StageElem, m_row, H3DPipeRes::StageActivationI, enabled ? 1 : 0);
}

QXmlTreeNode* QPipelineStageNode::createChild(const QDomElement& childNode, int row)
{
	if( childNode.tagName() == "ClearTarget" )
		return new QPipelineClearTargetNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "DrawGeometry" )
		return new QPipelineDrawGeometryNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "BindBuffer" )
		return new QPipelineBindBufferNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "DrawQuad" )
		return new QPipelineDrawQuadNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "DrawOverlays" )
		return new QPipelineDrawOverlaysNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "UnbindBuffers" )
		return new QPipelineUnbindBuffersNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "SwitchTarget" )
		return new QPipelineSwitchTargetNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "SetUniform" )
		return new QPipelineSetUniformNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "DoForwardLightLoop" )
		return new QPipelineDoForwardLightLoopNode(childNode, row, m_model, this);
	else if( childNode.tagName() == "DoDeferredLightLoop" )
		return new QPipelineDoDeferredLightLoopNode(childNode, row, m_model, this);
	return 0;
}