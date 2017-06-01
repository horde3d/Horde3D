// *************************************************************************************************
//
// Horde3D ExternalTexture Extension
// --------------------------------------------------------
// Copyright (C) 2017 Volker Vogelhuber
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

/*	Title: Horde3D ExternalTexture Extension */

#pragma once

#include "Horde3D.h"

#ifndef DLL
#	if defined( WIN32 ) || defined( _WINDOWS )
#		define DLL extern "C" __declspec( dllimport )
#	else
#		if defined( __GNUC__ ) && __GNUC__ >= 4
#		  define DLLEXP extern "C" __attribute__ ((visibility("default")))
#   	else
#		  define DLLEXP extern "C"
#   	endif
#	endif
#endif


/*	Topic: Introduction
                Some words about the ExternalTexture Extension.
	

*/

/* Function: h3dextImportTexture
                Binds an existing OpenGL Texture name to a Horde3D texture
	
	Details:
                Calling this function will bind an existing OpenGL texture to a Horde3D texture. Any previously bound
                OpenGL texture name will be released, but texture properties will remain. So keep in mind
                that you allocate the texture resource within Horde3D already with the right properties before
                replacing the texture ID with an external one. External textures are not released (no glDeleteTexture call). 
                They have to be released by the creator. 
	
	Parameters:
                texRes       - handle to an existing Horde3D texture resource
                texID        - OpenGL texture name of an existing OpenGL texture                

	Returns:
                 true in case of success, false otherwise
*/

DLL bool h3dextImportTexture( H3DRes texRes, int texID );
