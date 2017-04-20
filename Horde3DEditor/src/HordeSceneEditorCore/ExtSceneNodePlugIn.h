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


#ifndef ExtSceneNodePlugIn_H_
#define ExtSceneNodePlugIn_H_

#include <QtCore/QObject>
#include <QtXml/qdom.h>

struct lua_State;
class SceneTreeView;
class PlugInManager;

/**
* \brief This class provides an interface for plugins managing Horde3D SceneNode Extensions
* 
* Extensions can be used to enhence the available scene nodes in Horde3D 
* Using this plugin interface the editor can be enhanced with these extensions.
*/
class ExtSceneNodePlugIn
{	
public:
	/**
	 * Destructor
	 */
	virtual ~ExtSceneNodePlugIn() {}

	/**
	 * \brief returns the name of the node this plugin handles.
	 * @return QString name of the node
	 */
	virtual QString extensionName() const = 0;


	/**
	 * Provides the PlugInManager instance for the editor that loaded this plugin
	 */
	virtual void setPlugInManager(PlugInManager* factory) = 0;

	/**
	 * \brief Method for registering the plugin's lua functions
	 * 
	 * If the plugin's functionallity can be controlled by lua, it can implement
	 * the registration of these functions on the provided lua stack within this method.
	 * @param lua pointer to an existing lua stack used by the editor's LuaController
	 */
        virtual void registerLuaFunctions(lua_State* /*lua*/) {}

};

Q_DECLARE_INTERFACE(ExtSceneNodePlugIn,	"HordeSceneEditor.ExtSceneNodePlugIn/1.0")

#endif
