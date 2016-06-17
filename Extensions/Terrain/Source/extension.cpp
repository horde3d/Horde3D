// *************************************************************************************************
//
// Horde3D Terrain Extension
// --------------------------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz, Volker Wiendl and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#include "utPlatform.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include "extension.h"
#include "terrain.h"

#include "utDebug.h"


// Internal extension interface
namespace Horde3DTerrain {

using namespace Horde3D;

bool ExtTerrain::init()
{
	Modules::sceneMan().registerNodeType( SNT_TerrainNode, "Terrain",
		TerrainNode::parsingFunc, TerrainNode::factoryFunc );
	Modules::renderer().registerRenderFunc( SNT_TerrainNode, TerrainNode::renderFunc );

	// Create vertex layout
	VertexLayoutAttrib attribs[2] = {
		{"vertPos", 0, 3, 0},
		{"terHeight", 1, 1, 0}
	};
	TerrainNode::vlTerrain = Modules::renderer().getRenderDevice()->registerVertexLayout( 2, attribs );

	// Upload default shader used for debug view
	if ( Modules::renderer().getRenderDeviceType() == RenderBackendType::OpenGL2 )
	{
		Modules::renderer().createShaderComb( TerrainNode::debugViewShader,
											  vsTerrainDebugView, fsTerrainDebugView, nullptr, nullptr, nullptr, nullptr );
	} 
	else
	{
		Modules::renderer().createShaderComb( TerrainNode::debugViewShader,
											  vsTerrainDebugViewGL4, fsTerrainDebugViewGL4, nullptr, nullptr, nullptr, nullptr );
	}
	
	return true;
}

void ExtTerrain::release()
{
	Modules::renderer().releaseShaderComb( TerrainNode::debugViewShader );
}

}  // namespace


// Public C API
namespace Horde3DTerrain {

std::string safeStr( const char *str )
{
	if( str != 0x0 ) return str;
	else return "";
}


DLLEXP NodeHandle h3dextAddTerrainNode( NodeHandle parent, const char *name, ResHandle heightMapRes,
                                        ResHandle materialRes )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	if( parentNode == 0x0 ) return 0;
	
	Resource *hmapRes = Modules::resMan().resolveResHandle( heightMapRes );
	if( hmapRes == 0x0 || hmapRes->getType() != ResourceTypes::Texture ||
		((TextureResource *)hmapRes)->getTexType() != TextureTypes::Tex2D ) return 0;
	
	Resource *matRes =  Modules::resMan().resolveResHandle( materialRes );
	if( matRes == 0x0 || matRes->getType() != ResourceTypes::Material ) return 0;
	
	Modules::log().writeInfo( "Adding Terrain node '%s'", safeStr( name ).c_str() );
	
	TerrainNodeTpl tpl( safeStr( name ), (TextureResource *)hmapRes, (MaterialResource *)matRes );
	SceneNode *sn = Modules::sceneMan().findType( SNT_TerrainNode )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP ResHandle h3dextCreateTerrainGeoRes( NodeHandle node, const char *name, float meshQuality )
{	
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	if( sn != 0x0 && sn->getType() == SNT_TerrainNode )
		return ((TerrainNode *)sn)->createGeometryResource( safeStr( name ), 1.0f / meshQuality );
	else
		return 0;
}

}  // namespace
