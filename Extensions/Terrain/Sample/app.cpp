// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#include "app.h"
#include "Horde3DUtils.h"
#include "Horde3DTerrain.h"

using namespace std;


TerrainSample::TerrainSample( int argc, char** argv ) :
    SampleApplication( argc, argv )
{
    _winTitle = "Terrain - Horde3D Sample";
    _x = 512; _y = 120; _z = 512;
    _rx = 0; _ry = 225;
}


bool TerrainSample::initResources()
{
    if ( !SampleApplication::initResources() )
        return false;

    // 1. Add resources

	// Terrain
	H3DRes terrainRes = h3dAddResource( H3DResTypes::SceneGraph, "terrains/terrain1/terrain1.scene.xml", 0 );
	
    // 2. Load resources

    if ( !h3dutLoadResourcesFromDisk( _resourcePath.c_str() ) )
    {
		h3dutDumpMessages();
        return false;
    }
    
    // 3. Add scene nodes

	// Add camera
    _cam = h3dAddCameraNode( H3DRootNode, "Camera", _forwardPipeRes );
	// Add terrain
    H3DNode terrain = h3dAddNodes( H3DRootNode, terrainRes );

	// Set sun direction for ambient pass
	H3DRes matRes = h3dFindResource( H3DResTypes::Material, "terrains/terrain1/terrain1.material.xml" );
	h3dSetMaterialUniform( matRes, "sunDir", 1, -1, 0, 0 );

	return true;
}
