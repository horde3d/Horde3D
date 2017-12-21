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

#include "LuaBindingsHorde.h"

#include <stdio.h>

extern "C"
{
    #include <lualib.h>
    #include <lauxlib.h>
}

#include <Horde3D.h>
#include <Horde3DUtils.h>

#if LUA_VERSION_NUM >= 503
#define luaL_checkint luaL_checkinteger
#endif
#if LUA_VERSION_NUM <= 500
#define luaL_Reg luaL_reg
#endif

namespace Horde3D
{

	namespace LuaBindings
	{

		/* Group: Basic functions */
		/* 	Function: lua_getVersionString
		Returns the engine version string.

		This function returns a pointer to a string containing the current version of Horde3D.

		Parameters:
		none

		Returns:
		pointer to the version string
		*/
		int  lua_getVersionString( lua_State *L )
		{
			lua_pushstring(L, h3dGetVersionString());
			return 1;
		}

		/* 	Function: checkExtension
		Checks if an extension is part of the engine library.

		This function checks if a specified extension is contained in the DLL/shared object of the engine.

		Parameters:
		extensionName	- name of the extension

		Returns:
		true if extension is implemented, otherwise false
		*/
		int  lua_checkExtension( lua_State *L )
		{
			lua_pushboolean( L, h3dCheckExtension(luaL_checkstring(L, 1)) );
			return 1;
		}

		/* 	Function: init
		Initializes the engine.

		This function initializes the graphics engine and makes it ready for use. It has to be the
		first call to the engine except for getVersionString. In order to successfully initialize
		the engine the calling application must provide a valid OpenGL context.

		Parameters:
		none

		Returns:
		true in case of success, otherwise false
		*/
		int  lua_init( lua_State *L )
		{
            int backend = luaL_checkint(L, 1);
            switch( backend )
            {
            case H3DRenderDevice::OpenGL4:
            case H3DRenderDevice::OpenGL2:
                lua_pushboolean( L, h3dInit((H3DRenderDevice::List)backend) );
                return 1;
            default:
                luaL_error(L, "Unknown Render backend requested");
                return 0;
            }
		}

		/* 	Function: release
		Releases the engine.

		This function releases the engine and frees all objects and associated memory. It should be
		called when the application is destroyed.

		Parameters:
		none

		Returns:
		nothing
		*/
		int lua_release( lua_State *L )
		{
			luaL_error( L, "lua call to Horde3D::release is not allowed " );
			return 0;
		}

		/* 	Function: resizePipelineBuffers
		Changes the size of the render targets of a pipeline.

		This function sets the base width and height which affects render targets with relative (in percent) size 
		specification. Changing the base size is usually desired after engine initialization and when the window
		is being resized. Note that in case several cameras use the same pipeline resource instance, the change
		will affect all cameras.

		Parameters:
			pipeRes  - the pipeline resource instance to be changed
			width    - base width in pixels used for render targets with relative size
			height   - base height in pixels used for render targets with relative size

		Returns:
		nothing
		*/		
		int lua_ResizePipelineBuffers( lua_State *L )
		{
			int pipeRes = luaL_checkint(L, 1);
			int width = luaL_checkint(L, 2);
			int height = luaL_checkint(L, 3);			
			h3dResizePipelineBuffers( pipeRes, width, height );
			return 0;
		}

		/* 	Function: render
		Main rendering function.

		This is the main function of the engine. It executes all the rendering, animation and other
		tasks. The function can be called several times per frame, for example in order to write to different
		output buffers.

		Parameters:
		cameraNode	- camera node used for rendering scene

		Returns:
		true in case of success, otherwise false
		*/
		int lua_render( lua_State *L )
		{
			int cameraID = luaL_checkint(L, 1);
			h3dRender(cameraID);
			return 0;
		}


		/* 	Function: clear
		Removes all resources and scene nodes.

		This function removes all nodes from the scene graph except the root node and releases all resources.

		*Warning: All resource and node IDs are invalid after calling this function*

		Parameters:
		none

		Returns:
		nothing
		*/
                int lua_clear( lua_State * )
		{
			h3dClear();
			return 0;
		}

		/* 	Function: getMessage
		Gets the next message from the message queue.

		This function returns the next message string from the message queue and writes additional information
		to the specified variables. If no message is left over in the queue an empty string is returned.

		Parameters:
		level	- pointer to variable for storing message level indicating importance (can be NULL)
		time	- pointer to variable for storing time when message was added (can be NULL)

		Returns:
		message string or empty string if no message is in queue
		*/
		int lua_getMessage( lua_State *L )
		{
			float time;
			int level;
			const char* msg = h3dGetMessage(&level, &time);
			lua_pushinteger(L, level);
			lua_pushnumber(L, time);
			lua_pushstring(L, msg);
			return 3;
		}


		/* 	Function: getOption
		Gets an option parameter of the engine.

		This function gets a specified option parameter and returns its value.

		Parameters:
		param	- option parameter

		Returns:
		current value of the specified option parameter
		*/
		int  lua_getOption( lua_State *L )
		{
			H3DOptions::List option = (H3DOptions::List) luaL_checkint(L,1);			
			lua_pushnumber(L,h3dGetOption(option)); 
			return 1;
		}

		/* 	Function: setOption
		Sets an option parameter for the engine.

		This function sets a specified option parameter to a specified value.

		Parameters:
		param	- option parameter
		value	- value of the option parameter

		Returns:
		true if the option could be set to the specified value, otherwise false
		*/
		int  lua_setOption( lua_State *L )
		{
			H3DOptions::List param = (H3DOptions::List) luaL_checkint(L, 1);
			float value = static_cast<float>(luaL_checknumber(L, 2));
			lua_pushboolean(L, h3dSetOption(param, value));
			return 1;
		}

		/* 	Function: showOverlay
		Shows an overlay on the screen.

		This function displays an overlay with a specified material at a specified position on the screen.
		An overlay is a 2D image that can be used to render 2D GUI elements. The coordinate system
		used has its origin (0, 0) at the lower left corner of the screen and its maximum (1, 1) at
		the upper right corner. Texture coordinates are using the same system, where the coordinates
		(0, 0) correspond to the lower left corner of the image.
		Overlays can have different layers which describe the order in which they are drawn. Overlays with
		smaller layer numbers are drawn before overlays with higher layer numbers.
		Note that the overlays have to be removed manually using the function clearOverlays.

		Parameters:
		x_ll, y_ll, u_ll, v_ll	- position and texture coordinates of the lower left corner
		x_lr, y_lr, u_lr, v_lr	- position and texture coordinates of the lower right corner
		x_ur, y_ur, u_ur, v_ur	- position and texture coordinates of the upper right corner
		x_ul, y_ul, u_ul, v_ul	- position and texture coordinates of the upper left corner
		colR, colG, colB, colA  - color of the overlay that is set for the material's shader
		materialRes				- material resource used for rendering
		layer					- layer index of the overlay (Values: from 0 to 7)

		Returns:
		nothing
		*/
		int  lua_showOverlays( lua_State *L )			
		{
			float x_ll = static_cast<float>(luaL_checknumber(L,1));
			float y_ll = static_cast<float>(luaL_checknumber(L,2));
			float u_ll = static_cast<float>(luaL_checknumber(L,3));
			float v_ll = static_cast<float>(luaL_checknumber(L,4));
			float x_lr = static_cast<float>(luaL_checknumber(L,5));
			float y_lr = static_cast<float>(luaL_checknumber(L,6));
			float u_lr = static_cast<float>(luaL_checknumber(L,7));
			float v_lr = static_cast<float>(luaL_checknumber(L,8));
			float x_ur = static_cast<float>(luaL_checknumber(L,9));
			float y_ur = static_cast<float>(luaL_checknumber(L,10));
			float u_ur = static_cast<float>(luaL_checknumber(L,11));
			float v_ur = static_cast<float>(luaL_checknumber(L,12));
			float x_ul = static_cast<float>(luaL_checknumber(L,13));
			float y_ul = static_cast<float>(luaL_checknumber(L,14));
			float u_ul = static_cast<float>(luaL_checknumber(L,15));
			float v_ul = static_cast<float>(luaL_checknumber(L,16));
			float colR = static_cast<float>(luaL_checknumber(L,17));
			float colG = static_cast<float>(luaL_checknumber(L,18));
			float colB = static_cast<float>(luaL_checknumber(L,19));
			float colA = static_cast<float>(luaL_checknumber(L,20));
			H3DRes material = luaL_checkint(L, 20);
			int layer = luaL_checkint(L, 22);
			//h3dShowOverlays(x_ll, y_ll, u_ll, v_ll, x_lr, y_lr, u_lr, v_lr, x_ur, y_ur, u_ur, v_ur, x_ul, y_ul, u_ul, v_ul, colR, colG, colB, colA, material, layer);
			return 0;
		}

