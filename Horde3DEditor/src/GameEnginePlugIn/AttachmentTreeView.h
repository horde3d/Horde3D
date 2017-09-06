// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the GameEngine developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// The GameEngine is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The GameEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************


// ****************************************************************************************
//
// GameEngine Horde3D Editor Plugin of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#ifndef ATTACHMENTTREEVIEW_H_
#define ATTACHMENTTREEVIEW_H_

#include <QXmlTree/QXmlTreeView.h>

class AttachmentTreeView : public QXmlTreeView
{
	Q_OBJECT
public:
	AttachmentTreeView(QWidget* parent = 0);
	virtual ~AttachmentTreeView();

protected slots:
	void addPhysicsComponent();
	void addTTSComponent();
	void addKeyframeAnimComponent();
	void addDynamidComponent();
	void addCrowdParticleComponent();
	void addCrowdVisNodeComponent();
	void addSound3DComponent();

protected:
	virtual QAddXmlNodeUndoCommand* createAddUndoCommand(const QDomElement& node, const QDomElement& parent, QXmlTreeModel* model, const QString& text) const;

	virtual QRemoveXmlNodeUndoCommand* createRemoveUndoCommand(QXmlTreeNode* node, const QString& text) const;


};
#endif