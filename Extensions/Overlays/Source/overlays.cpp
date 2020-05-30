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
#include "egCom.h"

#include <sstream>
#include <iomanip>

namespace Horde3DOverlays {
	
using namespace std;
using namespace Horde3D;

const uint32 MaxNumOverlayVerts = 8192;
const uint32 QuadIdxBufCount = MaxNumOverlayVerts * 6;

uint32 OverlayRenderer::_overlayGeo = 0;
uint32 OverlayRenderer::_overlayVB = 0;
int OverlayRenderer::_uni_olayColor = -1;
int OverlayRenderer::_vlOverlay = -1;
InfoBox OverlayRenderer::_infoBox;

//std::vector< CachedUniformLocation > OverlayRenderer::_cachedLocations;

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

//	_cachedLocations.reserve( 16 );

	// Register new engine uniform that will be searched for in all loaded shaders
	_uni_olayColor = Modules::renderer().registerEngineUniform( "olayColor" );

	// Register new pipeline commands
	Modules::pipeMan().registerPipelineCommand( "DrawOverlays", OverlayRenderer::parsePipelineCommandFunc, 
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
	ShaderCombination *curShader = 0x0;

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
			curShader = Modules::renderer().getCurShader();
//			_uni_olayColor = getInternalUniformLocation( ob.materialRes );
		}

		if ( curShader->uniLocs[ _uni_olayColor ] >= 0 )
			rdi->setShaderConst( curShader->uniLocs[ _uni_olayColor ], CONST_FLOAT4, ob.colRGBA );

		// Draw batch
		rdi->drawIndexed( PRIM_TRILIST, ob.firstVert * 6 / 4, ob.vertCount * 6 / 4, ob.firstVert, ob.vertCount );
	}
}


void OverlayRenderer::showText( const char *text, float x, float y, float size, 
								float colR, float colG, float colB, Horde3D::MaterialResource *fontMatRes )
{
	if ( text == 0x0 || *text == '\0' ) return;

	float ovFontVerts[ 64 * 16 ];
	float *p = ovFontVerts;
	float pos = 0;

	do
	{
		unsigned char ch = ( unsigned char ) *text++;

		float u0 = 0.0625f * ( ch % 16 );
		float v0 = 1.0f - 0.0625f * ( ch / 16 );

		*p++ = x + size * 0.5f * pos;         *p++ = y;         *p++ = u0;            *p++ = v0;
		*p++ = x + size * 0.5f * pos, *p++ = y + size;  *p++ = u0;            *p++ = v0 - 0.0625f;
		*p++ = x + size * 0.5f * pos + size;  *p++ = y + size;  *p++ = u0 + 0.0625f;  *p++ = v0 - 0.0625f;
		*p++ = x + size * 0.5f * pos + size;  *p++ = y;         *p++ = u0 + 0.0625f;  *p++ = v0;

		pos += 1.f;
	} while ( *text && pos < 64 );

	float color[ 4 ] = { colR, colG, colB, 1.f };
	showOverlays( ovFontVerts, ( int ) pos * 4, color, fontMatRes, 0 );
}

void OverlayRenderer::beginInfoBox( float x, float y, float width, int numRows, const char *title,
									Horde3D::MaterialResource *fontMatRes, Horde3D::MaterialResource *boxMatRes )
{
	float fontSize = 0.03f;
	float barHeight = fontSize + 0.01f;
	float bodyHeight = numRows * 0.035f + 0.005f;
	float titleColor[ 4 ] = { 0.15f, 0.23f, 0.31f, 0.8f };
	float bodyColor[ 4 ] = { 0.12f, 0.12f, 0.12f, 0.5f };

	_infoBox.fontMatRes = fontMatRes;
	_infoBox.x = x;
	_infoBox.y_row0 = y + barHeight + 0.005f;
	_infoBox.width = width;
	_infoBox.row = 0;

	// Title bar
	float ovTitleVerts[] = { x, y, 0, 1, x, y + barHeight, 0, 0,
		x + width, y + barHeight, 1, 0, x + width, y, 1, 1 };
	showOverlays( ovTitleVerts, 4, titleColor, boxMatRes, 0 );

	// Title text
	showText( title, x + 0.005f, y + 0.005f, fontSize, 0.7f, 0.85f, 0.95f, fontMatRes );

	// Body
	float yy = y + barHeight;
	float ovBodyVerts[] = { x, yy, 0, 1, x, yy + bodyHeight, 0, 0,
		x + width, yy + bodyHeight, 1, 0, x + width, yy, 1, 1 };
	showOverlays( ovBodyVerts, 4, bodyColor, boxMatRes, 0 );
}


void OverlayRenderer::addInfoBoxRow( const char *column1, const char *column2 )
{
	float fontSize = 0.028f;
	float fontWidth = fontSize * 0.5f;
	float x = _infoBox.x;
	float y = _infoBox.y_row0 + _infoBox.row++ * 0.035f;

	// First column
	if ( column1 )
	{
		showText( column1, x + 0.005f, y, fontSize, 1, 1, 1, _infoBox.fontMatRes );
	}

	// Second column
	if ( column2 )
	{
		x = _infoBox.x + _infoBox.width - ( ( strlen( column2 ) - 1 ) * fontWidth + fontSize );
		showText( column2, x - 0.005f, y, fontSize, 1, 1, 1, _infoBox.fontMatRes );
	}
}


