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
#include "QPipelineDoForwardLightLoopNode.h"

#include "QPipelineNode.h"

QPipelineDoForwardLightLoopNode::QPipelineDoForwardLightLoopNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : 
QXmlTreeNode(xmlNode, row, model, parent)
{
	setProperty("Type", QPipelineNode::PIPELINEDOFORWARDLIGHTLOOPNODE);
}


QPipelineDoForwardLightLoopNode::~QPipelineDoForwardLightLoopNode()
{
}

QString QPipelineDoForwardLightLoopNode::context() const
{
	return m_xmlNode.attribute( "context" );
}

QString QPipelineDoForwardLightLoopNode::classes() const
{
	return m_xmlNode.attribute( "class" );
}

QString QPipelineDoForwardLightLoopNode::order() const
{
	return m_xmlNode.attribute( "order" );
}

bool QPipelineDoForwardLightLoopNode::noShadow() const
{
	return m_xmlNode.attribute("noShadow").compare( "true", Qt::CaseInsensitive ) == 0 || m_xmlNode.attribute("noShadow").compare( "1", Qt::CaseInsensitive ) == 0;
}

QString QPipelineDoForwardLightLoopNode::description() const
{
	QString retVal = context();			
	if( classes().length() )
		retVal += " (" + classes() + ") ";
	retVal += order();
	if( noShadow() )
		retVal += " NO SHADOW";
	return retVal;	
}
