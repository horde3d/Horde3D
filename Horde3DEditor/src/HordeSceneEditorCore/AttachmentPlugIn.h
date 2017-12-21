// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor but is licensed under the LGPL.
//
// The Plugin Interface is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

//
// ****************************************************************************************


#ifndef ATTACHMENTPLUGIN_H_
#define ATTACHMENTPLUGIN_H_

#include <QtCore/QObject>
#include <QtXml/qdom.h>
#include <QtCore/QFileInfoList>

struct lua_State;
class SceneFile;
class QXmlTreeNode;
class QXmlTreeModel;
class QPropertyEditorWidget;

/**
* \brief This class provides an interface for plugins managing different Horde3D attachments nodes
* 
* Attachment nodes can be used to integrate additional functionality to Horde3D scenegraph nodes.
* They can contain user-defined xml-data which will be sent by Horde3D to a registered callback during the 
* parsing of the scene graph. To manage these attachments within the editor you can implement this
* interface in your own plugin.
*/
class AttachmentPlugIn : public QObject
{	
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * normally not called explicitly since the plugin is loaded by the QPluginLoader
	 * @param parent optional parent 
	 */
	AttachmentPlugIn(QObject* parent = 0) : QObject(parent) {}

	/**
	 * Destructor
	 */
	virtual ~AttachmentPlugIn() {}

	/**
	 * \brief returns the name of the plugin.
	 * The name is used to identify if an attachment can be handled by the loaded plugin. Each
	 * attachment node will therefore define an attribute called "type" that contains the name of 
	 * the plugin returned by this method.
	 * @return QString name of the plugin
	 */
	virtual QString plugInName() const = 0;

	/**
	 * \brief a GUI for attachment manipulation through this plugin.
	 * The manipulation of the attachment configuration has to be implemented by this widget within the plugin.
	 * The Horde3D editor integrates the widget when a node with an attachment has been selected whose type
	 * attribute matches with the name of the plugin.	 
	 * @return QWidget* pointer to the GUI widget instance
	 */
	virtual QWidget* configurationWidget() = 0;

	/**
	 * \brief Init method when new scene file is loaded.
	 *
	 * Setting the current scene file allows the plugin to initialize engine specific settings
	 * like plugin specific directories and register custom properties in the QPropertyEditorWidget of the editor
	 * @param file instance to the currently lodaded scene file
	 * @param widget the property view used to display node properties
	 */
	virtual void init(SceneFile* file, QPropertyEditorWidget* widget) = 0;


	/**
	 * \brief informs the plugin about the currently selected node
	 * 
	 * Each time a different node has been selected in the editor this method will be called.
	 * If the node contains no attachment or no node was selected it will be called with a null pointer.
	 * @param parentNode the node instance that contains the attachment or a null pointer if no valid node has been selected
	 */
	virtual void setCurrentNode(QXmlTreeNode* parentNode) = 0;

	/**
	 * \brief update attachment engine	 
	 *
	 * If an attachment plugin was loaded the update method of it will be called before the render method.
	 */
	virtual void update() {};

	/**
	 * \brief Render method of plugin called instead of the Horde3D's render method
	 *
	 * If an attachment plugin was loaded the render method of it will be called instead of the Horde3D's render method to 
	 * allow additional debug geometry be rendered by the plugin.
	 * 
	 * @param activeCameraID the currently active camera node
	 */
	virtual void render( int activeCameraID ) = 0;

	/**
	 * \brief let the plugin initialize the node's attachment data when loading the node
	 * 
	 */
	virtual void initNodeAttachment(QXmlTreeNode* sceneNode) = 0;

	/**
	 * \brief let the plugin destroy the node's attachment data when removing the whole scene node from the graph
	 */
	virtual void destroyNodeAttachment(QXmlTreeNode* sceneNode) = 0;

	/**
	 * \brief create a new attachment for the current node
	 */
	virtual void createNodeAttachment() = 0;

	/**
	 * \brief remove an existing attachment from the current scene node
	 */
	virtual void removeNodeAttachment() = 0;

	virtual QXmlTreeModel* initExtras( const QDomElement& extraNode, QObject* parent ) = 0;

	/**
	 * \brief Will be called when a new scene file has been created.
	 * The plugin can implement a wizard here, which allows the user to configure plugin specific
	 * settings that have to be done when creating a new scene. e.g. specifiying directory settings
	 */
	virtual void sceneFileConfig() = 0;	

	/**
	 * \brief Method for registering the plugin's lua functions
	 * 
	 * If the plugin's functionallity can be controlled by lua, it can implement
	 * the registration of these functions on the provided lua stack within this method.
	 * @param lua pointer to an existing lua stack used by the editor's LuaController
	 */
	virtual void registerLuaFunctions(lua_State* lua) = 0;


	/**
	 * \brief This method has to be implemented by AttachmentPlugIns
	 *
	 * If a save as or clean scene command is triggered, the Editor will parse the complete scene file and
	 * scene graph files. If it encounters an Attachment node, it will call this methods that should return all files
	 * referenced in the given node
	 * @param node the attachment node that will be parsed for file references
	 * @return QFileInfoList all files referenced in the given node
	 */
	virtual QFileInfoList findReferences(const QDomElement& node) const = 0;


signals:
	/**
	 * Has to be emitted if the plugin has made changes on the scene file
	 */
	void modified(bool);
};

Q_DECLARE_INTERFACE(AttachmentPlugIn,	"HordeSceneEditor.AttachmentPlugIn/1.0")

#endif
