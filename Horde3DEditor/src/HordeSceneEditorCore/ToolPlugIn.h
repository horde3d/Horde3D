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


#ifndef TOOLPLUGIN_H_
#define TOOLPLUGIN_H_

#include <QAction>
#include <QtCore/QList>

class SceneFile;

/**
* \brief This class provides an interface for tool plugins integrateable in the editor
* 
* 
*/
class ToolPlugIn 
{	
public:

	/**
	 * Destructor
	 */
	virtual ~ToolPlugIn() {}

	/**
	 * \brief returns the name of the plugin. 
	 * 
	 * @return QString name of the plugin
	 */
	virtual QString plugInName() const = 0;

	/**
	 * \brief Actions provided by this plugin
	 * @return QList<QAction*> 
	 */
	virtual QList<QAction*> actions() const = 0;

	/**
	 * \brief Sets the currently loaded scene
	 * 
	 */
	virtual void setSceneFile(SceneFile* file) = 0;
	
};

Q_DECLARE_INTERFACE(ToolPlugIn,	"HordeSceneEditor.ToolPlugIn/1.0")

#endif
