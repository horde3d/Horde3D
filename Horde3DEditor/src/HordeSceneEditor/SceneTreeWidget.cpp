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
#include "SceneTreeWidget.h"
#include "SceneTreeModel.h"
#include "QReferenceNode.h"

#include <QMessageBox>
#include <QAction>

#include "SceneFile.h"

#include <Horde3D.h>


SceneTreeWidget::SceneTreeWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags)
{
	setupUi(this);
	connect(m_sceneTreeView, SIGNAL(rootLayer(const QString&)), this, SLOT(setRootLayer(const QString&)));
	connect(m_previousLayerButton, SIGNAL(clicked()), m_sceneTreeView, SLOT(popSceneGraph()));
	connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
	connect(m_sceneTreeView, SIGNAL(modelChanged(SceneTreeModel*)), this, SLOT(modelChanged(SceneTreeModel*)));
	connect(m_sceneTreeView, SIGNAL(nodePropertyChanged(QXmlTreeNode*)), this, SIGNAL(nodePropertyChanged(QXmlTreeNode*)));
	connect(m_extraTreeView, SIGNAL(nodePropertyChanged(QXmlTreeNode*)), this, SIGNAL(nodePropertyChanged(QXmlTreeNode*)));
	connect(m_sceneTreeView, SIGNAL(currentNodeChanged(QXmlTreeNode*)), this, SIGNAL(currentNodeChanged(QXmlTreeNode*)));
	connect(m_extraTreeView, SIGNAL(currentNodeChanged(QXmlTreeNode*)), this, SIGNAL(currentNodeChanged(QXmlTreeNode*)));
	connect(m_sceneTreeView, SIGNAL(nodeAboutToChange()), this, SIGNAL(nodeAboutToChange()));
	connect(m_extraTreeView, SIGNAL(nodeAboutToChange()), this, SIGNAL(nodeAboutToChange()));
}


SceneTreeWidget::~SceneTreeWidget()
{
}

void SceneTreeWidget::save()
{
	m_sceneTreeView->save();
}

bool SceneTreeWidget::setScene(SceneFile* sceneFile)
{
	if( sceneFile )
	{
		if (m_sceneTreeView->loadSceneGraph(sceneFile->sceneGraphFile()))
		{
			m_extraTreeView->loadExtras(sceneFile);
			tabChanged(m_tabWidget->currentIndex());
			return true;
		}
		else return false;		
	}
	else
	{
		m_extraTreeView->closeTree();
		m_sceneTreeView->closeTree();
		return true;
	}	
}

void SceneTreeWidget::tabChanged(int index)
{
	// Scene Tree View
	switch(index)
	{
	case 0:
		// Disable all actions in extra tree view
		for(int i=0; i<m_extraTreeView->actions().size(); ++i)
			m_extraTreeView->actions().at(i)->setEnabled(false);		
		// Enable all actions in scene tree view, additional updates regarding the state
		// will be made in HordeSceneEditor::currentNodeChanged
		for(int i=0; i<m_sceneTreeView->actions().size(); ++i)
			m_sceneTreeView->actions().at(i)->setEnabled(true);		
		m_sceneTreeView->currentNodeChanged(m_sceneTreeView->currentIndex(), QModelIndex());
		if (m_sceneTreeView->currentUndoStack())
			emit undoStackActivated(m_sceneTreeView->currentUndoStack());
		break;
	case 1:
		// Disable all actions in scene tree view
		for(int i=0; i<m_sceneTreeView->actions().size(); ++i)
			m_sceneTreeView->actions().at(i)->setEnabled(false);
		// Enable all actions in scene tree view, additional updates regarding the state
		// will be made in HordeSceneEditor::currentNodeChanged
		for(int i=0; i<m_extraTreeView->actions().size(); ++i)
			m_extraTreeView->actions().at(i)->setEnabled(true);		
		m_extraTreeView->currentNodeChanged(m_extraTreeView->currentIndex(), QModelIndex());
		if (m_extraTreeView->currentUndoStack())
			emit undoStackActivated(m_extraTreeView->currentUndoStack());
		break;
	}
}

void SceneTreeWidget::removeCurrentNode()
{
	switch(m_tabWidget->currentIndex())
	{
	case 0:
		m_sceneTreeView->removeCurrentNode();
		break;
	case 1:
		m_extraTreeView->removeCurrentNode();
		break;
	}
}

void SceneTreeWidget::copyCurrentNode()
{
	switch(m_tabWidget->currentIndex())
	{
	case 0:
		m_sceneTreeView->copyCurrentNode();
		break;
	case 1:
		m_extraTreeView->copyCurrentNode();
		break;
	}
}

void SceneTreeWidget::cutCurrentNode()
{
	switch(m_tabWidget->currentIndex())
	{
	case 0:
		m_sceneTreeView->copyCurrentNode();
		m_sceneTreeView->removeCurrentNode(false);
		break;
	case 1:
		m_extraTreeView->copyCurrentNode();
		m_extraTreeView->removeCurrentNode(false);
		break;
	}
}

void SceneTreeWidget::pasteNode()
{
	switch(m_tabWidget->currentIndex())
	{
	case 0:
		m_sceneTreeView->pasteNode();
		break;
	case 1:
		m_extraTreeView->pasteNode();
		break;
	}
}

void SceneTreeWidget::selectNode( int H3DNode )
{
	if( m_sceneTreeView->selectNode( H3DNode ) )
		m_tabWidget->setCurrentWidget( m_sceneWidget );
	else if( m_extraTreeView->selectNode( H3DNode ) )
		m_tabWidget->setCurrentWidget( m_extraWidget );
	else if( H3DNode != 0 )
		QMessageBox::information(
		this, 
		tr("Error"), 
		tr("The scenegraph file for the node '%1' (ID: %2) couldn't be found").arg(h3dGetNodeParamStr(H3DNode, H3DNodeParams::NameStr)).arg(H3DNode));
}

void SceneTreeWidget::setRootLayer(const QString& layer)
{		
	m_previousLayerButton->setEnabled(!layer.isNull());
	if (layer.isNull())
		m_previousLayerButton->setToolTip(tr("The current view shows already the root layer"));
	else
		m_previousLayerButton->setToolTip(tr("Jump back to %1").arg(layer));
}

void SceneTreeWidget::modelChanged(SceneTreeModel* model)
{
	// Activate the current undostack
	emit undoStackActivated(model->undoStack());
	// Update label of currently manipulated scenefile
	m_sceneGraphLabel->setText(QFileInfo(model->parentNode()->property("Scene_Graph_File").toString()).fileName());
}
