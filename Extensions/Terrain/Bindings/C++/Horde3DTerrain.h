// *************************************************************************************************
//
// Horde3D Terrain Extension
// --------------------------------------------------------
// Copyright (C) 2006-2009 Nicolas Schulz and Volker Wiendl
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

/*	Title: Horde3D Terrain Extension */

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
		Some words about the Terrain Extension.
	
	The Terrain Extension extends Horde3D with the capability to render large landscapes. A special
	level of detail algorithm adapts the resolution of the terrain mesh so that near regions get
	more details than remote ones. The algorithm also considers the geometric complexity of the
	terrain to increase the resoultion solely where this is really required. This makes the
	rendering fast and provides a high quality with a minimum of popping artifacts.
	
	A height map is used to define the altitude of the terrain. The height map is a usual texture
	map that encodes 16 bit height information in two channels. The red channel of the texture
	contains the coarse height, while the green channel encodes finer graduations. The encoding of
	the information is usually done with an appropriate tool. If you just want to use 8 bit
	height information, you can simply copy the greyscale image to the red channel of the height
	map and leave the green channel black.

	The extension defines the uniform *terBlockParams* and the attribute *terHeight* that can be used
	in a shader to render the terrain. To see how this is working in detail, have a look at the included
	sample shader.
*/


/*
	Constants: Predefined constants
	H3DEXT_NodeType_Terrain  - Type identifier of Terrain scene node
*/
const int H3DEXT_NodeType_Terrain = 100;


struct H3DEXTTerrain
{
	/*	Enum: H3DEXTTerrain
			The available Terrain node parameters.
		
		HeightTexResI  - Height map texture; must be square and a power of two [write-only]
		MatResI        - Material resource used for rendering the terrain
		MeshQualityF   - Constant controlling the overall resolution of the terrain mesh (default: 50.0)
		SkirtHeightF   - Height of the skirts used to hide cracks (default: 0.1)
		BlockSizeI     - Size of a terrain block that is drawn in a single render call; must be 2^n+1 (default: 17)
	*/
	enum List
	{
		HeightTexResI = 10000,
		MatResI,
		MeshQualityF,
		SkirtHeightF,
		BlockSizeI
	};
};


/* Function: h3dextAddTerrainNode
		Adds a Terrain node to the scene.
	
	Details:
		This function creates a new Terrain node and attaches it to the specified parent node.
	
	Parameters:
		parent        - handle to parent node to which the new node will be attached
		name          - name of the node
		heightMapRes  - handle to a 2D Texture resource that contains the terrain height information (must be square and POT) 
		materialRes   - handle to the Material resource used for rendering the terrain

	Returns:
		 handle to the created node or 0 in case of failure
*/
DLL H3DNode h3dextAddTerrainNode( H3DNode parent, const char *name, H3DRes heightMapRes, H3DRes materialRes );


/* Function: h3dextCreateTerrainGeoRes
		Creates a Geometry resource from a specified Terrain node.
			
	Details:
		This function creates a new Geometry resource that contains the vertex data of the specified Terrain node.
		To reduce the amount of data, it is possible to specify a quality value which controls the overall resolution
		of the terrain mesh. The algorithm will automatically create a higher resoultion in regions where the
		geometrical complexity is higher and optimize the vertex count for flat regions.
	
	Parameters:
		node         - handle to terrain node that will be accessed
		resName      - name of the Geometry resource that shall be created
		meshQuality  - constant controlling the overall mesh resolution
		
	Returns:
		 handle to the created Geometry resource or 0 in case of failure
*/
DLL H3DRes h3dextCreateTerrainGeoRes( H3DNode node, const char *resName, float meshQuality );
