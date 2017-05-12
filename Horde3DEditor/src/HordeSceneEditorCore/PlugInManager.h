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

#ifndef PlugInManager_H_
#define PlugInManager_H_

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtXml/qdom.h>

class QSceneNode;
class QXmlTreeModel;
class QXmlTreeNode;
class SceneTreeModel;
class AttachmentPlugIn;
class ToolPlugIn;
class QAction;
class QPropertyEditorWidget;
class QPluginLoader;

class SceneFile;
struct lua_State;

/**
 * The PlugInManager class loads all plugins available in the Horde3D Scene Editor
 *
 * It provides a registry for creating and loading of all nodes
 */
class PlugInManager : public QObject
{
	Q_OBJECT

public:
	enum NodeType {EXTRANODE, SCENENODE};

	typedef QDomElement (*NodeCreateFunc) (QWidget* parent);
	typedef QXmlTreeNode* (*NodeLoadFunc) (const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent);
	typedef QSceneNode* (*SceneNodeLoadFunc) (const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent);

	PlugInManager(QObject* parent = 0);
	virtual ~PlugInManager();

	/**
	 * Loads all plugins
	 */
	void init();

	/**
	 * Sets the property viewer widget (necessary to register custom types implemented in attachment plugin)	 
	 */
	void setPropertyWidget(QPropertyEditorWidget* widget) { m_propertyEditor = widget; }

	/**
	 * Sets the scene file to choose the correct attachment plugin specified in the scene
	 */
	void setSceneFile(SceneFile* scene);

	/**
	 * Returns the attachment plugin for the currently loaded scene
	 */
	AttachmentPlugIn* attachmentPlugIn() const {return m_currentAttachmentPlugIn;}

	void registerSceneNode( const QString& name, SceneNodeLoadFunc loadFunc, NodeCreateFunc createFunc);
	void unregisterSceneNode( const QString& name );
	QSceneNode* loadSceneNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent) const;

	void registerExtraNode( const QString& name, NodeLoadFunc loadFunc, NodeCreateFunc createFunc);
	void unregisterExtraNode( const QString& name );
	QXmlTreeNode* loadExtraNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent) const;

	QDomElement createNode(const QString& name, QWidget* parent = 0); 

	QStringList sceneNodeNames();

	void registerLuaFunctions( lua_State *L );

public slots:

	/**
	 * 
	 */
	void setCurrentNode(QXmlTreeNode* node);

signals:
	/// Will be emitted when a new node creation method has been registered
	void nodeRegistered(const QString& nodeName, PlugInManager::NodeType type);
	/// Will be emitted if a node creation method has been unregistered
	void nodeUnregistered(const QString& nodeName, PlugInManager::NodeType type);
	/// Will be emitted if a tool plugin has been loaded
	void toolRegistered(const QList<QAction*>& toolActions);

	/// Will be emitted if a plugin has changed the scene
	void sceneModified(bool modified);

private:

	QMap<QString, AttachmentPlugIn*>	m_attachmentPluginRegistry;
	
        QMap<QString, SceneNodeLoadFunc>        m_sceneNodeLoadRegistry;
        QMap<QString, NodeLoadFunc>             m_nodeLoadRegistry;
        QMap<QString, NodeCreateFunc>           m_nodeCreateRegistry;

		QList<QPluginLoader*>					m_loaders;

        QPropertyEditorWidget*                  m_propertyEditor;
        AttachmentPlugIn*                       m_currentAttachmentPlugIn;

        QList<ToolPlugIn*>                      m_toolPlugIns;

};
#endif
