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

#ifndef SCENETREEVIEW_H_
#define SCENETREEVIEW_H_

#include <QXmlTree/QXmlTreeView.h>
#include <QtXml/qdom.h>

class QAction;
class QXmlTreeNode;
class SceneTreeModel;
class QUndoStack;
class QReferenceNode;

/**
 * Displays the scene tree for Horde3D by using the SceneTreeModel
 */
class SceneTreeView : public QXmlTreeView
{
	friend class SceneTreeWidget;

	Q_OBJECT
public:
	SceneTreeView(QWidget* parent = 0);
	virtual ~SceneTreeView();

	/**
	 * Loads the scene from the given xml structure into the tree model and the Horde3D scene graph
	 * All successfully loaded nodes will then be synchronized for further modifications on the
	 * elements.
	 */
	bool loadSceneGraph(const QString& fileName);

	/**
	 * Saves all currently opened scene trees
	 */
	void save();

	/**
	 * Deletes the current model and emits a currentNodeChanged(0) signal
	 */
	void closeTree();

public slots:
	/**
	 * Removes the deepest scene graph layer
	 * @return bool false if the root scene graph is the current one
	 */
	bool popSceneGraph();


	QUndoStack* currentUndoStack();


	/**
	 * Adds a new scene node
	 */
	void addNode();

	/**
	 * \brief Adds an attachment to the current scene node
	 * 
	 */ 
	void addAttachmentNode();

	/**
	 * Enters a reference node's scenegraph
	 */
	void nodeActivated(const QModelIndex& index = QModelIndex());

signals:

	/**
	 * Signal will be triggered if more than one scene graph is active inside this view
	 * @param layer the scene graph file of the layer the currently active scene is child of
	 */
	void rootLayer(const QString& layer);

	/**
	 * Signal will be emitted if another scene graph file has become the active view
	 * @param sceneGraphFile the filename of the currently active scene graph
	 */
	void modelChanged(SceneTreeModel* model);

	/**
	 * Signal will be emitted if a property of the node has changed
	 */
	void nodePropertyChanged(QXmlTreeNode* node);

	/**
	 * Signal will be emitted just before the node selected will become active
	 */
	void nodeAboutToChange();

protected:
	/**
	 * Tries to select the specified node
	 */
	bool selectNode( int H3DNode );

	virtual void contextMenuEvent(QContextMenuEvent* event);
	virtual void dropEvent(QDropEvent* event);
	//virtual void startDrag(Qt::DropActions supportedActions);

	virtual QAddXmlNodeUndoCommand* createAddUndoCommand(const QDomElement& node, const QDomElement& parent, QXmlTreeModel* model, const QString& text) const;

	virtual QRemoveXmlNodeUndoCommand* createRemoveUndoCommand(QXmlTreeNode* node, const QString& text) const;

protected slots:	
	virtual void currentNodeChanged(const QModelIndex& current, const QModelIndex& previous);

private slots:
	
	/**
	 * sets the transformation of the currently selected node to the transformation
	 * of the node in the scene (it can differ if a script or physic engine or the user has changed the transformation)
	 */
	void useCameraTransformation();
	
	/**
	 * Sets the transformation of the currently active camera to the transformation of the currently selected node
	 */
	void setCameraTransformation();

	/**
	 * Will be called if node data has been changed to allow update of dependend views or actions
	 */
	void nodeDataChanged(const QModelIndex& index);

	void resizeColumns( const QModelIndex& index );

private:	
	void pushSceneGraph(SceneTreeModel* model);
	
	/**
	 * Searches recursively for the scenegraph that contains a node with a specific id
	 * @param model the root model that will be searched for the specific id
	 * @param H3DNode the id of the node to find
	 * @return QList<SceneTreeModel*> a list containing the model that contains the id and all of it's parent models
	 */
	QList<SceneTreeModel*> findSceneGraph(SceneTreeModel* model, unsigned int H3DNode);

	QList<SceneTreeModel*>	m_activeModels;

	QReferenceNode*			m_rootNode;

	bool					m_contextCall;

};
#endif 