void OverlayRenderer::showFrameStats( MaterialResource *fontMatRes, MaterialResource *panelMatRes, int mode )
{
	static stringstream text;
	static float curFPS = 30;
	static float timer = 100;
	static float fps = 30;
	static float frameTime = 0;
	static float animTime = 0;
	static float geoUpdateTime = 0;
	static float particleSimTime = 0;
	static float fwdLightsGPUTime = 0;
	static float defLightsGPUTime = 0;
	static float shadowsGPUTime = 0;
	static float particleGPUTime = 0;
	static float computeGPUTime = 0;
	static float cullingTime = 0;

	// Calculate FPS
	float curFrameTime = Modules::stats().getStat( EngineStats::FrameTime, true );
	curFPS = 1000.0f / curFrameTime;

	timer += curFrameTime / 1000.0f;
	if ( timer > 0.7f )
	{
		fps = curFPS;
		frameTime = curFrameTime;
		animTime = Modules::stats().getStat( EngineStats::AnimationTime, true );
		geoUpdateTime = Modules::stats().getStat( EngineStats::GeoUpdateTime, true );
		particleSimTime = Modules::stats().getStat( EngineStats::ParticleSimTime, true );
		fwdLightsGPUTime = Modules::stats().getStat( EngineStats::FwdLightsGPUTime, true );
		defLightsGPUTime = Modules::stats().getStat( EngineStats::DefLightsGPUTime, true );
		shadowsGPUTime = Modules::stats().getStat( EngineStats::ShadowsGPUTime, true );
		particleGPUTime = Modules::stats().getStat( EngineStats::ParticleGPUTime, true );
		computeGPUTime = Modules::stats().getStat( EngineStats::ComputeGPUTime, true );
		cullingTime = Modules::stats().getStat( EngineStats::CullingTime, true );;
		timer = 0;
	}
	else
	{
		// Reset accumulative counters
		Modules::stats().getStat( EngineStats::AnimationTime, true );
		Modules::stats().getStat( EngineStats::GeoUpdateTime, true );
		Modules::stats().getStat( EngineStats::ParticleSimTime, true );
		Modules::stats().getStat( EngineStats::CullingTime, true );
	}

	if ( mode > 0 )
	{
		// InfoBox
		beginInfoBox( 0.03f, 0.03f, 0.32f, 4, "Frame Stats", fontMatRes, panelMatRes );

		// FPS
		text.str( "" );
		text << fixed << std::setprecision( 2 ) << fps;
		addInfoBoxRow( "FPS", text.str().c_str() );

		// Triangle count
		text.str( "" );
		text << ( int ) Modules::stats().getStat( EngineStats::TriCount, true );
		addInfoBoxRow( "Tris", text.str().c_str() );

		// Number of batches
		text.str( "" );
		text << ( int ) Modules::stats().getStat( EngineStats::BatchCount, true );
		addInfoBoxRow( "Batches", text.str().c_str() );

		// Number of lighting passes
		text.str( "" );
		text << ( int ) Modules::stats().getStat( EngineStats::LightPassCount, true );
		addInfoBoxRow( "Lights", text.str().c_str() );
	}

	if ( mode > 1 )
	{
		// Video memory
		beginInfoBox( 0.03f, 0.30f, 0.32f, 2, "VMem", fontMatRes, panelMatRes );

		// Textures
		text.str( "" );
		text << Modules::stats().getStat( EngineStats::TextureVMem, false ) << "mb";
		addInfoBoxRow( "Textures", text.str().c_str() );

		// Geometry
		text.str( "" );
		text << Modules::stats().getStat( EngineStats::GeometryVMem, false ) << "mb";
		addInfoBoxRow( "Geometry", text.str().c_str() );

		// CPU time
		beginInfoBox( 0.03f, 0.44f, 0.32f, 5, "CPU Time", fontMatRes, panelMatRes );

		// Frame time
		text.str( "" );
		text << frameTime << "ms";
		addInfoBoxRow( "Frame Total", text.str().c_str() );

		// Animation
		text.str( "" );
		text << animTime << "ms";
		addInfoBoxRow( "Animation", text.str().c_str() );

		// Geometry updates
		text.str( "" );
		text << geoUpdateTime << "ms";
		addInfoBoxRow( "Geo Updates", text.str().c_str() );

		// Particle simulation
		text.str( "" );
		text << particleSimTime << "ms";
		addInfoBoxRow( "Particles", text.str().c_str() );

		// Culling time
		text.str( "" );
		text << cullingTime << "ms";
		addInfoBoxRow( "Culling", text.str().c_str() );

		// GPU time
		beginInfoBox( 0.03f, 0.68f, 0.32f, 4, "GPU Time", fontMatRes, panelMatRes );

		// Forward and deferred lights
		text.str( "" );
		text << ( fwdLightsGPUTime + defLightsGPUTime ) << "ms";
		addInfoBoxRow( "Lights", text.str().c_str() );

		// Shadows
		text.str( "" );
		text << shadowsGPUTime << "ms";
		addInfoBoxRow( "Shadows", text.str().c_str() );

		// Particles
		text.str( "" );
		text << particleGPUTime << "ms";
		addInfoBoxRow( "Particles", text.str().c_str() );

		// Compute
		text.str( "" );
		text << computeGPUTime << "ms";
		addInfoBoxRow( "Compute", text.str().c_str() );
	}
}

// int OverlayRenderer::getInternalUniformLocation(MaterialResource *mat)
// {
// 	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
// 	ShaderCombination *curShader = Modules::renderer().getCurShader();
// 
// 	for ( size_t i = 0; i < _cachedLocations.size(); ++i )
// 	{
// 		if ( _cachedLocations[ i ].material == mat )
// 		{
// 			return _cachedLocations[ i ].uniformLocation;
// 		}
// 	}
// 
// 	// create new entry
// 	CachedUniformLocation loc( mat, rdi->getShaderConstLoc( curShader->shaderObj, "olayColor" ) );
// 	_cachedLocations.push_back( loc );
// 
// 	return loc.uniformLocation;
// }

} // namespace