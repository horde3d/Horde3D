// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2017 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#include "overlays.h"

#include "egModules.h"
#include "egRenderer.h"
#include "egRendererBase.h"
#include "egCamera.h"
#include "utXML.h"

namespace Horde3DOverlays {
	
using namespace std;
using namespace Horde3D;

const uint32 MaxNumOverlayVerts = 2048;
const uint32 QuadIdxBufCount = MaxNumOverlayVerts * 6;

uint32 OverlayRenderer::_overlayGeo = 0;
uint32 OverlayRenderer::_overlayVB = 0;
int OverlayRenderer::_uni_olayColor = -1;
int OverlayRenderer::_vlOverlay = -1;

std::vector< CachedUniformLocation > OverlayRenderer::_cachedLocations;

std::vector< OverlayBatch > OverlayRenderer::_overlayBatches = {};
OverlayVert *OverlayRenderer::_overlayVerts = nullptr;

const char * OverlayRenderer::parsePipelineCommandFunc( const char *commandName, void *xmlNodeParams, PipelineCommand &cmd )
{
	XMLNode *node = ( XMLNode * ) xmlNodeParams;

	if ( strcmp( node->getName(), "DrawOverlays" ) == 0 )
	{
		if ( !node->getAttribute( "context" ) ) return "Missing DrawOverlays attribute 'context'";

		vector< PipeCmdParam > &params = cmd.params;
		params.resize( 1 );
		params[ 0 ].setString( node->getAttribute( "context" ) );
	}

	return "";
}


void OverlayRenderer::executePipelineCommandFunc( const PipelineCommand *commandParams )
{
	drawOverlays( commandParams->params[ 0 ].getString() );
}


bool OverlayRenderer::init()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// Create vertex layout
	VertexLayoutAttrib attribsOverlay[ 2 ] = {
		{ "vertPos", 0, 2, 0 },
		{ "texCoords0", 0, 2, 8 }
	};
	_vlOverlay = Modules::renderer().getRenderDevice()->registerVertexLayout( 2, attribsOverlay );

	// Create index buffer used for drawing overlay quads
	uint16 *quadIndices = new uint16[ QuadIdxBufCount ];
	for ( uint32 i = 0; i < QuadIdxBufCount / 6; ++i )
	{
		quadIndices[ i * 6 + 0 ] = i * 4 + 0; quadIndices[ i * 6 + 1 ] = i * 4 + 1; quadIndices[ i * 6 + 2 ] = i * 4 + 2;
		quadIndices[ i * 6 + 3 ] = i * 4 + 2; quadIndices[ i * 6 + 4 ] = i * 4 + 3; quadIndices[ i * 6 + 5 ] = i * 4 + 0;
	}
	
	int quadIdxBuf = rdi->createIndexBuffer( QuadIdxBufCount * sizeof( uint16 ), quadIndices );
	delete[] quadIndices; quadIndices = 0x0;

	_overlayBatches.reserve( 64 );
	_overlayVerts = new OverlayVert[ MaxNumOverlayVerts ];
	_overlayVB = rdi->createVertexBuffer( MaxNumOverlayVerts * sizeof( OverlayVert ), 0x0 );

	// Create geometry bindings
	_overlayGeo = rdi->beginCreatingGeometry( _vlOverlay );

	rdi->setGeomVertexParams( _overlayGeo, _overlayVB, 0, 0, sizeof( OverlayVert ) );
	rdi->setGeomIndexParams( _overlayGeo, quadIdxBuf, IDXFMT_16 );

	rdi->finishCreatingGeometry( _overlayGeo );

	_cachedLocations.reserve( 16 );

	// Register new pipeline commands
	ExternalPipelineCommandsManager::registerPipelineCommand( "DrawOverlays", OverlayRenderer::parsePipelineCommandFunc, 
																			  OverlayRenderer::executePipelineCommandFunc );
	return true;
}


