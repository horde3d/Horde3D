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

#include "LuaController.h"

extern "C"
{
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}
#include <EditorLib/LuaFunctions.h>

#include <QtCore/QDir>
#include <QtCore/QTimer>

#include "QStdOutListener.h"
#include "QHordeSceneEditorSettings.h"
#include "PlugInManager.h"
#include "LuaBindingsHorde.h"
#include "SceneFile.h"

//#define AVOIDSTDREDIRECT

LuaController::LuaController(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags), m_L(0)
{
	setupUi(this);
	m_execTimer = new QTimer(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	m_execTimer->setTimerType(Qt::PreciseTimer);
#endif
	connect(m_clearLog, SIGNAL(clicked()), m_scriptLog, SLOT(clear()));
	connect(m_executeButton, SIGNAL(clicked()), this, SLOT(runScript()));
	connect(m_runLoop, SIGNAL(toggled(bool)), this, SLOT(toggleLoopExecution(bool)));
	connect(m_execTimer, SIGNAL(timeout()), this, SLOT(runScript()));
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("Lua");
	m_scriptEditor->setPlainText(settings.value("Editor", "").toString());
	m_clearOnExecute->setChecked(settings.value("ClearOnExecute").toBool());
	settings.endGroup();
#ifndef AVOIDSTDREDIRECT
	m_stdOutListener = new QStdOutListener("stdoutlistener", this);
	connect(m_stdOutListener, SIGNAL(received(const QString&)), this, SLOT(appendLog(const QString&)));	
	m_stdOutListener->start(100);
#endif
}


LuaController::~LuaController()
{
#ifndef AVOIDSTDREDIRECT
	m_stdOutListener->stop();
#endif
	if (m_L)
		lua_close( m_L );	
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("Lua");
	settings.setValue("Editor", m_scriptEditor->toPlainText());
	settings.setValue("ClearOnExecute", m_clearOnExecute->isChecked());
	settings.endGroup();
}

void LuaController::setScene(SceneFile* scene)
{
	if (m_L)
	{
		m_runLoop->setChecked(false);
		lua_close( m_L );		
		m_L = 0;
	}
	if( scene )
	{
#if LUA_VERSION_NUM > 500
        m_L = luaL_newstate();
#else
		m_L = lua_open(); 
#endif
		luaL_openlibs( m_L );
		Horde3D::registerLuaBindings(m_L);
		// register lua functions of plugins on stack
		scene->pluginManager()->registerLuaFunctions(m_L);			
		// load autocomplete functions for lua editor
		m_scriptEditor->setCompleterPhrases(Lua::functionList(m_L));
	}
}

void LuaController::runScript()	
{	
	if (m_clearOnExecute->isChecked())
		m_scriptLog->clear();
	if (luaL_loadstring(m_L,qPrintable(m_scriptEditor->toPlainText())) != 0)
	{
		stackDump(); //prints errors... 
		m_runLoop->setChecked(false);
	}   
	else if (lua_pcall(m_L, 0, 0, 0)!=0)
	{
		stackDump(); //prints errors... 		
		m_runLoop->setChecked(false);
	}
	else
		lua_settop(m_L,0); // empty stack	
} 



void LuaController::toggleLoopExecution(bool loop)
{
	if( loop )
		m_execTimer->start(50);
	else
		m_execTimer->stop();
}

void LuaController::appendLog(const QString &log)
{
    if( log.trimmed().length() > 0 )
        m_scriptLog->append(log.trimmed());
}

void LuaController::stackDump () 
{
	QString dump;
	int i;
	int top = lua_gettop(m_L);
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(m_L, i);
		switch (t) {
		  case LUA_TSTRING:  /* strings */
			  dump += lua_tostring(m_L, i);
			  break;
		  case LUA_TBOOLEAN:  /* booleans */
			  dump += lua_toboolean(m_L, i) ? "true" : "false";
			  break;
		  case LUA_TNUMBER:  /* numbers */
			  dump += QString::number(lua_tonumber(m_L, i));
			  break;
		  default:  /* other values */
			  dump += lua_typename(m_L, t);
			  break;
		}
		dump += "  ";  /* put a separator */
	}
	dump += "\n";  /* end the listing */
	appendLog(dump);
	lua_settop(m_L,0); // empty stack
}


