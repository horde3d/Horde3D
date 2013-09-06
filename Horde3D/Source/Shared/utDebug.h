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

// This file should be included as the last include to all cpp files
// If in Debug Mode, call _CrtDumpMemoryLeaks() at the end of the application

#include "utPlatform.h"


#if defined( PLATFORM_WIN ) && defined( _MSC_VER ) && defined( _DEBUG )

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#define MYDEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the allocations to be of _CLIENT_BLOCK type
// Compile options needed: /Zi /D_DEBUG /MLd

#define new MYDEBUG_NEW

#endif