void OverlayRenderer::release()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	rdi->destroyGeometry( _overlayGeo );
	delete[] _overlayVerts;

	_overlayBatches.clear();
	_overlayVB = 0;
	_vlOverlay = -1;
}


void OverlayRenderer::showOverlays( const float *verts, uint32 vertCount, float *colRGBA,
	MaterialResource *matRes, int flags )
{
	uint32 numOverlayVerts = 0;
	if ( !_overlayBatches.empty() )
		numOverlayVerts = _overlayBatches.back().firstVert + _overlayBatches.back().vertCount;

	if ( numOverlayVerts + vertCount > MaxNumOverlayVerts ) return;

	memcpy( &_overlayVerts[ numOverlayVerts ], verts, vertCount * sizeof( OverlayVert ) );

	// Check if previous batch can be extended
	if ( !_overlayBatches.empty() )
	{
		OverlayBatch &prevBatch = _overlayBatches.back();
		if ( matRes == prevBatch.materialRes && flags == prevBatch.flags &&
			memcmp( colRGBA, prevBatch.colRGBA, 4 * sizeof( float ) ) == 0 )
		{
			prevBatch.vertCount += vertCount;
			return;
		}
	}

	// Create new batch
	_overlayBatches.push_back( OverlayBatch( numOverlayVerts, vertCount, colRGBA, matRes, flags ) );
}


void OverlayRenderer::clearOverlays()
{
	_overlayBatches.resize( 0 );
}


void OverlayRenderer::drawOverlays( const string &shaderContext )
{
	uint32 numOverlayVerts = 0;
	if ( !_overlayBatches.empty() )
		numOverlayVerts = _overlayBatches.back().firstVert + _overlayBatches.back().vertCount;

	if ( numOverlayVerts == 0 ) return;

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
	CameraNode *curCamera = Modules::renderer().getCurCamera();
	if ( curCamera == 0x0 ) return;

	// Upload overlay vertices
	rdi->updateBufferData( _overlayGeo, _overlayVB, 0, MaxNumOverlayVerts * sizeof( OverlayVert ), _overlayVerts );

	rdi->setGeometry( _overlayGeo );
	ASSERT( QuadIdxBufCount >= MaxNumOverlayVerts * 6 );

	float aspect = ( float ) curCamera->getViewportWidth() / ( float ) curCamera->getViewportHeight();
	Modules::renderer().setupViewMatrices( Matrix4f(), Matrix4f::OrthoMat( 0, aspect, 1, 0, -1, 1 ) );

	MaterialResource *curMatRes = 0x0;

	for ( size_t i = 0, s = _overlayBatches.size(); i < s; ++i )
	{
		OverlayBatch &ob = _overlayBatches[ i ];

		if ( curMatRes != ob.materialRes )
		{
			if ( !Modules::renderer().setMaterial( ob.materialRes, shaderContext ) )
			{
				// Unsuccessful material setting probably has destroyed the last setted material
				curMatRes = 0x0;
				continue;
			}

			curMatRes = ob.materialRes;
			_uni_olayColor = getInternalUniformLocation( ob.materialRes );
		}

		if ( _uni_olayColor >= 0 )
			rdi->setShaderConst( _uni_olayColor, CONST_FLOAT4, ob.colRGBA );

		// Draw batch
		rdi->drawIndexed( PRIM_TRILIST, ob.firstVert * 6 / 4, ob.vertCount * 6 / 4, ob.firstVert, ob.vertCount );
	}
}


int OverlayRenderer::getInternalUniformLocation(MaterialResource *mat)
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
	ShaderCombination *curShader = Modules::renderer().getCurShader();

	for ( size_t i = 0; i < _cachedLocations.size(); ++i )
	{
		if ( _cachedLocations[ i ].material == mat )
		{
			return _cachedLocations[ i ].uniformLocation;
		}
	}

	// create new entry
	_cachedLocations.push_back( CachedUniformLocation( mat, rdi->getShaderConstLoc( curShader->shaderObj, "olayColor" ) ) );
}

} // namespace