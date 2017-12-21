// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2012 Volker Vogelhuber
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
#include "QPipelineDrawQuadNode.h"

#include "QPipelineNode.h"

QPipelineDrawQuadNode::QPipelineDrawQuadNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : 
QXmlTreeNode(xmlNode, row, model, parent)
{
	setProperty("Type", QPipelineNode::PIPELINEDRAWQUADNODE);
}


QPipelineDrawQuadNode::~QPipelineDrawQuadNode()
{
}

QString QPipelineDrawQuadNode::material() const
{
	return m_xmlNode.attribute( "material" );
}

QString QPipelineDrawQuadNode::context() const
{
	return m_xmlNode.attribute( "context" );
}

QString QPipelineDrawQuadNode::description() const
{
	return context() + " | " + material();
}