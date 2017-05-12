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
#include "QPipelineNode.h"

#include "QPipelineSetupNode.h"
#include "QPipelineCommandQueueNode.h"

QPipelineNode::QPipelineNode(unsigned int pipelineID, const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : 
QXmlTreeNode(xmlNode, row, model, parent), m_noSetupNode(false), m_pipelineID(pipelineID)
{
	m_knownNodeNames << "Setup" << "CommandQueue";
	if (m_xmlNode.firstChildElement("Setup").isNull())
	{
		m_xmlNode.insertBefore( m_xmlNode.ownerDocument().createElement("Setup"), QDomElement());
		m_noSetupNode = true;
	}
	setProperty("Type", PIPELINENODE);
}


QPipelineNode::~QPipelineNode()
{
}

QXmlTreeNode* QPipelineNode::createChild(const QDomElement& childNode, int row)
{
	QXmlTreeNode* childItem = 0;
	if (childNode.tagName().compare("Setup")==0)
		childItem = new QPipelineSetupNode(childNode, row, m_model, this);
	if (childNode.tagName().compare("CommandQueue")==0)
		childItem = new QPipelineCommandQueueNode(childNode, row, m_model, this);
	return childItem;
}


