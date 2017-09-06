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
#include "QPipelineClearTargetNode.h"

#include "QPipelineNode.h"

QPipelineClearTargetNode::QPipelineClearTargetNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : 
QXmlTreeNode(xmlNode, row, model, parent)
{
	setProperty("Type", QPipelineNode::PIPELINECLEARTARGETNODE);
}


QPipelineClearTargetNode::~QPipelineClearTargetNode()
{
}

bool QPipelineClearTargetNode::clearDepth() const
{
	return m_xmlNode.attribute("depthBuf").compare( "true", Qt::CaseInsensitive ) == 0 || m_xmlNode.attribute("depthBuf").compare( "1", Qt::CaseInsensitive ) == 0;
}

void QPipelineClearTargetNode::setClearDepth( bool clear )
{
}

bool QPipelineClearTargetNode::clearColBuf( int buffer ) const
{
	QString bufferName = QString("colBuf%1").arg(buffer);
	return m_xmlNode.attribute(bufferName).compare( "true", Qt::CaseInsensitive ) == 0 || m_xmlNode.attribute(bufferName).compare( "1", Qt::CaseInsensitive ) == 0;
}

void QPipelineClearTargetNode::setClearColorBuffer( int buffer, bool clear )
{
}

QQuatF QPipelineClearTargetNode::clearColor() const
{
	QQuatF retVal;
	retVal.w = m_xmlNode.attribute( "col_A", "0" ).toFloat();
	retVal.x = m_xmlNode.attribute( "col_R", "0" ).toFloat();
	retVal.y = m_xmlNode.attribute( "col_G", "0" ).toFloat();
	retVal.z = m_xmlNode.attribute( "col_B", "0" ).toFloat();
	return retVal;
}

QString QPipelineClearTargetNode::description() const
{
	QString targets;
	if( clearDepth() )
		targets += "Depth";
	if( clearColBuf(0) )
	{
		if( targets.length() ) targets += " | ";
		targets += "ColBuf0";
	}

	if( clearColBuf(1) )
	{
		if( targets.length() ) targets += " | ";
		targets += "ColBuf1";
	}
	if( clearColBuf(2) )
	{
		if( targets.length() ) targets += " | ";
		targets += "ColBuf2";
	}
	if( clearColBuf(3) )
	{
		if( targets.length() ) targets += " | ";
		targets += "ColBuf3";
	}
	if( targets.isEmpty() )
		targets += tr("None");
	QQuatF color = clearColor();
	targets += QString(" | R: %1 G: %2 B: %3 A: %4").arg(color.x).arg(color.y).arg(color.z).arg(color.w);
	return targets;
}

