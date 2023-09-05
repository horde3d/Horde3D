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

#ifndef SCENETREEWIDGET_H_
#define SCENETREEWIDGET_H_

#include "ui_SceneTreeWidget.h"

class QUndoStack;
class SceneTreeModel;

/**
 * Manages the scene tree view and the navigation between scene layers (TODO)
 */
class SceneTreeWidget : public QWidget, protected Ui_SceneTreeWidget
{
	friend class HordeSceneEditor;

	Q_OBJECT
public:
	SceneTreeWidget(QWidget* parent = 0, Qt::WindowFlags flags = (Qt::WindowFlags) 0);
	virtual ~SceneTreeWidget();

	void save();

	/**
	 * Loads the scene file
	 * @param sceneFile pointer to the currently loaded scene or 0 if you want to close the current scene
	 * @return bool true if the scene was loaded successfully
	 */
	bool setScene(SceneFile* sceneFile);

public slots:
	/**
	 * Removes the currently selected node from the active tree
	 */
	void removeCurrentNode();

	/**
	 * Copies the currently selected node from the active tree to the clipboard
	 */
	void copyCurrentNode();

	/**
	 * Copies the currently selected node from the active tree to the clipboard
	 * and removes it from the tree
	 */
	void cutCurrentNode();

	/**
	 * Copies the currently selected node from the active tree to the clipboard
	 */
	void pasteNode();

	/**
	 */
	void selectNode( int H3DNode );

signals:
	void undoStackActivated(QUndoStack* stack);

	void currentNodeChanged(QXmlTreeNode*);

	void nodePropertyChanged(QXmlTreeNode*);

	void nodeAboutToChange();

private slots:
	void tabChanged(int index);
	void setRootLayer(const QString& layer);
	void modelChanged(SceneTreeModel* model);

};
#endif