		/* 	Function: clearOverlays
		Removes all overlays.

		This function removes all overlays that were added usig showOverlay.

		Parameters:
		none

		Returns:
		nothing
		*/
                int  lua_clearOverlays( lua_State * )
		{
			h3dClearOverlays();
			return 0;
		}

		/* Group: General resource management functions */
		/* 	Function: getResourceType
		Returns the type of a resource.

		This function returns the type of a specified resource. If the resource handle is invalid,
		the function returns the resource type 'Unknown'.

		Parameters:
		res	- handle to the resource

		Returns:
		type of the resource
		*/
		int  lua_getResType( lua_State *L )
		{
			H3DRes res = luaL_checkint(L, 1);			
			lua_pushinteger(L, h3dGetResType(res));
			return 1;
		}		

		/* 	Function: findResource
		Finds a resource and returns its handle.

		This function searches the resource of the specified type and name and returns its handle. If
		the resource is not available in the resource manager a zero handle is returned.

		Parameters:
		type	- type of the resource
		name	- name of the resource

		Returns:
		handle to the resource or 0 if not found
		*/
		int  lua_findResource( lua_State *L )
		{
			H3DResTypes::List type = (H3DResTypes::List) luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);				
			lua_pushinteger(L,h3dFindResource(type, name)); 
			return 1;
		}

