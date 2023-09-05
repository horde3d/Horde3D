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

#include "LuaFunctions.h"

extern "C"
{
        #include <lua.h>
        #include <lualib.h>
        #include <lauxlib.h>
}

#include <QtCore/QFile>

namespace Lua
{
	const char* functionScript = 
		"function printTable(t, tablename)																					\
			local functions = \"\"																							\
			for k,v in pairs(t) do																							\
				if type(v)==\"function\" then																				\
					if (string.len(tablename) > 0) then																		\
						functions = functions..tablename..\".\"..k..\",\"													\
					else																									\
						functions = functions..k..\",\"																		\
					end																										\
				elseif type(v) == \"table\" and k~=\"_G\" and k~=\"table\"  and k~=\"loaded\" and k~=\"preloaded\" then		\
					if (string.len(tablename) > 0) then																		\
						temp = printTable(v, tablename..\".\"..k)															\
						functions = functions..temp																			\
					else																									\
				        temp = printTable(v, k)																				\
						functions = functions..temp																			\
					end																										\
				end																											\
			end																												\
			return functions																								\
		 end																												\
																															\
		 function main()																									\
			return printTable(_G, \"\")																						\
		 end";

	void stackDump(lua_State* luaState) 
	{
		int i;
		int top = lua_gettop(luaState);
		for (i = 1; i <= top; i++) {  /* repeat for each level */
			int t = lua_type(luaState, i);
			switch (t) {
		  case LUA_TSTRING:  /* strings */
			  printf("`%s'", lua_tostring(luaState, i));
			  break;
		  case LUA_TBOOLEAN:  /* booleans */
			  printf(lua_toboolean(luaState, i) ? "true" : "false");
			  break;
		  case LUA_TNUMBER:  /* numbers */
			  printf("%g", lua_tonumber(luaState, i));
			  break;
		  default:  /* other values */
			  printf("%s", lua_typename(luaState, t));
			  break;
			}
			printf("  ");  /* put a separator */
		}
		printf("\n");  /* end the listing */
		lua_settop(luaState,0); // empty stack
	}

	QStringList functionList(lua_State* luaState)
	{		
		if (luaL_dostring(luaState,functionScript) != 0)
		{
			stackDump(luaState); //prints errors... 		
			return QStringList();
		}   
		// search for main function in lua script file
		lua_getglobal(luaState, "main");
		if (lua_pcall(luaState, 0, 1, 0) != 0)
		{		
			qWarning("LUA Error %s\n!",lua_tostring(luaState, -1));
			lua_pop(luaState,1);
			return QStringList();
		}
		if (!lua_isstring(luaState, -1))		
		{			
			lua_settop(luaState,0); // empty stack
			return QStringList();
		}
		QStringList retVal = QString(lua_tostring(luaState, -1)).split(',', Qt::SkipEmptyParts);
		// Add library names additionally
		for (int i=0; i<retVal.size(); ++i)
		{
			QString f(retVal.at(i));
			int index = f.indexOf('.');
			while (index != -1)
			{
				QString lib(f.left(index));
				if (!retVal.contains(lib))
					retVal.append(lib);
				index = f.indexOf('.', index+1);
			}
		}
		// sort list alphabetically
		retVal.sort();
		lua_settop(luaState,0); // empty stack
		return retVal;

	}
}
