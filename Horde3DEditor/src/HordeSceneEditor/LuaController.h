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

#ifndef LUACONTROLLER_H_
#define LUACONTROLLER_H_

#include "ui_LuaController.h"

struct lua_State;
class SceneFile;
class QTimer;
class QStdOutListener;

/**
 * Manages the lua bindings 
 */
class LuaController : public QWidget, protected Ui_LuaController
{
	Q_OBJECT
public:
	LuaController(QWidget* parent = 0, Qt::WindowFlags flags = (Qt::WindowFlags) 0);
	virtual ~LuaController();

	/**
	 * Opens the lua state and registers the lua functions
	 */
	void setScene(SceneFile* scene);

	/**
	 * Returns a list of all registered lua functions
	 */
	QStringList functionList();

private slots:
	/**
	 * Executes the current code.	 	 
	 */
	void runScript();

	/**
	 * Toggles a timer to execute the current script in a loop
	 * @param loop enables/disables the timer
	 */
	void toggleLoopExecution(bool loop);
    
    /// Helper to add log non-empty messages to text field
    /// @param log the lua log message
    void appendLog( const QString& log );
    
private:
	/// prints the current lua stack to the standard output
	void stackDump();

	QStdOutListener*	m_stdOutListener;
	/// The LUA Stack
	lua_State*		m_L;

	/// Execution Timer for Loop execution
	QTimer*			m_execTimer;

};
#endif