		/* 	Function: addResource
		Adds a resource.

		This function tries to add a resource of a specified type and name to the resource manager. If
		a resource of the same type and name is already existing, the handle to the existing resource is returned
		instead of creating a new one and the user reference count of the resource is increased.

		*Note: The name string may not contain a colon character (:)*

		Parameters:
		type	- type of the resource
		name	- name of the resource
		flags	- flags used for creating the resource

		Returns:
		handle to the resource to be added or 0 in case of failure
		*/
		int  lua_addResource( lua_State *L )
		{
			H3DResTypes::List type = (H3DResTypes::List) luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);	
			unsigned int flags = luaL_checkint(L, 3);			
			lua_pushinteger(L,h3dAddResource(type, name, flags)); 
			return 1;
		}

		/* 	Function: cloneResource
		Duplicates a resource.

		This function duplicates a specified resource. In the cloning process a new resource with the
		specified name is added to the resource manager and filled with the data of the specified source
		resource. If the specified name for the new resource is already in use, the function fails and
		returns 0. If the name string is empty, a unique name for the resource is generated automatically.

		*Note: The name string may not contain a colon character (:)*

		Parameters:
		sourceRes	- handle to resource to be cloned
		name		- name of new resource (can be empty for auto-naming)

		Returns:
		handle to the cloned resource or 0 in case of failure
		*/
		int  lua_cloneResource( lua_State *L )
		{
			H3DRes res = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);				
			lua_pushinteger(L,h3dCloneResource(res, name)); 
			return 1;
		}

		/* 	Function: removeResource
		Removes a resource.

		This function decreases the user reference count of a specified resource. When the user reference
		count is zero and there are no internal references, the resource can be released and removed using
		the API fuction releaseUnusedResources.

		Parameters:
		res	- handle to the resource to be removed

		Returns:
		the number of references that the application is still holding after removal or -1 in case of an error
		*/	
		int  lua_removeResource( lua_State *L )
		{
			H3DRes res = luaL_checkint(L, 1);
			lua_pushboolean(L, h3dRemoveResource(res));
			return 1;
		}

		/* 	Function: loadResource
		Loads a resource.

		This function loads data for a resource that was previously added to the resource manager.
		If data is a NULL-pointer the resource manager is told that the resource doesn't have any data.
		This function can only be called once for every resource.

		*Important Note: XML-data must be NULL-terminated*

		Parameters:
		res		- handle to the resource for which data is loaded
		data	- pointer to the data to be loaded
		size	- size of the data block

		Returns:
		true in case of success, otherwise false
		*/
		int lua_loadResource( lua_State *L )
		{
			luaL_error(L, "lua binding not yet implemented");
			return 0;
		}


		/* 	Function: unloadResource
		Unloads a resource.

		This function unloads a previously loaded resource and restores the default values
		it had before loading. The state is set back to unloaded which makes it possible to load
		the resource again.

		Parameters:
		res	- handle to resource to be unloaded

		Returns:
		true in case of success, otherwise false
		*/
		int lua_unloadResource( lua_State *L )
		{
			h3dUnloadResource((H3DRes) luaL_checkint(L, 1));
			return 0;
		}

		/* 	Function: getResourceParami
		Gets a property of a resource.

		This function returns a specified property of the specified resource.
		The property must be of the type int.

		Parameters:
		res		- handle to the resource to be accessed
		param	- parameter to be accessed

		Returns:
		value of the parameter
		*/
		int lua_getResParamI( lua_State *L )
		{
			lua_pushinteger(L, h3dGetResParamI(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4)));
			return 1;
		}

		/* 	Function: setResourceParami
			Sets a property of a resource.
		
		This function sets a specified property of the specified resource to a specified value.
		The property must be of the type int.
 		
		Parameters:
			node	- handle to the node to be modified
			param	- parameter to be modified
			value	- new value for the specified parameter
			
		Returns:
			 true in case of success otherwise false
		*/
		int lua_setResParamI( lua_State *L )
		{
			h3dSetResParamI(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4), luaL_checkint(L, 5) );
			return 0;
		}

		/* 	Function: getResourceParamf
		Gets a property of a resource.

		This function returns a specified property of the specified resource.
		The property must be of the type float.

		Parameters:
		res		- handle to the resource to be accessed
		param	- parameter to be accessed

		Returns:
		value of the parameter
		*/
		int lua_getResParamF( lua_State *L )
		{
			lua_pushnumber(L, h3dGetResParamF(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4), luaL_checkint(L, 5)));
			return 1;
		}

		/* 	Function: setResourceParamf
			Sets a property of a resource.
		
		This function sets a specified property of the specified resource to a specified value.
		The property must be of the type int.
 		
		Parameters:
			node	- handle to the node to be modified
			param	- parameter to be modified
			value	- new value for the specified parameter
			
		Returns:
			 true in case of success otherwise false
		*/
		int lua_setResParamF( lua_State *L )
		{
			h3dSetResParamF( luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4), luaL_checkint(L, 5), static_cast<float>( luaL_checknumber(L, 6) ) );
			return 0;
		}

		/* 	Function: getResourceParamf
		Gets a property of a resource.

		This function returns a specified property of the specified resource.
		The property must be of the type float.

		Parameters:
		res		- handle to the resource to be accessed
		param	- parameter to be accessed

		Returns:
		value of the parameter
		*/
		int lua_getResParamStr( lua_State *L )
		{
			lua_pushstring(L, h3dGetResParamStr(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4)));
			return 1;
		}

		/* 	Function: setResourceParamf
			Sets a property of a resource.
		
		This function sets a specified property of the specified resource to a specified value.
		The property must be of the type int.
 		
		Parameters:
			node	- handle to the node to be modified
			param	- parameter to be modified
			value	- new value for the specified parameter
			
		Returns:
			 true in case of success otherwise false
		*/
		int lua_setResParamStr( lua_State *L )
		{
			h3dSetResParamStr( luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4), luaL_checkstring(L, 5) );
			return 0;
		}

		/* 	Function: getResourceData
		Gives access to resource data.

		This function returns a pointer to the specified data of a specified resource. For information on the
		format (uint, float, ..) of the pointer see the ResourceData description.

		*Important Note: The pointer is const and allows only read access to the data. Do never try to modify the
		data of the pointer since that can corrupt the engine's internal states!*

		Parameters:
		res		- handle to the resource to be accessed
		param	- parameter indicating data of the resource that will be accessed

		Returns:
		pointer to the specified resource data if it is available, otherwise NULL-pointer
		*/
		int lua_getResourceData( lua_State *L )
		{
			luaL_error(L, "lua binding not yet implemented");
			return 0;
		}

		/* 	Function: updateResourceData
		Updates the data of a resource.

		This function updates the content of a resource that was successfully loaded before. The new data must
		have exactly the same data layout as the data that was loaded.

		Notes on available ResourceData parameters:
		- Texture2DResData::PixelData
		Sets the image data of a Texture2D resource. The data pointer must point to a memory block that contains
		the pixels of the image. Each pixel needs to have 32 bit color data in BGRA format and the dimensions
		of the image (width, height) must be exactly the same as the dimensions of the image that was
		originally loaded for the resource. The first element in the data array corresponds to the lower left
		corner of the image and subsequent elements progress from left to right in the image.

		Parameters:
		res		- handle to the resource for which the data is modified
		param	- data structure which will be updated
		data	- pointer to the new data
		size	- size of the new data block

		Returns:
		true in case of success, otherwise false
		*/
		int lua_updateResourceData( lua_State *L )
		{
			luaL_error(L, "lua binding not yet implemented");
			return 0;
		}

		/* 	Function: queryUnloadedResource
		Returns handle to an unloaded resource.

		This function looks for a resource that is not yet loaded and returns its handle.
		If there are no unloaded resources, 0 is returned.

		Parameters:
		none

		Returns:
		handle to an unloaded resource or 0
		*/
		int  lua_queryUnloadedResource( lua_State *L )
		{
			lua_pushnumber(L, h3dQueryUnloadedResource( luaL_checkint(L, 1) ));
			return 1;
		}

		/* 	Function: releaseUnusedResources
		Frees resources that are no longer used.

		This function releases resources that are no longer used. Unused resources were either told
		to be released by the user calling egReleaseResource or are no more referenced by any other
		engine objects.

		Parameters:
		none

		Returns:
		nothing
		*/
		int  lua_releasedUnusedResources( lua_State* /*L*/ )
		{			
			h3dReleaseUnusedResources();
			return 0;
		}

		/* Group: Specific resource management functions */
	/* 	Function: createTexture2D
			Adds a Texture2D resource.
		
		This function tries to create and add a Texture2D resource with the specified name to the resource
		manager. If a Texture2D resource with the same name is already existing, the function fails. The
		texture is initialized with the specified dimensions and the resource is declared as loaded. This
		function is especially useful to create dynamic textures (e.g. for displaying videos) or output buffers
		for render-to-texture.

		*Note: The name string may not contain a colon character (:)*
		
		Parameters:
			name		- name of the resource
			flags		- flags used for creating the resource
			width		- width of the texture image
			height		- height of the texture image
			renderable	- flag indicating whether the texture can be used as an output buffer for a Camera node
			
		Returns:
			handle to the created resource or 0 in case of failure
	*/
	int lua_createTexture2D( lua_State *L )
	{
			luaL_error(L, "lua binding not yet implemented");
			return 0;
		}

	/* 	Function: setShaderPreambles
			Sets preambles of all Shader resources.
		
        This function defines a header that is inserted at the beginning of all shaders. The preamble
        is used when a shader is compiled, so changing it will not affect any shaders that are already
        compiled. The preamble is useful for setting platform-specific defines that can be employed for
        creating several shader code paths, e.g. for supporting different hardware capabilities.
        Based on render device requirements, engine may internally set shader preambles. OpenGL 4 render device
        sets shader preambles to specify minimum version, required to compile a shader program of required type.
		
		Parameters:
			vertPreamble	- preamble text of vertex shaders (default: empty string)
			fragPreamble	- preamble text of fragment shaders (default: empty string)
            geomPreamble  - preamble text of geometry shaders (default: empty string)
            tessControlPreamble - preamble text of tessellation control shaders (default: empty string)
            tessEvalPreamble - preamble text of tessellation evaluation shaders (default: empty string)
            computePreamble - preamble text of compute shaders (default: empty string)
		Returns:
			nothing
	*/
	int lua_setShaderPreambles( lua_State *L )
	{
        h3dSetShaderPreambles( luaL_checkstring(L, 1), luaL_checkstring(L, 2), luaL_checkstring(L, 3), luaL_checkstring(L, 4), luaL_checkstring(L, 5), luaL_checkstring(L, 6) );
		return 0;
	}

	/* 	Function: setMaterialUniform
			Sets a shader uniform of a Material resource.
		
		This function sets the specified shader uniform of the specified material to the specified values.
		
		Parameters:
			materialRes	- handle to the Material resource to be accessed
			name		- name of the uniform as defined in Material resource
			a, b, c, d	- values of the four components
			
		Returns:
			true in case of success, otherwise false
	*/
	int lua_setMaterialUniform( lua_State *L )
	{
		lua_pushboolean( L, h3dSetMaterialUniform( luaL_checkint(L, 1), luaL_checkstring(L, 2), 
			float( luaL_checknumber(L, 3) ), 
			float( luaL_checknumber(L, 4) ),
			float( luaL_checknumber(L, 5) ),
			float( luaL_checknumber(L, 6) ) ) );
		return 1;
	}
	

		/* 	Function: getPipelineRenderTargetData
				Reads the pixel data of a pipeline render target buffer.
			
			This function reads the pixels of the specified buffer of the specified render target from the specified
			pipeline resource and stores it in the specified float array. To calculate the size required for the array this
			function can be called with a NULL pointer for dataBuffer and pointers to variables where the width,
			height and number of (color) components (e.g. 4 for RGBA or 1 for depth) will be stored.
			The function is not intended to be used for real-time scene rendering but rather as a tool for debugging.
			For more information about the render buffers please refer to the Pipeline documentation.
			
			Parameters:
				pipelineRes	- handle to pipeline resource
				targetName	- unique name of render target to access
				bufIndex	- index of buffer to be accessed
				width		- pointer to variable where the width of the buffer will be stored (can be NULL)
				height		- pointer to variable where the height of the buffer will be stored (can be NULL)
				compCount	- pointer to variable where the number of components will be stored (can be NULL)
				dataBuffer	- pointer to float array where the pixel data will be stored (can be NULL)
				bufferSize	- size of dataBuffer array in bytes
				
			Returns:
				true in case of success, otherwise false
		*/
		int lua_getPipelineRenderTargetData( lua_State *L )
		{
			luaL_error(L, "lua binding not yet implemented");
			return 0;
		}

		/* Group: General scene graph functions */
		/* 	Function: getNodeType
		Returns the type of a scene node.

		This function returns the type of a specified scene node. If the node handle is invalid,
		the function returns the node type 'Unknown'.

		Parameters:
		node	- handle to the scene node

		Returns:
		type of the scene node
		*/
		int  lua_getNodeType( lua_State *L )
		{
			H3DNode node = luaL_checkint(L,1);			
			lua_pushinteger(L,h3dGetNodeType(node)); 
			return 1;
		}

		/* 	Function: getNodeParamstr
				Gets a property of a scene node.
			
			This function returns a specified property of the specified node.
			The property must be of the type string (const char *).
			
			Parameters:
				node	- handle to the node to be accessed
				param	- parameter to be accessed
				
			Returns:
				 value of the property or empty string if no such property exists
		*/
		int  lua_getNodeParamStr( lua_State *L )
		{
			H3DNode node = luaL_checkint(L,1);			
			lua_pushstring(L,h3dGetNodeParamStr(node, luaL_checkint(L, 2))); 
			return 1;
		}


		/* 	Function: setNodeParamstr
			Sets a property of a scene node.
		
		This function sets a specified property of the specified node to a specified value.
		The property must be of the type string (const char *).
		
		Parameters:
			node	- handle to the node to be modified
			param	- parameter to be modified
			value	- new value for the specified parameter
			
		Returns:
			 true in case of success otherwise false
		*/
		int  lua_setNodeParamStr( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			int param = luaL_checkint(L, 2);
			const char* name = luaL_checkstring(L, 3);
			h3dSetNodeParamStr(node, param, name);
			return 0;
		}

		/* Function: h3dGetNodeFlags
		Gets the scene node flags.

			Details:
				This function returns a bit mask containing the set scene node flags.
			
			Parameters:
				node  - handle to the node to be accessed
				
			Returns:
				flag bitmask
		*/
		int  lua_getNodeFlags( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			lua_pushinteger( L, h3dGetNodeFlags(node) );
			return 1;
		}

		/* 	Function: h3dSetNodeFlags
				Sets the scene node flags.
	
			Details:
				This function sets the flags of the specified scene node.
			
			Parameters:
				node       - handle to the node to be modified
				flags      - new flag bitmask
				recursive  - specifies whether flags should be applied recursively to all child nodes
				
			Returns:
				nothing
		*/
		int  lua_setNodeFlags( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			int flags = luaL_checkint(L, 2);
			const bool recursive = luaL_checkint(L, 3) != 0;
			h3dSetNodeFlags(node, flags, recursive);
			return 0;
		}

		/* 	Function: getNodeAABB
				Gets the bounding box of a scene node.
			
			This function stores the world coordinates of the axis aligned bounding box of a specified node in
			the specified variables. The bounding box is represented using the minimum and maximum coordinates
			on all three axes.
			
			Parameters:
				node				- handle to the node which will be accessed
				minX, minY, minZ	- pointers to variables where minimum coordinates will be stored
				maxX, maxY, maxZ	- pointers to variables where maximum coordinates will be stored
				
			Returns:
				true in case of success otherwise false
		*/
		int  lua_getNodeAABB( lua_State *L )
		{
			H3DNode  node = luaL_checkint(L,1);
			float minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;
			h3dGetNodeAABB(node, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);			
			lua_pushnumber(L, minX);
			lua_pushnumber(L, minY);
			lua_pushnumber(L, minZ);
			lua_pushnumber(L, maxX);
			lua_pushnumber(L, maxY);
			lua_pushnumber(L, maxZ);			
			return 6;
		}


		/* 	Function: getNodeParent
				Returns the parent of a scene node.
			
			This function returns the handle to the parent node of a specified scene node. If the specified
			node handle is invalid or the root node, 0 is returned.
			
			Parameters:
				node	- handle to the scene node
				
			Returns:
				handle to parent node or 0 in case of failure
		*/
		int  lua_getNodeParent( lua_State *L )
		{
			H3DNode node = luaL_checkint(L,1);			
			lua_pushinteger(L,h3dGetNodeParent(node)); 
			return 1;
		}

	/* 	Function: setNodeParent
			Relocates a node in the scene graph.
		
		This function relocates a scene node. It detaches the node from its current parent and attaches
		it to the specified new parent node. If the attachment to the new parent is not possible, the
		function returns false. Relocation is not possible for the H3DRootNode.
		
		Parameters:
			node	- handle to the scene node to be relocated
			parent	- handle to the new parent node
			
		Returns:
			true in case of success, otherwise false
	*/
		int  lua_setNodeParent( lua_State *L )
		{
			H3DNode node = luaL_checkint(L,1);			
			lua_pushboolean(L, h3dSetNodeParent(node, luaL_checkint(L, 2) )); 
			return 1;
		}


		/* 	Function: getNodeChild
		Returns the handle to a child node.

		This function looks for the n-th (index) child node of a specified node and returns its handle. If the child
		doesn't exist, the function returns 0.

		Parameters:
		node	- handle to the parent node
		index	- index of the child node

		Returns:
		handle to the child node or 0 if child doesn't exist
		*/
		int  lua_getNodeChild( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			unsigned int index = luaL_checkint(L, 2);							
			lua_pushinteger(L,h3dGetNodeChild(parent, index)); 
			return 1;
		}

		/* 	Function: addNodes
				Adds nodes from a SceneGraph resource to the scene.
			
			This function creates several new nodes as described in a SceneGraph resource and
			attaches them to a specified parent node.

			*Important Note: The SceneGraph resource needs to be loaded!*
			
			Parameters:
				parent			- handle to parent node to which the root of the new nodes will be attached
				sceneGraphRes	- handle to SceneGraph resource
				
			Returns:
				handle to the root of the created nodes or 0 in case of failure
		*/
		int  lua_addNodes( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			H3DRes res  = luaL_checkint(L, 2);
			lua_pushinteger(L,h3dAddNodes(parent, res)); 
			return 1;
		}

		/* 	Function: removeNode
			Removes a node from the scene.
		
		This function removes the specified node and all of it's children from the scene.
		
		Parameters:
			node	- handle to the node to be removed
			
		Returns:
			true in case of success otherwise false
		*/
		int  lua_removeNode( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			h3dRemoveNode(node);
			return 0;
		}		

		/* 	Function: checkNodeTransformFlag
		Checks if a scene node has been transformed by the engine.

		This function checks if a scene node has been transformed by the engine since the last
		time the transformation flag was reset. Therefore, it stores a flag that is set to true when a
		setTransformation function is called explicitely by the application or when the node transformation
		has been updated by the animation system. The function also makes it possible to reset the
		transformation flag.

		Parameters:
		node	- handle to the node to be accessed
		reset	- flag indicating whether transformation flag shall be reset

		Returns:
		true if node has been transformed, otherwise false
		*/
		int  lua_checkNodeTransformFlag( lua_State *L )
		{			
			H3DNode node = luaL_checkint(L, 1);
			const bool reset = luaL_checkint(L, 2) != 0;
			lua_pushboolean(L, h3dCheckNodeTransFlag(node, reset));
			return 1;
		}

		
		/* 	Function: getNodeTransform
			Gets the relative transformation of a node.
		
		This function gets the translation, rotation and scale of a specified scene node object. The
		coordinates are in local space and contain the transformation of the node relative to its parent.
		
		Parameters:
			node		- handle to the node which will be accessed
			tx, ty, tz	- pointers to variables where translation of the node will be stored (can be NULL)
			rx, ry, rz	- pointers to variables where rotation of the node in Euler angles
						  will be stored (can be NULL)
			sx, sy, sz	- pointers to variables where scale of the node will be stored (can be NULL)
			
		Returns:
			true in case of success otherwise false
	*/
		int lua_getNodeTransform( lua_State *L )
		{
			unsigned int id = luaL_checkint(L,1);
			float x, y, z, rx, ry, rz, sx, sy, sz;
			h3dGetNodeTransform(id, &x,&y,&z,&rx,&ry,&rz,&sx,&sy,&sz);
			lua_pushnumber(L, x);
			lua_pushnumber(L, y);
			lua_pushnumber(L, z);
			lua_pushnumber(L, rx);
			lua_pushnumber(L, ry);
			lua_pushnumber(L, rz);
			lua_pushnumber(L, sx);
			lua_pushnumber(L, sy);
			lua_pushnumber(L, sz);
			return 9;
		}	

		/* 	Function: setNodeTransform
		Sets the relative transformation of a node.

		This function sets the relative translation, rotation and scale of a specified scene node object.
		The coordinates are in local space and contain the transformation of the node relative to its parent.

		Parameters:
		node		- handle to the node which will be modified
		tx, ty, tz	- translation of the node
		rx, ry, rz	- rotation of the node in Euler angles
		sx, sy, sz	- scale of the node

		Returns:
		true in case of success otherwise false
		*/
		int lua_setNodeTransform( lua_State *L )
		{
			unsigned int id = luaL_checkint(L, 1);
			h3dSetNodeTransform(id,
				// translation
				static_cast<float>(luaL_checknumber(L, 2)), static_cast<float>(luaL_checknumber(L, 3)), static_cast<float>(luaL_checknumber(L, 4)),
				// rotation
				static_cast<float>(luaL_checknumber(L, 5)), static_cast<float>(luaL_checknumber(L, 6)), static_cast<float>(luaL_checknumber(L, 7)),
				// scale
				static_cast<float>(luaL_checknumber(L, 8)), static_cast<float>(luaL_checknumber(L, 9)), static_cast<float>(luaL_checknumber(L, 10)));
			return 0;
		}	

		/* 	Function: getNodeTransformMatrices
		Returns the transformation matrices of a node.

		This function stores a pointer to the relative and absolute transformation matrices
		of the specified node in the specified pointer varaibles.

		Parameters:
		node	- handle to the scene node to be accessed
		relMat	- pointer to a variable where the address of the relative transformation matrix will be stored
		(can be NULL if matrix is not required)
		absMat	- pointer to a variable where the address of the absolute transformation matrix will be stored
		(can be NULL if matrix is not required)


		Returns:
		true in case of success otherwise false
		*/
		int lua_getNodeTransformMatrices( lua_State *L )
		{
			int node = luaL_checkint( L, 1 );
			const float *relMatrix = 0, *absMatrix = 0;
			h3dGetNodeTransMats( node, &relMatrix, &absMatrix );
			
			int retVals = 0;					
			if( relMatrix )
			{
				lua_pushnumber( L, relMatrix[0] ); lua_pushnumber( L, relMatrix[1] ); lua_pushnumber( L, relMatrix[2] ); lua_pushnumber( L, relMatrix[3] );
				lua_pushnumber( L, relMatrix[4] ); lua_pushnumber( L, relMatrix[5] ); lua_pushnumber( L, relMatrix[6] ); lua_pushnumber( L, relMatrix[7] );
				lua_pushnumber( L, relMatrix[8] ); lua_pushnumber( L, relMatrix[9] ); lua_pushnumber( L, relMatrix[10] ); lua_pushnumber( L, relMatrix[11] );
				lua_pushnumber( L, relMatrix[12] ); lua_pushnumber( L, relMatrix[13] ); lua_pushnumber( L, relMatrix[14] ); lua_pushnumber( L, relMatrix[15] );
				retVals += 16;
			}
			
			if( absMatrix )
			{
				lua_pushnumber( L, absMatrix[0] ); lua_pushnumber( L, absMatrix[1] ); lua_pushnumber( L, absMatrix[2] ); lua_pushnumber( L, absMatrix[3] );
				lua_pushnumber( L, absMatrix[4] ); lua_pushnumber( L, absMatrix[5] ); lua_pushnumber( L, absMatrix[6] ); lua_pushnumber( L, absMatrix[7] );
				lua_pushnumber( L, absMatrix[8] ); lua_pushnumber( L, absMatrix[9] ); lua_pushnumber( L, absMatrix[10] ); lua_pushnumber( L, absMatrix[11] );
				lua_pushnumber( L, absMatrix[12] ); lua_pushnumber( L, absMatrix[13] ); lua_pushnumber( L, absMatrix[14] ); lua_pushnumber( L, absMatrix[15] );
				retVals += 16;
			}			
			return retVals;			
		}

		/* 	Function: setNodeTransformMatrix
		Sets the relative transformation matrix of a node.

		This function sets the relative transformation matrix of the specified scene node. It is basically the
		same as setNodeTransform but takes directly a matrix instead of individual transformation parameters.

		Parameters:
		node	- handle to the node which will be modified
		mat4x4	- pointer to a 4x4 matrix in column major order

		Returns:
		true in case of success otherwise false
		*/
		int lua_setNodeTransformMatrix( lua_State *L )
		{
			if(lua_gettop(L)!=17)
			{
				luaL_error( L, "setNodeTransformMatrix requires a H3DNode and 16 float values! ");
				return 0;
			}

			float matrix[16];
			H3DNode node=(int) lua_tointeger(L,1);
			matrix[0]=static_cast<float>( lua_tonumber(L,2) );
			matrix[1]=static_cast<float>( lua_tonumber(L,3) );
			matrix[2]=static_cast<float>( lua_tonumber(L,4) );
			matrix[3]=static_cast<float>( lua_tonumber(L,5) );
			matrix[4]=static_cast<float>( lua_tonumber(L,6) );
			matrix[5]=static_cast<float>( lua_tonumber(L,7) );
			matrix[6]=static_cast<float>( lua_tonumber(L,8) );
			matrix[7]=static_cast<float>( lua_tonumber(L,9) );
			matrix[8]=static_cast<float>( lua_tonumber(L,10) );
			matrix[9]=static_cast<float>( lua_tonumber(L,11) );
			matrix[10]=static_cast<float>( lua_tonumber(L,12) );
			matrix[11]=static_cast<float>( lua_tonumber(L,13) );
			matrix[12]=static_cast<float>( lua_tonumber(L,14) );
			matrix[13]=static_cast<float>( lua_tonumber(L,15) );
			matrix[14]=static_cast<float>( lua_tonumber(L,16) );
			matrix[15]=static_cast<float>( lua_tonumber(L,17) );
			lua_pop(L, 17);

			h3dSetNodeTransMat(node, matrix);
			
			return 0;
		}

			/* 	Function: getNodeParamf
		Gets a property of a scene node.

		This function returns a specified property of the specified node.
		The property must be of the type float.

		Parameters:
		node	- handle to the node to be accessed
		param	- parameter to be accessed

		Returns:
		value of the parameter
		*/
		int  lua_getNodeParamF( lua_State *L )
		{
			H3DNode  node = luaL_checkint(L,1);
			H3DCamera::List param = (H3DCamera::List) luaL_checkint(L, 2);				
			lua_pushnumber(L,h3dGetNodeParamF(node, param, luaL_checkint( L,3 ))); 
			return 1;
		}

	/* 	Function: setNodeParamf
			Sets a property of a scene node.
		
		This function sets a specified property of the specified node to a specified value.
		The property must be of the type float.
		
		Parameters:
			node	- handle to the node to be modified
			param	- parameter to be modified
			value	- new value for the specified parameter
			
		Returns:
			 true in case of success otherwise false
	*/
		int lua_setNodeParamF( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			int param = luaL_checkint(L, 2);
			int compIdx = luaL_checkint(L, 3);
			float value = float( luaL_checknumber(L, 4) );
			h3dSetNodeParamF(node, param, compIdx, value);
			return 0;
		}

		/* 	Function: getNodeParami
		Gets a property of a scene node.

		This function returns a specified property of the specified node.
		The property must be of the type int or H3DRes.

		Parameters:
		node	- handle to the node to be accessed
		param	- parameter to be accessed

		Returns:
		value of the parameter
		*/
		int  lua_getNodeParamI( lua_State *L )
		{
			H3DNode  node = luaL_checkint(L,1);
			H3DCamera::List param = (H3DCamera::List) luaL_checkint(L, 2);							
			lua_pushinteger(L,h3dGetNodeParamI(node, param)); 
			return 1;
		}

		/* 	Function: setNodeParami
		Sets a property of a scene node.

		This function sets a specified property of the specified node to a specified value.
		The property must be of the type int or H3DRes.

		Parameters:
		node	- handle to the node to be modified
		param	- parameter to be modified
		value	- new value for the specified parameter

		Returns:
		true in case of success otherwise false
		*/
		int lua_setNodeParamI( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			int param = luaL_checkint(L, 2);
			int value = luaL_checkint(L, 3);			
			h3dSetNodeParamI(node, param, value);
			return 0;
		}
		
			
		/* 	Function: findNodes
				Finds scene nodes with the specified properties.
			
			This function loops recursively over all children of startNode and adds them to an internal list
			of results if they match the specified name and type. The result list is cleared each time this
			function is called. The function returns the number of nodes which were found and added to the list.
			
			Parameters:
				startNode	- handle to the node where the search begins
				name		- name of nodes to be searched (empty string for all nodes)
				type		- type of nodes to be searched (H3DNodeTypes::Undefined for all types)
				
			Returns:
				number of search results
		*/
		int  lua_findNodes( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			const char* name = luaL_checkstring(L, 2);
			int type = luaL_checkint(L, 3);
			lua_pushinteger(L, h3dFindNodes(node, name, type));
			return 1;
		}


		/* 	Function: getNodeFindResult
		Gets a result from the findNodes query.

		This function returns the n-th (index) result of a previous findNodes query. The result is the handle
		to a scene node with the poperties specified at the findNodes query. If the index doesn't exist in the
		result list the function returns 0.

		Parameters:
		index	- index of search result

		Returns:
		handle to scene node from findNodes query or 0 if result doesn't exist
		*/
		int  lua_getNodeFindResult( lua_State *L )
		{
			lua_pushinteger(L, h3dGetNodeFindResult(luaL_checkint(L, 1)));
			return 1;
		}

		/* 	Function: castRay
		Performs a recursive ray collision query.

		This function checks recursively if the specified ray intersects the specified node or one of its children.
		The function finds the nearest intersection relative to the ray origin and returns the handle to the
		corresponding scene node. The ray is a line segment and is specified by a starting point (the origin) and a
		finite direction vector which also defines its length. Currently this function is limited to returning
		intersections with Meshes.

		Parameters:
		node		- node at which intersection check is beginning
		ox, oy, oz	- ray origin
		dx, dy, dz	- ray direction vector also specifying ray length
		numNearest	- maximum number of results to return or 0 for all

		Returns:
		handle to nearest intersected node or 0 if no node was hit
		*/
		int  lua_castRay( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			float ox = float( luaL_checknumber(L, 2) ), oy = float( luaL_checknumber(L, 3) ), oz = float( luaL_checknumber(L, 4) );
			float dx = float( luaL_checknumber(L, 5) ), dy = float( luaL_checknumber(L, 6) ), dz = float( luaL_checknumber(L, 7) );
			lua_pushinteger(L, h3dCastRay(node, ox, oy, oz, dx, dy, dz, luaL_checkint(L, 8)));
			return 1;
		}	

		/*	Function: getCastRayResult
			Returns a result of a previous castRay query.

		This functions is used to access the results of a previous castRay query. The index is used to access
		a specific result. The intersection data is copied to the specified variables.

		Parameters:
			index			- index of result to be accessed (range: 0 to number of results returned by castRay)
			node			- handle of intersected node
			distance		- distance from ray origin to intersection point
			intersection	- coordinates of intersection point (float[3] array)

		Returns:
			true if index was valid and data could be copied, otherwise false
		*/
		int lua_getCastRayResult( lua_State *L )
		{
			int index = luaL_checkint( L, 1 );
			H3DNode node;
			float distance;
			float intersection[3];
			bool retVal = h3dGetCastRayResult( index, &node, &distance, intersection );
			lua_pushboolean( L, retVal );
			lua_pushinteger( L, node );
			lua_pushnumber( L, distance );
			lua_pushnumber( L, intersection[0] );
			lua_pushnumber( L, intersection[1] );
			lua_pushnumber( L, intersection[2] );
			return 6;
		}

	/*	Function: checkNodeVisibility
			Checks if a node is visible.

		This function checks if a specified node is visible from the perspective of a specified
		camera. The function always checks if the node is in the camera's frustum. If checkOcclusion
		is true, the function will take into account the occlusion culling information from the previous
		frame (if occlusion culling is disabled the flag is ignored). The flag calcLod determines whether the
		detail level for the node should be returned in case it is visible. The function returns -1 if the node
		is not visible, otherwise 0 (base LOD level) or the computed LOD level.

		Parameters:
			node			- node to be checked for visibility
			cameraNode		- camera node from which the visibility test is done
			checkOcclusion	- specifies if occlusion info from previous frame should be taken into account
			calcLod			- specifies if LOD level should be computed

		Returns:
			Computed LOD level or -1 if node is not visible
	*/
	int lua_checkNodeVisibility( lua_State *L )
	{
		H3DNode node = luaL_checkint( L, 1 );
		H3DNode cameraNode = luaL_checkint( L, 2 );
		bool checkOcclusion = luaL_checkint( L, 3 ) != 0;
		bool calcLod = luaL_checkint( L, 4 ) != 0;
		lua_pushinteger( L, h3dCheckNodeVisibility( node, cameraNode, checkOcclusion, calcLod ) );
		return 1;
	}

			/* Group: Group-specific scene graph functions */

	/* 	Function: addGroupNode
			Adds a Group node to the scene.
		
		This function creates a new Group node and attaches it to the specified parent node.
		
		Parameters:
			parent	- handle to parent node to which the new node will be attached
			name	- name of the node
			
		Returns:
			 handle to the created node or 0 in case of failure
	*/
		int  lua_addGroupNode( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);			
			lua_pushinteger(L,h3dAddGroupNode(parent, name)); 
			return 1;
		}

					/* Group: Model-specific scene graph functions */
	/* 	Function: addModelNode
			Adds a Model node to the scene.
		
		This function creates a new Model node and attaches it to the specified parent node.
		
		Parameters:
			parent		- handle to parent node to which the new node will be attached
			name		- name of the node
			geometryRes	- Geometry resource used by Model node
			
		Returns:
			 handle to the created node or 0 in case of failure
	*/
		int  lua_addModelNode( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);	
			H3DRes geoRes = luaL_checkint(L, 3);
			lua_pushinteger(L,h3dAddModelNode(parent, name, geoRes)); 
			return 1;
		}


					/* 	Function: setupModelAnimStage
			Configures an animation stage of a Model node.
		
		This function is used to setup the specified animation stage (channel) of the specified Model node.
        
        The function is used for animation blending. There is a fixed number of stages (by default 16) on
        which different animations can be played. The start node determines the first node (Joint or Mesh)
        to which the animation is recursively applied. If the start node is an empty string, the animation
        affects all animatable nodes (Joints and Meshes) of the model. If a NULL-handle is used for animationRes,
		the stage is cleared and the previous animation is removed.
        
        A simple way to do animation mixing is using additive animations. If a stage is configured to be
        additive  the engine calculates the difference between the current frame and the first frame in the
        animation and adds this delta to the current transformation of the joints or meshes.
		
		Parameters:
			modelNode		- handle to the Model node to be modified
			stage			- index of the animation stage to be configured
			animationRes	- handle to Animation resource (can be 0)
			startNode		- name of first node to which animation shall be applied (or empty string)
			additive		- flag indicating whether stage is additive
			
		Returns:
			 true in case of success, otherwise false
	*/
		int  lua_setupModelAnimStage( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			unsigned int stage = luaL_checkint(L, 2);
			H3DRes res = luaL_checkint(L, 3);
			int layer = luaL_checkint(L, 4);
			const char* startNode = luaL_checkstring(L, 5);
			bool additive = luaL_checkint(L, 6) != 0;
			h3dSetupModelAnimStage(node, stage, res, layer, startNode, additive);
			return 0;
		}

		/* 	Function: setModelAnimParams
			Sets the parameters of an animation stage in a Model node.
		
		This function sets the current animation time and weight for a specified stage of the specified model.
        The time corresponds to the frames of the animation and the animation is looped if the
        time is higher than the maximum number of frames in the Animation resource. The weight is used for
        animation blending and determines how much influence the stage has compared to the other active
        stages. When the sum of the weights of all stages is more than one, the animations on the lower
        stages get priority.
		
		Parameters:
			modelNode	- handle to the Model node to be modified
			stage		- index of the animation stage to be modified
			time		- new animation time/frame
			weight		- new blend weight
			
		Returns:
			 true in case of success, otherwise false
	*/
		int  lua_setModelAnimParams( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			unsigned int stage = luaL_checkint(L, 2);
			float time = static_cast<float>(luaL_checknumber(L, 3));
			float weight = static_cast<float>(luaL_checknumber(L, 4));
			h3dSetModelAnimParams(node, stage, time, weight);
			return 0;
		}

		
		/* 	Function: setModelMorpher
			Sets the weight of a morph target.
		
		This function sets the weight of a specified morph target. If the target parameter
        is an empty string the weight of all morph targets in the specified Model node is modified.
        If the specified morph target is not found the function returns false.
		
		Parameters:
			modelNode	- handle to the Model node to be modified
			target		- name of morph target
			weight		- new weight for morph target
			
		Returns:
			 true in case of success, otherwise false
	*/
		int  lua_setModelMorpher( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			const char* target = luaL_checkstring(L, 2);
			float weight = static_cast<float>(luaL_checknumber(L, 3));
			lua_pushboolean(L, h3dSetModelMorpher(node, target, weight));
			return 1;
		}


				/* Group: Mesh-specific scene graph functions */
		/* 	Function: addMeshNode
		Adds a Mesh node to the scene.

		This function creates a new Mesh node and attaches it to the specified parent node.

		Parameters:
		parent		- handle to parent node to which the new node will be attached
		name		- name of the node
		materialRes	- material resource used by Mesh node
		batchStart	- first triangle index of mesh in Geometry resource of parent Model node
		batchCount	- number of triangle indices used for drawing mesh
		vertRStart	- first vertex in Geometry resource of parent Model node
		vertREnd	- last vertex in Geometry resource of parent Model node

		Returns:
		handle to the created node or 0 in case of failure
		*/
		int  lua_addMeshNode( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);	
			H3DRes matRes  = luaL_checkint(L, 3);
			unsigned int batchStart = luaL_checkint(L, 4);	
			unsigned int batchCount = luaL_checkint(L, 5);	
			unsigned int vertRStart = luaL_checkint(L, 6);	
			unsigned int vertREnd = luaL_checkint(L, 7);	
			lua_pushinteger(L,h3dAddMeshNode(parent, name, matRes, batchStart, batchCount, vertRStart, vertREnd)); 
			return 1;
		}

