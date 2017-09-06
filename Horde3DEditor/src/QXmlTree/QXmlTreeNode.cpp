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

#include "QXmlTreeNode.h"


QXmlTreeNode::QXmlTreeNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent /*= 0*/) : QObject(parent), 
 m_model(model), m_xmlNode(xmlNode), m_row(row)
{
}


QXmlTreeNode::~QXmlTreeNode()
{
	qDeleteAll(m_childItems);
}

QXmlTreeNode* QXmlTreeNode::child(int i)
{
	if (i < 0)	return 0;
	if (i < m_childItems.size()) return m_childItems[i];
	if (i >= m_xmlNode.childNodes().count()) return 0;
	
	QDomElement childNode = m_xmlNode.firstChildElement();
	int j = 0, known = 0;
	while(!childNode.isNull() && j++ <= i)
	{	
		// Do we handle this node?
		if (isKnown(childNode.tagName())) 
			++known; // Increase number of known nodes
		else // Skip node
			--j;

		// If node hasn't be constructed before...	
		if (j > m_childItems.size())
		{
			//if ( known > m_childItems.size())				
			//{				
				QXmlTreeNode* childItem = createChild(childNode, j-1);			
				if (childItem)
				{
					m_childItems.push_back(childItem);
					 // Stop when we reached the requested node
					if (j - 1 == i)	return childItem;			
				}
				else 					
				//{
				//	--known;
					--j;
			//	}
			//}
			// ignore this node since we added it before or don't know it
			//else --j;
		}
		childNode = childNode.nextSiblingElement();		
	}	
	return 0;
}


unsigned int QXmlTreeNode::childs()
{
	int n = 0;
	while (child(n))
		++n;
	return n;
}

QXmlTreeNode* QXmlTreeNode::child(const QDomNode &node)
{
	unsigned int childCount = childs();
	for (unsigned int i = 0; i < childCount; ++i)
	{
		QXmlTreeNode* childNode(m_childItems[i]);		
		if (childNode->xmlNode() == node)
			return childNode;
		// Depth Search
		childNode = childNode->child(node);
		if (childNode)
			return childNode;
	}
	return 0;
}

void QXmlTreeNode::removeChild(int i)
{
	// Is index valid?
	if (child(i))
	{	
		QXmlTreeNode* item = m_childItems.takeAt(i);		
		m_xmlNode.removeChild(item->xmlNode());	
		delete item;
		// Ensure valid row index of items
		for (int j=i; j<m_childItems.size(); ++j)
			m_childItems.at(j)->m_row = j;
	}	
}

