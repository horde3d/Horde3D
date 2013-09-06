// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _egModules_H_
#define _egModules_H_

#include "egPrerequisites.h"


namespace Horde3D {

// Forward declarations
class EngineConfig;
class EngineLog;
class StatManager;
class SceneManager;
class ResourceManager;
class RenderDevice;
class Renderer;
class ExtensionManager;


// =================================================================================================
// Modules
// =================================================================================================

class Modules
{
public:
	static bool init();
	static void release();

	static void setError( const char *errorStr1 = 0x0, const char *errorStr2 = 0x0 );
	static bool getError();
	
	static EngineConfig &config() { return *_engineConfig; }
	static EngineLog &log() { return *_engineLog; }
	static StatManager &stats() { return *_statManager; }
	static SceneManager &sceneMan() { return *_sceneManager; }
	static ResourceManager &resMan() { return *_resourceManager; }
	static Renderer &renderer() { return *_renderer; }
	static ExtensionManager &extMan() { return *_extensionManager; }

public:
	static const char *versionString;

private:
	static void installExtensions();

private:
	static bool                   _errorFlag;

	static EngineConfig           *_engineConfig;
	static EngineLog              *_engineLog;
	static StatManager            *_statManager;
	static SceneManager           *_sceneManager;
	static ResourceManager        *_resourceManager;
	static RenderDevice           *_renderDevice;
	static Renderer               *_renderer;
	static ExtensionManager       *_extensionManager;
};

extern RenderDevice  *gRDI;


// =================================================================================================
// Useful macros for API functions
// =================================================================================================

#ifdef H3D_API_VALIDATION
	#define APIFUNC_VALIDATE_RES( res, func, retVal ) if( res == 0x0 ) { \
		Modules::setError( "Invalid resource handle in ", func ); return retVal; }
	#define APIFUNC_VALIDATE_RES_TYPE( res, type, func, retVal ) if( res == 0x0 || res->getType() != type ) { \
		Modules::setError( "Invalid resource handle in ", func ); return retVal; }
	#define APIFUNC_VALIDATE_NODE( node, func, retVal ) if( node == 0x0 ) { \
		Modules::setError( "Invalid node handle in ", func ); return retVal; }
	#define APIFUNC_VALIDATE_NODE_TYPE( node, type, func, retVal ) if( node == 0x0 || node->getType() != type ) { \
		Modules::setError( "Invalid node handle in ", func ); return retVal; }
	#define APIFUNC_RET_VOID
#else
	#define APIFUNC_VALIDATE_RES( res, func, retVal )
	#define APIFUNC_VALIDATE_RES_TYPE( res, type, func, retVal )
	#define APIFUNC_VALIDATE_NODE( node, func, retVal )
	#define APIFUNC_VALIDATE_NODE_TYPE( node, type, func, retVal )
	#define APIFUNC_RET_VOID
#endif

}
#endif // _egModules_H_