/* Group: Joint-specific scene graph functions */
	/* 	Function: addJointNode
			Adds a Joint node to the scene.
		
		This function creates a new Joint node and attaches it to the specified parent node.
		
		Parameters:
			parent		- handle to parent node to which the new node will be attached
			name		- name of the node
			jointIndex	- index of joint in Geometry resource of parent Model node
			
		Returns:
			 handle to the created node or 0 in case of failure
	*/
		int  lua_addJointNode( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);	
			unsigned int jointIndex = luaL_checkint(L, 3);
			lua_pushinteger(L,h3dAddJointNode(parent, name, jointIndex)); 
			return 1;
		}

		/* Group: Light-specific scene graph functions */
	/* 	Function: addLightNode
			Adds a Light node to the scene.
		
		This function creates a new Light node and attaches it to the specified parent node.
        The direction vector of the untransformed light node is pointing along the the negative
        z-axis. The specified material resource can define uniforms and projective textures.
        Furthermore it can contain a shader for doing lighting calculations if deferred shading
        is used. If no material is required the parameter can be zero. The context names
        define which shader contexts are used when rendering shadow maps or doing light calculations for
        forward rendering configurations.
		
		Parameters:
			parent			- handle to parent node to which the new node will be attached
			name			- name of the node
			materialRes		- material resource for light configuration or 0 if not used
			lightingContext	- name of the shader context used for doing light calculations
			shadowContext	- name of the shader context used for doing shadow map rendering
			
		Returns:
			 handle to the created node or 0 in case of failure
	*/
		int  lua_addLightNode( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);	
			H3DRes materialRes  = luaL_checkint(L, 3);
			const char* lightingContext  = luaL_checkstring(L, 4);	
			const char* shadowContext  = luaL_checkstring(L, 5);	
			lua_pushinteger(L,h3dAddLightNode(parent, name, materialRes, lightingContext, shadowContext)); 
			return 1;
		}

	/* Group: Camera-specific scene graph functions */
	/* 	Function: addCameraNode
			Adds a Camera node to the scene.
		
		This function creates a new Camera node and attaches it to the specified parent node.
		
		Parameters:
			parent		- handle to parent node to which the new node will be attached
			name		- name of the node
			pipelineRes	- pipeline resource used for rendering
			
		Returns:
			 handle to the created node or 0 in case of failure
	*/
		int  lua_addCameraNode( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);			
			lua_pushinteger(L,h3dAddCameraNode(parent, name, luaL_checkint(L, 3))); 
			return 1;
		}

	/* 	Function: setupCameraView
			Sets the planes of a camera viewing frustum.
		
		This function calculates the view frustum planes of the specified camera node using the specified view
        parameters.
		
		Parameters:
			cameraNode	- handle to the Camera node which will be modified
			fov			- field of view (FOV) angle
			aspect		- aspect ratio
			nearDist	- distance of near clipping plane
			farDist		- distance of far clipping plane 
			
		Returns:
			 true in case of success otherwise false
	*/
		int  lua_setupCameraView( lua_State *L )
		{
			H3DNode node = luaL_checkint(L, 1);
			float fov = static_cast<float>(luaL_checknumber(L, 2));
			float aspect = static_cast<float>(luaL_checknumber(L, 3));
			float near = static_cast<float>(luaL_checknumber(L, 4));
			float far = static_cast<float>(luaL_checknumber(L, 5));			
			h3dSetupCameraView(node, fov, aspect, near, far);
			return 0;
		}

	/* 	Function: getCameraProjectionMatrix
			Gets the camera projection matrix.
		
		This function gets the camera projection matrix used for bringing the geometry to
        screen space and copies it to the specified array.
		
		Parameters:
			cameraNode	- handle to Camera node
			projMat		- pointer to float array with 16 elements
			
		Returns:
			 true in case of success otherwise false
	*/
		int  lua_getCameraProjectionMatrix( lua_State *L )
		{
			if(lua_gettop(L)!=1) return 0;

			H3DNode camera=(H3DNode) lua_tointeger(L, 1);
			lua_pop(L, 1);

			float matrix[16] = {0};
			h3dGetCameraProjMat(camera, matrix);
			
			lua_pushnumber( L,matrix[0] );
			lua_pushnumber( L,matrix[1] );
			lua_pushnumber( L,matrix[2] );
			lua_pushnumber( L,matrix[3] );
			lua_pushnumber( L,matrix[4] );
			lua_pushnumber( L,matrix[5] );
			lua_pushnumber( L,matrix[6] );
			lua_pushnumber( L,matrix[7] );
			lua_pushnumber( L,matrix[8] );
			lua_pushnumber( L,matrix[9] );
			lua_pushnumber( L,matrix[10] );
			lua_pushnumber( L,matrix[11] );
			lua_pushnumber( L,matrix[12] );
			lua_pushnumber( L,matrix[13] );
			lua_pushnumber( L,matrix[14] );
			lua_pushnumber( L,matrix[15] );

			return 16;
		}

		/* Group: Emitter-specific scene graph functions */
	/* 	Function: addEmitterNode
			Adds a Emitter node to the scene.
		
		This function creates a new Emitter node and attaches it to the specified parent node.
		
		Parameters:
			parent				- handle to parent node to which the new node will be attached
			name				- name of the node
			materialRes			- handle to Material resource used for rendering
			effectRes			- handle to Effect resource used for configuring particle properties
			maxParticleCount	- maximal number of particles living at the same time
			respawnCount		- number of times a single particle is recreated after dying (-1 for infinite)
			
			
		Returns:
			 handle to the created node or 0 in case of failure
	*/
		int  lua_addEmitterNode( lua_State *L )
		{
			H3DNode parent = luaL_checkint(L,1);
			const char* name  = luaL_checkstring(L, 2);			
			H3DRes matRes  = luaL_checkint(L, 3);
			H3DRes effectRes = luaL_checkint(L, 4);
			unsigned int maxParticleCount = luaL_checkint(L, 5);
			unsigned int respawnCount = luaL_checkint(L, 6);
			lua_pushinteger(L,h3dAddEmitterNode(parent, name, matRes, effectRes, maxParticleCount, respawnCount)); 
			return 1;
		}

		//Advances the time value of an Emitter node.
		//
		//This function advances the simulation time of a particle system and continues the particle simulation with timeDelta being the time elapsed since the last call of this function.
		//
		//Parameters:
		//    	node  	handle to the Emitter node which will be modified
		//    	timeDelta  	time delta in seconds
		//
		//Returns:
		//    true in case of success, otherwise false 
		int  lua_updateEmitter( lua_State *L )
		{
			H3DNode  node  = luaL_checkint(L,1);
			float timeDelta = static_cast<float>(luaL_checknumber(L, 2));	
			h3dUpdateEmitter(node, timeDelta); 
			return 0;
		}

		//Advances the time value of an Emitter node.
		//
		//This function advances the simulation time of a particle system and continues the particle simulation with timeDelta being the time elapsed since the last call of this function.
		//
		//Parameters:
		//    	node  	handle to the Emitter node which will be modified
		//    	timeDelta  	time delta in seconds
		//
		//Returns:
		//    true in case of success, otherwise false 
		int  lua_updateModel( lua_State *L )
		{
			H3DNode  node  = luaL_checkint(L,1);
			int flags = luaL_checkint( L, 2 );
			h3dUpdateModel(node, flags); 
			return 0;
		}
			//Gets the search path of a resource type.
		//
		//This function returns the search path of a specified resource type.
		//
		//Parameters:
		//    	type  	type of resource
		//
		//Returns:
		//    pointer to the search path string 
		//

		int  lua_getResourcePath( lua_State *L )
		{
			H3DResTypes::List type = (H3DResTypes::List) luaL_checkint(L, 1);			
			lua_pushstring(L, h3dutGetResourcePath(type));
			return 1;
		}
		
		//Sets the search path for a resource type.
		//
		//This function sets the search path for a specified resource type. Whenever a new resource is added, the specified path is concatenated to the name of the created resource.
		//
		//Parameters:
		//    	type  	type of resource
		//    	path  	path where the resources can be found (without slash or backslash at the end)
		//
		//Returns:
		//    nothing 
		//
		int  lua_setResourcePath( lua_State *L )
		{
			H3DResTypes::List type = (H3DResTypes::List) luaL_checkint(L, 1);
			const char* path = luaL_checkstring(L, 2);
			h3dutSetResourcePath(type, path);
			return 0;
		}


		int lua_loadResourcesFromDisk( lua_State *L )
		{			
			const char* path = luaL_checkstring(L, 1);
			h3dutLoadResourcesFromDisk( path );
			return 0;
		}

        static const luaL_Reg horde3d_meta_methods[] = {
			{0,0}
		};

        static const luaL_Reg horde3d_utils_meta_methods[] = {
			{0,0}
		};


        const luaL_Reg horde3d_methods[] = {
			{"getVersionString", lua_getVersionString},
			{"checkExtension", lua_checkExtension},
			{"init", lua_init},
			{"release", lua_release},
			{"resizePipelineBuffers", lua_ResizePipelineBuffers},
			{"render", lua_render},
			{"clear", lua_clear},
			{"getMessage", lua_getMessage},
			{"getOption", lua_getOption},
			{"setOption", lua_setOption},
			{"showOverlays", lua_showOverlays},			
			{"clearOverlays", lua_clearOverlays},
			{"getResType", lua_getResType},			
			{"findResource", lua_findResource},
			{"addResource", lua_addResource},
			{"cloneResource", lua_cloneResource},
			{"removeResource", lua_removeResource},
			{"loadResource", lua_loadResource},
			{"unloadResource", lua_unloadResource},
			{"getResParamI", lua_getResParamI},
			{"setResParamI", lua_setResParamI},
			{"getResParamF", lua_getResParamF},
			{"setResParamF", lua_setResParamF},
			{"getResParamStr", lua_getResParamStr},
			{"setResParamStr", lua_setResParamStr},
			{"getResourceData", lua_getResourceData},
			{"updateResourceData", lua_updateResourceData},
			{"queryUnloadedResource", lua_queryUnloadedResource},
			{"releaseUnusedResources", lua_releasedUnusedResources},
			{"createTexture2D", lua_createTexture2D},
			{"setShaderPreambles", lua_setShaderPreambles},
			{"setMaterialUniform", lua_setMaterialUniform},
			{"getPipelineRenderTargetData", lua_getPipelineRenderTargetData},
			{"getNodeType", lua_getNodeType},
			{"getNodeParamstr", lua_getNodeParamStr},
			{"setNodeParamstr", lua_setNodeParamStr},
			{"getNodeParent", lua_getNodeParent},
			{"setNodeParent", lua_setNodeParent},
			{"getNodeChild", lua_getNodeChild},
			{"addNodes", lua_addNodes},
			{"removeNode", lua_removeNode},
			{"getNodeFlags", lua_getNodeFlags},
			{"setNodeFlags", lua_setNodeFlags},
			{"checkNodeTransformFlag", lua_checkNodeTransformFlag},
			{"getNodeTransform", lua_getNodeTransform},			
			{"setNodeTransform", lua_setNodeTransform},
			{"getNodeTransformMatrices", lua_getNodeTransformMatrices},
			{"setNodeTransformMatrix", lua_setNodeTransformMatrix},
			{"getNodeParamf", lua_getNodeParamF},
			{"setNodeParamf", lua_setNodeParamF},
			{"getNodeParami", lua_getNodeParamI},
			{"setNodeParami", lua_setNodeParamI},
			{"getNodeAABB", lua_getNodeAABB},
			{"findNodes", lua_findNodes},
			{"getNodeFindResult", lua_getNodeFindResult},
			{"castRay", lua_castRay},
			{"getCastRayResult", lua_getCastRayResult},
			{"checkNodeVisibility", lua_checkNodeVisibility},
			{"addGroupNode", lua_addGroupNode},
			{"addModelNode", lua_addModelNode},
			{"setupModelAnimStage", lua_setupModelAnimStage},
			{"setModelAnimParams", lua_setModelAnimParams},
			{"setModelMorpher", lua_setModelMorpher},
			{"addMeshNode", lua_addMeshNode},
			{"addJointNode", lua_addJointNode},
			{"addLightNode", lua_addLightNode},			
			{"addCameraNode", lua_addCameraNode},
			{"setupCameraView", lua_setupCameraView},
			{"getCameraProjectionMatrix", lua_getCameraProjectionMatrix},
			{"addEmitterNode", lua_addEmitterNode},
			{"updateEmitter", lua_updateEmitter},
			{"updateModel", lua_updateModel},
			{0,0}
		};

        const luaL_Reg horde3d_utils_methods[] = {
			// Horde3D Utils
			{"getResourcePath", lua_getResourcePath},
			{"setResourcePath", lua_setResourcePath},
			{"loadResourcesFromDisk", lua_loadResourcesFromDisk},
			{0,0}
		};

	}

