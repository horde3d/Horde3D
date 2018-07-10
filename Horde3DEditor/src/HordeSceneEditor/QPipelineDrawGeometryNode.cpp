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
#include "QPipelineDrawGeometryNode.h"

#include "QPipelineNode.h"

QPipelineDrawGeometryNode::QPipelineDrawGeometryNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : 
QXmlTreeNode(xmlNode, row, model, parent)
{
	setProperty("Type", QPipelineNode::PIPELINEDRAWGEOMETRYNODE);
}


QPipelineDrawGeometryNode::~QPipelineDrawGeometryNode()
{
}

QString QPipelineDrawGeometryNode::context() const
{
	return m_xmlNode.attribute( "context" );
}

QString QPipelineDrawGeometryNode::classes() const
{
	return m_xmlNode.attribute( "class" );
}

QString QPipelineDrawGeometryNode::order() const
{
	return m_xmlNode.attribute( "order" );
}


QString QPipelineDrawGeometryNode::description() const
{
	QString retVal = context();			
	if( classes().length() )
		retVal += " (" + classes() + ") ";
	retVal += order();
	return retVal;	
}

