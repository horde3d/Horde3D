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

#ifndef EXTRATREEVIEW_H_
#define EXTRATREEVIEW_H_

#include <QXmlTree/QXmlTreeView.h>
#include <QtXml/qdom.h>
class QAction;
class QXmlTreeNode;
class SceneFile;
class QUndoStack;

/**
 * \brief Displays the extras tree for extra nodes by using the ExtraTreeModel
 * 
 * Things that should not be stored within the scene graph file (*.scene.xml) can be declared as extra node
 * within the scene file (.scn). For example an ExtraPlugIn can implement the support of Horde3D overlays
 * by using extra nodes.
 */
class ExtraTreeView : public QXmlTreeView
{
	friend class SceneTreeWidget;

	Q_OBJECT
public:
	/**
	 * Constructor
	 * @param parent optional parent widget
	 */
	ExtraTreeView(QWidget* parent = 0);

	/// Destructor
	virtual ~ExtraTreeView();

	/**
	 * Loads the extras from the given xml structure into the tree model 	
	 * @param sceneFile the scene file instance that contains the extras
	 * @return bool true if loading was successfull otherwise false
	 */
	bool loadExtras(SceneFile* sceneFile);

	/**
	 * \brief Will be called when closing a scene file
	 * 
	 * Deletes the current model and emits a currentNodeChanged(0) 
	 * as well as a undoStackRemoved() signal
	 */
	void closeTree();

	/**
	 * Returns the current undo stack if any available
	 */
	QUndoStack* currentUndoStack();

signals:
	/**
	 * Signal will be emitted if a property of the node has changed
	 */
	void nodePropertyChanged(QXmlTreeNode* node);

	/**
	 * Signal will be emitted just before the node selected will become active
	 */
	void nodeAboutToChange();

public slots:
		/**
	 * Adds a new Extra Node to the document
	 */
	void addNode();

protected slots:	
	virtual void currentNodeChanged(const QModelIndex& current, const QModelIndex& previous);

protected:
	virtual void contextMenuEvent( QContextMenuEvent* event );

	virtual bool eventFilter( QObject* obj, QEvent* event );

	bool selectNode( int H3DNode );

private slots:
	void nodeDataChanged(const QModelIndex& index);

private:	
	bool					m_contextCall;

};
#endif
