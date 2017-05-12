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

#ifndef QPIPELINESTAGENODE_H_
#define QPIPELINESTAGENODE_H_

#include <QXmlTree/QXmlTreeNode.h>

class QPipelineStageNode : public QXmlTreeNode
{
	Q_OBJECT

	Q_PROPERTY(bool Enabled READ enabled WRITE setEnabled DESIGNABLE true USER true)

public:
	QPipelineStageNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent = 0);
	virtual ~QPipelineStageNode();

	bool enabled() const;
	void setEnabled(bool enabled);

protected:
	// QPipelineStageNode doesn't have childs
	QXmlTreeNode* createChild(const QDomElement& childNode, int row);

	bool m_active;

};
#endif