#define newtable(L) (lua_newtable(L), lua_gettop(L))

	void registerLuaBindings (lua_State *L)
	{
#if LUA_VERSION_NUM < 502
		int metatable, methods;

		lua_pushliteral(L, "Horde3D");         /* name of function table */
		methods   = newtable(L);           /* function methods table */
		metatable = newtable(L);           /* function metatable */
		lua_pushliteral(L, "__index");     /* add index event to metatable */
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);        /* metatable.__index = methods */
		lua_pushliteral(L, "__metatable"); /* hide metatable */
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);        /* metatable.__metatable = methods */
		luaL_openlib(L, 0, LuaBindings::horde3d_meta_methods,  0); /* fill metatable */
		luaL_openlib(L, 0, LuaBindings::horde3d_methods, 1); /* fill function methods table */
		lua_settable(L, LUA_GLOBALSINDEX); /* add function to globals */
	
		lua_pushliteral(L, "Horde3DUtils");         /* name of function table */
		methods   = newtable(L);           /* function methods table */
		metatable = newtable(L);           /* function metatable */
		lua_pushliteral(L, "__index");     /* add index event to metatable */
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);        /* metatable.__index = methods */
		lua_pushliteral(L, "__metatable"); /* hide metatable */
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);        /* metatable.__metatable = methods */
		luaL_openlib(L, 0, LuaBindings::horde3d_meta_methods,  0); /* fill metatable */
		luaL_openlib(L, 0, LuaBindings::horde3d_utils_methods, 1); /* fill function methods table */
		lua_settable(L, LUA_GLOBALSINDEX); /* add function to globals */
#else      
        luaL_newlib(L,LuaBindings::horde3d_methods);
        lua_setglobal(L, "Horde3D");
        luaL_newlib(L,LuaBindings::horde3d_utils_methods);
        lua_setglobal(L, "Horde3DUtils");
#endif
	}


}
