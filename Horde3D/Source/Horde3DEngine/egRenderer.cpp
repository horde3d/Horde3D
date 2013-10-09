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

#include "egRenderer.h"
#include "egParticle.h"
#include "egLight.h"
#include "egCamera.h"
#include "egModules.h"
#include "egCom.h"
#include <cstring>

#include "utDebug.h"


namespace Horde3D {

using namespace std;

Renderer::Renderer()
{
	_scratchBuf = 0x0;
	_overlayVerts = 0x0;
	_scratchBufSize = 0;
	_frameID = 1;
	_defShadowMap = 0;
	_quadIdxBuf = 0;
	_particleVBO = 0;
	_curCamera = 0x0;
	_curLight = 0x0;
	_curShader = 0x0;
	_curRenderTarget = 0x0;
	_curShaderUpdateStamp = 1;
	_maxAnisoMask = 0;
	_smSize = 0;
	_shadowRB = 0;
	_vlPosOnly = 0;
	_vlOverlay = 0;
	_vlModel = 0;
	_vlParticle = 0;
}


Renderer::~Renderer()
{
	releaseShadowRB();
	gRDI->destroyTexture( _defShadowMap );
	gRDI->destroyBuffer( _particleVBO );
	releaseShaderComb( _defColorShader );

	delete[] _scratchBuf;
	delete[] _overlayVerts;
}


// =================================================================================================
// Basic Initialization and Setup
// =================================================================================================

void Renderer::registerRenderFunc( int nodeType, RenderFunc rf )
{
	RenderFuncListItem item;
	item.nodeType = nodeType;
	item.renderFunc = rf;
	_renderFuncRegistry.push_back( item );
}


unsigned char *Renderer::useScratchBuf( uint32 minSize )
{
	if( _scratchBufSize < minSize )
	{
		delete[] _scratchBuf;
		_scratchBuf = new unsigned char[minSize + 15];
		_scratchBufSize = minSize;
	}

	return _scratchBuf + (size_t)_scratchBuf % 16;  // 16 byte aligned
}


bool Renderer::init()
{
	// Init Render Device Interface
	if( !gRDI->init() ) return false;

	// Check capabilities
	if( !gRDI->getCaps().texFloat )
		Modules::log().writeWarning( "Renderer: No floating point texture support available" );
	if( !gRDI->getCaps().texNPOT )
		Modules::log().writeWarning( "Renderer: No non-Power-of-two texture support available" );
	if( !gRDI->getCaps().rtMultisampling )
		Modules::log().writeWarning( "Renderer: No multisampling for render targets available" );
	
	// Create vertex layouts
	VertexLayoutAttrib attribsPosOnly[1] = {
		{"vertPos", 0, 3, 0}
	};
	_vlPosOnly = gRDI->registerVertexLayout( 1, attribsPosOnly );

	VertexLayoutAttrib attribsOverlay[2] = {
		{"vertPos", 0, 2, 0},
		{"texCoords0", 0, 2, 8}
	};
	_vlOverlay = gRDI->registerVertexLayout( 2, attribsOverlay );
	
	VertexLayoutAttrib attribsModel[7] = {
		{"vertPos", 0, 3, 0},
		{"normal", 1, 3, 0},
		{"tangent", 2, 4, 0},
		{"joints", 3, 4, 8},
		{"weights", 3, 4, 24},
		{"texCoords0", 3, 2, 0},
		{"texCoords1", 3, 2, 40}
	};
	_vlModel = gRDI->registerVertexLayout( 7, attribsModel );

	VertexLayoutAttrib attribsParticle[2] = {
		{"texCoords0", 0, 2, 0},
		{"parIdx", 0, 1, 8}
	};
	_vlParticle = gRDI->registerVertexLayout( 2, attribsParticle );
	
	// Upload default shaders
	if( !createShaderComb( gRDI->getDefaultVSCode(), gRDI->getDefaultFSCode(), _defColorShader ) )
	{
		Modules::log().writeError( "Failed to compile default shaders" );
		return false;
	}

	// Cache common uniforms
	_defColShader_color = gRDI->getShaderConstLoc( _defColorShader.shaderObj, "color" );
	
	// Create shadow map render target
	if( !createShadowRB( Modules::config().shadowMapSize, Modules::config().shadowMapSize ) )
	{
		Modules::log().writeError( "Failed to create shadow map" );
		return false;
	}

	// Create default shadow map
	float shadowTex[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	_defShadowMap = gRDI->createTexture( TextureTypes::Tex2D, 4, 4, 1, TextureFormats::DEPTH, false, false, false, false );
	gRDI->uploadTextureData( _defShadowMap, 0, 0, shadowTex );

	// Create index buffer used for drawing quads
	uint16 *quadIndices = new uint16[QuadIndexBufCount];
	for( uint32 i = 0; i < QuadIndexBufCount / 6; ++i )
	{
		quadIndices[i*6+0] = i * 4 + 0; quadIndices[i*6+1] = i * 4 + 1; quadIndices[i*6+2] = i * 4 + 2;
		quadIndices[i*6+3] = i * 4 + 2; quadIndices[i*6+4] = i * 4 + 3; quadIndices[i*6+5] = i * 4 + 0;
	}
	_quadIdxBuf = gRDI->createIndexBuffer( QuadIndexBufCount * sizeof( uint16 ), quadIndices );
	delete[] quadIndices; quadIndices = 0x0;
	
	// Create particle geometry array
	ParticleVert v0( 0, 0 );
	ParticleVert v1( 1, 0 );
	ParticleVert v2( 1, 1 );
	ParticleVert v3( 0, 1 );
	
	ParticleVert *parVerts = new ParticleVert[ParticlesPerBatch * 4];
	for( uint32 i = 0; i < ParticlesPerBatch; ++i )
	{
		parVerts[i * 4 + 0] = v0; parVerts[i * 4 + 0].index = (float)i;
		parVerts[i * 4 + 1] = v1; parVerts[i * 4 + 1].index = (float)i;
		parVerts[i * 4 + 2] = v2; parVerts[i * 4 + 2].index = (float)i;
		parVerts[i * 4 + 3] = v3; parVerts[i * 4 + 3].index = (float)i;
	}
	_particleVBO = gRDI->createVertexBuffer( ParticlesPerBatch * 4 * sizeof( ParticleVert ), (float *)parVerts );
	delete[] parVerts; parVerts = 0x0;

	_overlayBatches.reserve( 64 );
	_overlayVerts = new OverlayVert[MaxNumOverlayVerts];
	_overlayVB = gRDI->createVertexBuffer( MaxNumOverlayVerts * sizeof( OverlayVert ), 0x0 );

	// Create unit primitives
	createPrimitives();

	// Init scratch buffer with some default size
	useScratchBuf( 4 * 1024*1024 );

	// Reset states
	finishRendering();

	// Start frame timer
	Timer *timer = Modules::stats().getTimer( EngineStats::FrameTime );
	ASSERT( timer != 0x0 );
	timer->setEnabled( true );
	
	return true;
}


void Renderer::initStates()
{
	gRDI->initStates();
}


// =================================================================================================
// Misc Helper Functions
// =================================================================================================

void Renderer::setupViewMatrices( const Matrix4f &viewMat, const Matrix4f &projMat )
{
	// Note: The viewing matrices should be set before a material is set, otherwise the general
	//       uniforms need to be commited manually
	
	_viewMat = viewMat;
	_viewMatInv = viewMat.inverted();
	_projMat = projMat;
	_viewProjMat = projMat * viewMat;
	_viewProjMatInv = _viewProjMat.inverted();

	++_curShaderUpdateStamp;
}


// =================================================================================================
// Rendering Helper Functions
// =================================================================================================

void Renderer::createPrimitives()
{
	// Unit cube
	float cubeVerts[8 * 3] = {  // x, y, z
		0.f, 0.f, 1.f,   1.f, 0.f, 1.f,   1.f, 1.f, 1.f,   0.f, 1.f, 1.f,
		0.f, 0.f, 0.f,   1.f, 0.f, 0.f,   1.f, 1.f, 0.f,   0.f, 1.f, 0.f
	};
	uint16 cubeInds[36] = {
		0, 1, 2, 2, 3, 0,   1, 5, 6, 6, 2, 1,   5, 4, 7, 7, 6, 5,
		4, 0, 3, 3, 7, 4,   3, 2, 6, 6, 7, 3,   4, 5, 1, 1, 0, 4
	};
	_vbCube = gRDI->createVertexBuffer( 8 * 3 * sizeof( float ), cubeVerts );
	_ibCube = gRDI->createIndexBuffer( 36 * sizeof( uint16 ), cubeInds );

	// Unit (geodesic) sphere (created by recursively subdividing a base octahedron)
	Vec3f spVerts[126] = {  // x, y, z
		Vec3f( 0.f, 1.f, 0.f ),   Vec3f( 0.f, -1.f, 0.f ),
		Vec3f( -0.707f, 0.f, 0.707f ),   Vec3f( 0.707f, 0.f, 0.707f ),
		Vec3f( 0.707f, 0.f, -0.707f ),   Vec3f( -0.707f, 0.f, -0.707f )
	};
	uint16 spInds[128 * 3] = {  // Number of faces: (4 ^ iterations) * 8
		2, 3, 0,   3, 4, 0,   4, 5, 0,   5, 2, 0,   2, 1, 3,   3, 1, 4,   4, 1, 5,   5, 1, 2
	};
	for( uint32 i = 0, nv = 6, ni = 24; i < 2; ++i )  // Two iterations
	{
		// Subdivide each face into 4 tris by bisecting each edge and push vertices onto unit sphere
		for( uint32 j = 0, prevNumInds = ni; j < prevNumInds; j += 3 )
		{
			spVerts[nv++] = ((spVerts[spInds[j + 0]] + spVerts[spInds[j + 1]]) * 0.5f).normalized();
			spVerts[nv++] = ((spVerts[spInds[j + 1]] + spVerts[spInds[j + 2]]) * 0.5f).normalized();
			spVerts[nv++] = ((spVerts[spInds[j + 2]] + spVerts[spInds[j + 0]]) * 0.5f).normalized();

			spInds[ni++] = spInds[j + 0]; spInds[ni++] = nv - 3; spInds[ni++] = nv - 1;
			spInds[ni++] = nv - 3; spInds[ni++] = spInds[j + 1]; spInds[ni++] = nv - 2;
			spInds[ni++] = nv - 2; spInds[ni++] = spInds[j + 2]; spInds[ni++] = nv - 1;
			spInds[j + 0] = nv - 3; spInds[j + 1] = nv - 2; spInds[j + 2] = nv - 1;
		}
	}
	_vbSphere = gRDI->createVertexBuffer( 126 * sizeof( Vec3f ), spVerts );
	_ibSphere = gRDI->createIndexBuffer( 128 * 3 * sizeof( uint16 ), spInds );
	
	// Unit cone
	float coneVerts[13 * 3] = {  // x, y, z
		0.f, 0.f, 0.f,
		0.f, 1.f, -1.f,   -0.5f, 0.866f, -1.f,   -0.866f, 0.5f, -1.f,
		-1.f, 0.f, -1.f,   -0.866f, -0.5f, -1.f,   -0.5f, -0.866f, -1.f,
		0.f, -1.f, -1.f,   0.5f, -0.866f, -1.f,   0.866f, -0.5f, -1.f,
		1.f, 0.f, -1.f,   0.866f, 0.5f, -1.f,   0.5f, 0.866f, -1.f,
	};
	uint16 coneInds[22 * 3] = {
		0, 1, 2,   0, 2, 3,   0, 3, 4,   0, 4, 5,   0, 5, 6,   0, 6, 7,
		0, 7, 8,   0, 8, 9,   0, 9, 10,   0, 10, 11,   0, 11, 12,   0, 12, 1,
		10, 6, 2,   10, 8, 6,   10, 9, 8,   8, 7, 6,   6, 4, 2,   6, 5, 4,   4, 3, 2,
		2, 12, 10,   2, 1, 12,   12, 11, 10
	};
	_vbCone = gRDI->createVertexBuffer( 13 * 3 * sizeof( float ), coneVerts );
	_ibCone = gRDI->createIndexBuffer( 22 * 3 * sizeof( uint16 ), coneInds );

	// Fullscreen polygon
	float fsVerts[3 * 3] = {  // x, y, z
		0.f, 0.f, 1.f,   2.f, 0.f, 1.f,   0.f, 2.f, 1.f
	};
	_vbFSPoly = gRDI->createVertexBuffer( 3 * 3 * sizeof( float ), fsVerts );
}


void Renderer::drawAABB( const Vec3f &bbMin, const Vec3f &bbMax )
{
	ASSERT( _curShader != 0x0 );
	
	Matrix4f mat = Matrix4f::TransMat( bbMin.x, bbMin.y, bbMin.z ) *
		Matrix4f::ScaleMat( bbMax.x - bbMin.x, bbMax.y - bbMin.y, bbMax.z - bbMin.z );
	gRDI->setShaderConst( _curShader->uni_worldMat, CONST_FLOAT44, &mat.x[0] );
	
	gRDI->setVertexBuffer( 0, _vbCube, 0, 12 );
	gRDI->setIndexBuffer( _ibCube, IDXFMT_16 );
	gRDI->setVertexLayout( _vlPosOnly );

	gRDI->drawIndexed( PRIM_TRILIST, 0, 36, 0, 8 );
}


void Renderer::drawSphere( const Vec3f &pos, float radius )
{
	ASSERT( _curShader != 0x0 );

	Matrix4f mat = Matrix4f::TransMat( pos.x, pos.y, pos.z ) *
	               Matrix4f::ScaleMat( radius, radius, radius );
	gRDI->setShaderConst( _curShader->uni_worldMat, CONST_FLOAT44, &mat.x[0] );
	
	gRDI->setVertexBuffer( 0, _vbSphere, 0, 12 );
	gRDI->setIndexBuffer( _ibSphere, IDXFMT_16 );
	gRDI->setVertexLayout( _vlPosOnly );

	gRDI->drawIndexed( PRIM_TRILIST, 0, 128 * 3, 0, 126 );
}


void Renderer::drawCone( float height, float radius, const Matrix4f &transMat )
{
	ASSERT( _curShader != 0x0 );

	Matrix4f mat = transMat * Matrix4f::ScaleMat( radius, radius, height );
	gRDI->setShaderConst( _curShader->uni_worldMat, CONST_FLOAT44, &mat.x[0] );
	
	gRDI->setVertexBuffer( 0, _vbCone, 0, 12 );
	gRDI->setIndexBuffer( _ibCone, IDXFMT_16 );
	gRDI->setVertexLayout( _vlPosOnly );

	gRDI->drawIndexed( PRIM_TRILIST, 0, 22 * 3, 0, 13 );
}


// =================================================================================================
// Material System
// =================================================================================================

bool Renderer::createShaderComb( const char *vertexShader, const char *fragmentShader, ShaderCombination &sc )
{
	// Create shader program
	uint32 shdObj = gRDI->createShader( vertexShader, fragmentShader );
	if( shdObj == 0 ) return false;
	
	sc.shaderObj = shdObj;
	gRDI->bindShader( shdObj );
	
	// Set standard uniforms
	int loc =gRDI-> getShaderSamplerLoc( shdObj, "shadowMap" );
	if( loc >= 0 ) gRDI->setShaderSampler( loc, 12 );

	// Misc general uniforms
	sc.uni_frameBufSize = gRDI->getShaderConstLoc( shdObj, "frameBufSize" );
	
	// View/projection uniforms
	sc.uni_viewMat = gRDI->getShaderConstLoc( shdObj, "viewMat" );
	sc.uni_viewMatInv = gRDI->getShaderConstLoc( shdObj, "viewMatInv" );
	sc.uni_projMat = gRDI->getShaderConstLoc( shdObj, "projMat" );
	sc.uni_viewProjMat = gRDI->getShaderConstLoc( shdObj, "viewProjMat" );
	sc.uni_viewProjMatInv = gRDI->getShaderConstLoc( shdObj, "viewProjMatInv" );
	sc.uni_viewerPos = gRDI->getShaderConstLoc( shdObj, "viewerPos" );
	
	// Per-instance uniforms
	sc.uni_worldMat = gRDI->getShaderConstLoc( shdObj, "worldMat" );
	sc.uni_worldNormalMat = gRDI->getShaderConstLoc( shdObj, "worldNormalMat" );
	sc.uni_nodeId = gRDI->getShaderConstLoc( shdObj, "nodeId" );
	sc.uni_customInstData = gRDI->getShaderConstLoc( shdObj, "customInstData[0]" );
	sc.uni_skinMatRows = gRDI->getShaderConstLoc( shdObj, "skinMatRows[0]" );
	
	// Lighting uniforms
	sc.uni_lightPos = gRDI->getShaderConstLoc( shdObj, "lightPos" );
	sc.uni_lightDir = gRDI->getShaderConstLoc( shdObj, "lightDir" );
	sc.uni_lightColor = gRDI->getShaderConstLoc( shdObj, "lightColor" );
	sc.uni_shadowSplitDists = gRDI->getShaderConstLoc( shdObj, "shadowSplitDists" );
	sc.uni_shadowMats = gRDI->getShaderConstLoc( shdObj, "shadowMats" );
	sc.uni_shadowMapSize = gRDI->getShaderConstLoc( shdObj, "shadowMapSize" );
	sc.uni_shadowBias = gRDI->getShaderConstLoc( shdObj, "shadowBias" );
	
	// Particle-specific uniforms
	sc.uni_parPosArray = gRDI->getShaderConstLoc( shdObj, "parPosArray" );
	sc.uni_parSizeAndRotArray = gRDI->getShaderConstLoc( shdObj, "parSizeAndRotArray" );
	sc.uni_parColorArray = gRDI->getShaderConstLoc( shdObj, "parColorArray" );
	
	// Overlay-specific uniforms
	sc.uni_olayColor = gRDI->getShaderConstLoc( shdObj, "olayColor" );

	return true;
}


void Renderer::releaseShaderComb( ShaderCombination &sc )
{
	gRDI->destroyShader( sc.shaderObj );
}


void Renderer::setShaderComb( ShaderCombination *sc )
{
	if( _curShader != sc )
	{
		if( sc == 0x0 ) gRDI->bindShader( 0 );
		else gRDI->bindShader( sc->shaderObj );

		_curShader = sc;
	}
}


void Renderer::commitGeneralUniforms()
{
	ASSERT( _curShader != 0x0 );

	// Note: Make sure that all functions which modify one of the following params increase the stamp
	if( _curShader->lastUpdateStamp != _curShaderUpdateStamp )
	{
		if( _curShader->uni_frameBufSize >= 0 )
		{
			float dimensions[2] = { (float)gRDI->_fbWidth, (float)gRDI->_fbHeight };
			gRDI->setShaderConst( _curShader->uni_frameBufSize, CONST_FLOAT2, dimensions );
		}
		
		// Viewer params
		if( _curShader->uni_viewMat >= 0 )
			gRDI->setShaderConst( _curShader->uni_viewMat, CONST_FLOAT44, _viewMat.x );
		
		if( _curShader->uni_viewMatInv >= 0 )
			gRDI->setShaderConst( _curShader->uni_viewMatInv, CONST_FLOAT44, _viewMatInv.x );
		
		if( _curShader->uni_projMat >= 0 )
			gRDI->setShaderConst( _curShader->uni_projMat, CONST_FLOAT44, _projMat.x );
		
		if( _curShader->uni_viewProjMat >= 0 )
			gRDI->setShaderConst( _curShader->uni_viewProjMat, CONST_FLOAT44, _viewProjMat.x );

		if( _curShader->uni_viewProjMatInv >= 0 )
			gRDI->setShaderConst( _curShader->uni_viewProjMatInv, CONST_FLOAT44, _viewProjMatInv.x );
		
		if( _curShader->uni_viewerPos >= 0 )
			gRDI->setShaderConst( _curShader->uni_viewerPos, CONST_FLOAT3, &_viewMatInv.x[12] );
		
		// Light params
		if( _curLight != 0x0 )
		{
			if( _curShader->uni_lightPos >= 0 )
			{
				float data[4] = { _curLight->_absPos.x, _curLight->_absPos.y,
				                  _curLight->_absPos.z, _curLight->_radius };
				gRDI->setShaderConst( _curShader->uni_lightPos, CONST_FLOAT4, data );
			}
			
			if( _curShader->uni_lightDir >= 0 )
			{
				float data[4] = { _curLight->_spotDir.x, _curLight->_spotDir.y,
				                  _curLight->_spotDir.z, cosf( degToRad( _curLight->_fov / 2.0f ) ) };
				gRDI->setShaderConst( _curShader->uni_lightDir, CONST_FLOAT4, data );
			}
			
			if( _curShader->uni_lightColor >= 0 )
			{
				Vec3f col = _curLight->_diffuseCol * _curLight->_diffuseColMult;
				gRDI->setShaderConst( _curShader->uni_lightColor, CONST_FLOAT3, &col.x );
			}
			
			if( _curShader->uni_shadowSplitDists >= 0 )
				gRDI->setShaderConst( _curShader->uni_shadowSplitDists, CONST_FLOAT4, &_splitPlanes[1] );

			if( _curShader->uni_shadowMats >= 0 )
				gRDI->setShaderConst( _curShader->uni_shadowMats, CONST_FLOAT44, &_lightMats[0].x[0], 4 );
			
			if( _curShader->uni_shadowMapSize >= 0 )
				gRDI->setShaderConst( _curShader->uni_shadowMapSize, CONST_FLOAT, &_smSize );
			
			if( _curShader->uni_shadowBias >= 0 )
				gRDI->setShaderConst( _curShader->uni_shadowBias, CONST_FLOAT, &_curLight->_shadowMapBias );
		}

		_curShader->lastUpdateStamp = _curShaderUpdateStamp;
	}
}


bool Renderer::setMaterialRec( MaterialResource *materialRes, const string &shaderContext,
                               ShaderResource *shaderRes )
{
	if( materialRes == 0x0 ) return false;
	
	bool firstRec = (shaderRes == 0x0);
	bool result = true;
	
	// Set shader in first recursion step
	if( firstRec )
	{	
		shaderRes = materialRes->_shaderRes;
		if( shaderRes == 0x0 ) return false;	
	
		// Find context
		ShaderContext *context = shaderRes->findContext( shaderContext );
		if( context == 0x0 ) return false;
		
		// Set shader combination
		ShaderCombination *sc = shaderRes->getCombination( *context, materialRes->_combMask );
		if( sc != _curShader ) setShaderComb( sc );
		if( _curShader == 0x0 || gRDI->_curShaderId == 0 ) return false;

		// Setup standard shader uniforms
		commitGeneralUniforms();

		// Configure depth mask
		gRDI->setDepthMask( context->writeDepth );

		// Configure cull mode
		if( !Modules::config().wireframeMode )
		{
			gRDI->setCullMode( (RDICullMode)context->cullMode );
		}
		
		// Configure blending
		switch( context->blendMode )
		{
		case BlendModes::Replace:
			gRDI->setBlendMode( false );
			break;
		case BlendModes::Blend:
			gRDI->setBlendMode( true, BS_BLEND_SRC_ALPHA, BS_BLEND_INV_SRC_ALPHA );
			break;
		case BlendModes::Add:
			gRDI->setBlendMode( true, BS_BLEND_ONE, BS_BLEND_ONE );
			break;
		case BlendModes::AddBlended:
			gRDI->setBlendMode( true, BS_BLEND_SRC_ALPHA, BS_BLEND_ONE );
			break;
		case BlendModes::Mult:
			gRDI->setBlendMode( true, BS_BLEND_DEST_COLOR, BS_BLEND_ZERO );
			break;
		}

		// Configure depth test
		gRDI->setDepthTest( context->depthTest );
		gRDI->setDepthFunc( (RDIDepthFunc)context->depthFunc );

		// Configure alpha-to-coverage
		gRDI->setAlphaToCoverage( context->alphaToCoverage && Modules::config().sampleCount > 0 );
	}

	// Setup texture samplers
	for( size_t i = 0, si = shaderRes->_samplers.size(); i < si; ++i )
	{
		if( _curShader->customSamplers[i] < 0 ) continue;
		
		ShaderSampler &sampler = shaderRes->_samplers[i];
		TextureResource *texRes = 0x0;

		// Use default texture
		if( firstRec) texRes = sampler.defTex;
		
		// Find sampler in material
		for( size_t j = 0, sj = materialRes->_samplers.size(); j < sj; ++j )
		{
			if( materialRes->_samplers[j].name == sampler.id )
			{
				if( materialRes->_samplers[j].texRes->isLoaded() )
					texRes = materialRes->_samplers[j].texRes;
				break;
			}
		}

		uint32 sampState = shaderRes->_samplers[i].sampState;
		if( (sampState & SS_FILTER_TRILINEAR) && !Modules::config().trilinearFiltering )
			sampState = (sampState & ~SS_FILTER_TRILINEAR) | SS_FILTER_BILINEAR;
		if( (sampState & SS_ANISO_MASK) > _maxAnisoMask )
			sampState = (sampState & ~SS_ANISO_MASK) | _maxAnisoMask;

		// Bind texture
		if( texRes != 0x0 )
		{
			if( texRes->getTexType() != sampler.type ) break;  // Wrong type
			
			if( texRes->getTexType() == TextureTypes::Tex2D )
			{
				if( texRes->getRBObject() == 0 )
				{
					gRDI->setTexture( shaderRes->_samplers[i].texUnit, texRes->getTexObject(), sampState );
				}
				else if( texRes->getRBObject() != gRDI->_curRendBuf )
				{
					gRDI->setTexture( shaderRes->_samplers[i].texUnit,
					                  gRDI->getRenderBufferTex( texRes->getRBObject(), 0 ), sampState );
				}
				else  // Trying to bind active render buffer as texture
				{
					gRDI->setTexture( shaderRes->_samplers[i].texUnit, TextureResource::defTex2DObject, 0 );
				}
			}
			else
			{
				gRDI->setTexture( shaderRes->_samplers[i].texUnit, texRes->getTexObject(), sampState );
			}
		}

		// Find sampler in pipeline
		if( firstRec )
		{
			for( size_t j = 0, sj = _pipeSamplerBindings.size(); j < sj; ++j )
			{
				if( strcmp( _pipeSamplerBindings[j].sampler, sampler.id.c_str() ) == 0 )
				{
					gRDI->setTexture( shaderRes->_samplers[i].texUnit, gRDI->getRenderBufferTex(
						_pipeSamplerBindings[j].rbObj, _pipeSamplerBindings[j].bufIndex ), sampState );

					break;
				}
			}
		}
	}

	// Set custom uniforms
	for( size_t i = 0, si = shaderRes->_uniforms.size(); i < si; ++i )
	{
		if( _curShader->customUniforms[i] < 0 ) continue;
		
		float *unifData = 0x0;

		// Find uniform in material
		for( size_t j = 0, sj = materialRes->_uniforms.size(); j < sj; ++j )
		{
			MatUniform &matUniform = materialRes->_uniforms[j];
			
			if( matUniform.name == shaderRes->_uniforms[i].id )
			{
				unifData = matUniform.values;
				break;
			}
		}

		// Use default values if not found
		if( unifData == 0x0 && firstRec )
			unifData = shaderRes->_uniforms[i].defValues;

		if( unifData )
		{
			switch( shaderRes->_uniforms[i].size )
			{
			case 1:
				gRDI->setShaderConst( _curShader->customUniforms[i], CONST_FLOAT, unifData );
				break;
			case 4:
				gRDI->setShaderConst( _curShader->customUniforms[i], CONST_FLOAT4, unifData );
				break;
			}
		}
	}

	if( firstRec )
	{
		// Handle link of stage
		if( _curStageMatLink != 0x0 && _curStageMatLink != materialRes )
			result &= setMaterialRec( _curStageMatLink, shaderContext, shaderRes );

		// Handle material of light source
		if( _curLight != 0x0 && _curLight->_materialRes != 0x0 && _curLight->_materialRes != materialRes )
			result &= setMaterialRec( _curLight->_materialRes, shaderContext, shaderRes );
	}

	// Handle link of material resource
	if( materialRes->_matLink != 0x0 )
		result &= setMaterialRec( materialRes->_matLink, shaderContext, shaderRes );

	return result;
}


bool Renderer::setMaterial( MaterialResource *materialRes, const string &shaderContext )
{
	if( materialRes == 0x0 )
	{	
		setShaderComb( 0x0 );
		gRDI->setBlendMode( false );
		gRDI->setAlphaToCoverage( false );
		gRDI->setDepthTest( true );
		gRDI->setDepthFunc( DSS_DEPTHFUNC_LESS_EQUAL );
		gRDI->setDepthMask( true );
		return false;
	}

	if( !setMaterialRec( materialRes, shaderContext, 0x0 ) )
	{
		_curShader = 0x0;
		return false;
	}

	return true;
}


// =================================================================================================
// Shadowing
// =================================================================================================

bool Renderer::createShadowRB( uint32 width, uint32 height )
{
	_shadowRB = gRDI->createRenderBuffer( width, height, TextureFormats::BGRA8, true, 0, 0 );
	
	return _shadowRB != 0;
}


void Renderer::releaseShadowRB()
{
	if( _shadowRB ) gRDI->destroyRenderBuffer( _shadowRB );
}


void Renderer::setupShadowMap( bool noShadows )
{
	uint32 sampState = SS_FILTER_BILINEAR | SS_ANISO1 | SS_ADDR_CLAMPCOL | SS_COMP_LEQUAL;
	
	// Bind shadow map
	if( !noShadows && _curLight->_shadowMapCount > 0 )
	{
		gRDI->setTexture( 12, gRDI->getRenderBufferTex( _shadowRB, 32 ), sampState );
		_smSize = (float)Modules::config().shadowMapSize;
	}
	else
	{
		gRDI->setTexture( 12, _defShadowMap, sampState );
		_smSize = 4;
	}
}


Matrix4f Renderer::calcCropMatrix( const Frustum &frustSlice, const Vec3f lightPos, const Matrix4f &lightViewProjMat )
{
	float frustMinX =  Math::MaxFloat, bbMinX =  Math::MaxFloat;
	float frustMinY =  Math::MaxFloat, bbMinY =  Math::MaxFloat;
	float frustMinZ =  Math::MaxFloat, bbMinZ =  Math::MaxFloat;
	float frustMaxX = -Math::MaxFloat, bbMaxX = -Math::MaxFloat;
	float frustMaxY = -Math::MaxFloat, bbMaxY = -Math::MaxFloat;
	float frustMaxZ = -Math::MaxFloat, bbMaxZ = -Math::MaxFloat;
	
	// Find post-projective space AABB of all objects in frustum
	Modules::sceneMan().updateQueues( frustSlice, 0x0, RenderingOrder::None,
		SceneNodeFlags::NoDraw | SceneNodeFlags::NoCastShadow, false, true );
	RenderQueue &renderQueue = Modules::sceneMan().getRenderQueue();
	
	for( size_t i = 0, s = renderQueue.size(); i < s; ++i )
	{
		const BoundingBox &aabb = renderQueue[i].node->getBBox();
		
		// Check if light is inside AABB
		if( lightPos.x >= aabb.min.x && lightPos.y >= aabb.min.y && lightPos.z >= aabb.min.z &&
			lightPos.x <= aabb.max.x && lightPos.y <= aabb.max.y && lightPos.z <= aabb.max.z )
		{
			bbMinX = bbMinY = bbMinZ = -1;
			bbMaxX = bbMaxY = bbMaxZ = 1;
			break;
		}
		
		for( uint32 j = 0; j < 8; ++j )
		{
			Vec4f v1 = lightViewProjMat * Vec4f( aabb.getCorner( j ) );
			v1.w = 1.f / fabsf( v1.w );
			v1.x *= v1.w; v1.y *= v1.w; v1.z *= v1.w;
			
			if( v1.x < bbMinX ) bbMinX = v1.x;
			if( v1.y < bbMinY ) bbMinY = v1.y;
			if( v1.z < bbMinZ ) bbMinZ = v1.z;
			if( v1.x > bbMaxX ) bbMaxX = v1.x;
			if( v1.y > bbMaxY ) bbMaxY = v1.y;
			if( v1.z > bbMaxZ ) bbMaxZ = v1.z;
		}
	}

	// Find post-projective space AABB of frustum slice if light is not inside
	if( frustSlice.cullSphere( _curLight->_absPos, 0 ) )
	{
		// Get frustum in post-projective space
		for( uint32 i = 0; i < 8; ++i )
		{
			// Frustum slice
			Vec4f v1 = lightViewProjMat * Vec4f( frustSlice.getCorner( i ) );
			v1.w = 1.f / fabsf( v1.w );  // Use absolute value to reduce problems with back projection when v1.w < 0
			v1.x *= v1.w; v1.y *= v1.w; v1.z *= v1.w;

			if( v1.x < frustMinX ) frustMinX = v1.x;
			if( v1.y < frustMinY ) frustMinY = v1.y;
			if( v1.z < frustMinZ ) frustMinZ = v1.z;
			if( v1.x > frustMaxX ) frustMaxX = v1.x;
			if( v1.y > frustMaxY ) frustMaxY = v1.y;
			if( v1.z > frustMaxZ ) frustMaxZ = v1.z;
		}
	}
	else
	{
		frustMinX = frustMinY = frustMinZ = -1;
		frustMaxX = frustMaxY = frustMaxZ = 1;
	}

	// Merge frustum and AABB bounds and clamp to post-projective range [-1, 1]
	float minX = clamp( maxf( frustMinX, bbMinX ), -1, 1 );
	float minY = clamp( maxf( frustMinY, bbMinY ), -1, 1 );
	float minZ = clamp( minf( frustMinZ, bbMinZ ), -1, 1 );
	float maxX = clamp( minf( frustMaxX, bbMaxX ), -1, 1 );
	float maxY = clamp( minf( frustMaxY, bbMaxY ), -1, 1 );
	float maxZ = clamp( minf( frustMaxZ, bbMaxZ ), -1, 1 );

	// Zoom-in slice to make better use of available shadow map space
	float scaleX = 2.0f / (maxX - minX);
	float scaleY = 2.0f / (maxY - minY);
	float scaleZ = 2.0f / (maxZ - minZ);

	float offsetX = -0.5f * (maxX + minX) * scaleX;
	float offsetY = -0.5f * (maxY + minY) * scaleY;
	float offsetZ = -0.5f * (maxZ + minZ) * scaleZ;

	// Build final matrix
	float cropMat[16] = { scaleX, 0, 0, 0,
	                      0, scaleY, 0, 0,
	                      0, 0, scaleZ, 0,
	                      offsetX, offsetY, offsetZ, 1 };

	return Matrix4f( cropMat );
}


void Renderer::updateShadowMap()
{
	if( _curLight == 0x0 ) return;
	
	uint32 prevRendBuf = gRDI->_curRendBuf;
	int prevVPX = gRDI->_vpX, prevVPY = gRDI->_vpY, prevVPWidth = gRDI->_vpWidth, prevVPHeight = gRDI->_vpHeight;
	RDIRenderBuffer &shadowRT = gRDI->_rendBufs.getRef( _shadowRB );
	gRDI->setViewport( 0, 0, shadowRT.width, shadowRT.height );
	gRDI->setRenderBuffer( _shadowRB );
	
	gRDI->setColorWriteMask( false );
	gRDI->setDepthMask( true );
	gRDI->clear( CLR_DEPTH, 0x0, 1.f );

	// ********************************************************************************************
	// Cascaded Shadow Maps
	// ********************************************************************************************
	
	// Find AABB of lit geometry
	BoundingBox aabb;
	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), &_curLight->getFrustum(),
		RenderingOrder::None, SceneNodeFlags::NoDraw | SceneNodeFlags::NoCastShadow, false, true );
	for( size_t j = 0, s = Modules::sceneMan().getRenderQueue().size(); j < s; ++j )
	{
		aabb.makeUnion( Modules::sceneMan().getRenderQueue()[j].node->getBBox() );
	}

	// Find depth range of lit geometry
	float minDist = Math::MaxFloat, maxDist = 0.0f;
	for( uint32 i = 0; i < 8; ++i )
	{
		float dist = -(_curCamera->getViewMat() * aabb.getCorner( i )).z;
		if( dist < minDist ) minDist = dist;
		if( dist > maxDist ) maxDist = dist;
	}

	// Don't adjust near plane; this means less precision if scene is far away from viewer but that
	// shouldn't be too noticeable and brings better performance since the nearer split volumes are empty
	minDist = _curCamera->_frustNear;
	
	// Calculate split distances using PSSM scheme
	const float nearDist = maxf( minDist, _curCamera->_frustNear );
	const float farDist = maxf( maxDist, minDist + 0.01f );
	const uint32 numMaps = _curLight->_shadowMapCount;
	const float lambda = _curLight->_shadowSplitLambda;
	
	_splitPlanes[0] = nearDist;
	_splitPlanes[numMaps] = farDist;
	
	for( uint32 i = 1; i < numMaps; ++i )
	{
		float f = (float)i / numMaps;
		float logDist = nearDist * powf( farDist / nearDist, f );
		float uniformDist = nearDist + (farDist - nearDist) * f;
		
		_splitPlanes[i] = (1 - lambda) * uniformDist + lambda * logDist;  // Lerp
	}
	
	// Prepare shadow map rendering
	gRDI->setDepthTest( true );
	//gRDI->setCullMode( RS_CULL_FRONT );	// Front face culling reduces artefacts but produces more "peter-panning"
	
	// Split viewing frustum into slices and render shadow maps
	Frustum frustum;
	for( uint32 i = 0; i < numMaps; ++i )
	{
		// Create frustum slice
		if( !_curCamera->_orthographic )
		{
			float newLeft = _curCamera->_frustLeft * _splitPlanes[i] / _curCamera->_frustNear;
			float newRight = _curCamera->_frustRight * _splitPlanes[i] / _curCamera->_frustNear;
			float newBottom = _curCamera->_frustBottom * _splitPlanes[i] / _curCamera->_frustNear;
			float newTop = _curCamera->_frustTop * _splitPlanes[i] / _curCamera->_frustNear;
			frustum.buildViewFrustum( _curCamera->_absTrans, newLeft, newRight, newBottom, newTop,
			                          _splitPlanes[i], _splitPlanes[i + 1] );
		}
		else
		{
			frustum.buildBoxFrustum( _curCamera->_absTrans, _curCamera->_frustLeft, _curCamera->_frustRight,
			                         _curCamera->_frustBottom, _curCamera->_frustTop,
			                         -_splitPlanes[i], -_splitPlanes[i + 1] );
		}
		
		// Get light projection matrix
		float ymax = _curCamera->_frustNear * tanf( degToRad( _curLight->_fov / 2 ) );
		float xmax = ymax * 1.0f;  // ymax * aspect
		Matrix4f lightProjMat = Matrix4f::PerspectiveMat(
			-xmax, xmax, -ymax, ymax, _curCamera->_frustNear, _curLight->_radius );
		
		// Build optimized light projection matrix
		Matrix4f lightViewProjMat = lightProjMat * _curLight->getViewMat();
		lightProjMat = calcCropMatrix( frustum, _curLight->_absPos, lightViewProjMat ) * lightProjMat;
		
		// Generate render queue with shadow casters for current slice
		frustum.buildViewFrustum( _curLight->getViewMat(), lightProjMat );
		Modules::sceneMan().updateQueues( frustum, 0x0, RenderingOrder::None,
			SceneNodeFlags::NoDraw | SceneNodeFlags::NoCastShadow, false, true );
		
		// Create texture atlas if several splits are enabled
		if( numMaps > 1 )
		{
			const int hsm = Modules::config().shadowMapSize / 2;
			const int scissorXY[8] = { 0, 0,  hsm, 0,  hsm, hsm,  0, hsm };
			const float transXY[8] = { -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, 0.5f,  -0.5f, 0.5f };
			
			gRDI->setScissorTest( true );

			// Select quadrant of shadow map
			lightProjMat.scale( 0.5f, 0.5f, 1.0f );
			lightProjMat.translate( transXY[i * 2], transXY[i * 2 + 1], 0.0f );
			gRDI->setScissorRect( scissorXY[i * 2], scissorXY[i * 2 + 1], hsm, hsm );
		}
	
		_lightMats[i] = lightProjMat * _curLight->getViewMat();
		setupViewMatrices( _curLight->getViewMat(), lightProjMat );
		
		// Render
		drawRenderables( _curLight->_shadowContext, "", false, &frustum, 0x0, RenderingOrder::None, -1 );
	}

	// Map from post-projective space [-1,1] to texture space [0,1]
	for( uint32 i = 0; i < numMaps; ++i )
	{
		_lightMats[i].scale( 0.5f, 0.5f, 1.0f );
		_lightMats[i].translate( 0.5f, 0.5f, 0.0f );
	}

	// ********************************************************************************************

	gRDI->setCullMode( RS_CULL_BACK );
	gRDI->setScissorTest( false );
		
	gRDI->setViewport( prevVPX, prevVPY, prevVPWidth, prevVPHeight );
	gRDI->setRenderBuffer( prevRendBuf );
	gRDI->setColorWriteMask( true );
}


// =================================================================================================
// Occlusion Culling
// =================================================================================================

int Renderer::registerOccSet()
{
	for( int i = 0; i < (int)_occSets.size(); ++i )
	{
		if( _occSets[i] == 0 )
		{
			_occSets[i] = 1;
			return i;
		}
	}

	_occSets.push_back( 1 );
	return (int)_occSets.size() - 1;
}


void Renderer::unregisterOccSet( int occSet )
{
	if( occSet >= 0 && occSet < (int)_occSets.size() )
		_occSets[occSet] = 0;
}


void Renderer::drawOccProxies( uint32 list )
{
	ASSERT( list < 2 );

	bool prevColorMask, prevDepthMask;
	gRDI->getColorWriteMask( prevColorMask );
	gRDI->getDepthMask( prevDepthMask );
	
	setMaterial( 0x0, "" );
	gRDI->setColorWriteMask( false );
	gRDI->setDepthMask( false );
	
	setShaderComb( &Modules::renderer()._defColorShader );
	commitGeneralUniforms();
	gRDI->setVertexBuffer( 0, _vbCube, 0, 12 );
	gRDI->setIndexBuffer( _ibCube, IDXFMT_16 );
	gRDI->setVertexLayout( _vlPosOnly );

	// Draw occlusion proxies
	for( size_t i = 0, s = _occProxies[list].size(); i < s; ++i )
	{
		OccProxy &proxy = _occProxies[list][i];

		gRDI->beginQuery( proxy.queryObj );
		
		Matrix4f mat = Matrix4f::TransMat( proxy.bbMin.x, proxy.bbMin.y, proxy.bbMin.z ) *
			Matrix4f::ScaleMat( proxy.bbMax.x - proxy.bbMin.x, proxy.bbMax.y - proxy.bbMin.y, proxy.bbMax.z - proxy.bbMin.z );
		gRDI->setShaderConst( _curShader->uni_worldMat, CONST_FLOAT44, &mat.x[0] );

		// Draw AABB
		gRDI->drawIndexed( PRIM_TRILIST, 0, 36, 0, 8 );

		gRDI->endQuery( proxy.queryObj );
	}

	setShaderComb( 0x0 );
	gRDI->setColorWriteMask( prevColorMask );
	gRDI->setDepthMask( prevDepthMask );

	_occProxies[list].resize( 0 );
}


// =================================================================================================
// Overlays
// =================================================================================================

void Renderer::showOverlays( const float *verts, uint32 vertCount, float *colRGBA,
                             MaterialResource *matRes, int flags )
{
	uint32 numOverlayVerts = 0;
	if( !_overlayBatches.empty() )
		numOverlayVerts = _overlayBatches.back().firstVert + _overlayBatches.back().vertCount;
	
	if( numOverlayVerts + vertCount > MaxNumOverlayVerts ) return;

	memcpy( &_overlayVerts[numOverlayVerts], verts, vertCount * sizeof( OverlayVert ) );
	
	// Check if previous batch can be extended
	if( !_overlayBatches.empty() )
	{
		OverlayBatch &prevBatch = _overlayBatches.back();
		if( matRes == prevBatch.materialRes && flags == prevBatch.flags &&
			memcmp( colRGBA, prevBatch.colRGBA, 4 * sizeof( float ) ) == 0 )
		{
			prevBatch.vertCount += vertCount;
			return;
		}
	}
	
	// Create new batch
	_overlayBatches.push_back( OverlayBatch( numOverlayVerts, vertCount, colRGBA, matRes, flags ) );
}


void Renderer::clearOverlays()
{
	_overlayBatches.resize( 0 );
}


void Renderer::drawOverlays( const string &shaderContext )
{
	uint32 numOverlayVerts = 0;
	if( !_overlayBatches.empty() )
		numOverlayVerts = _overlayBatches.back().firstVert + _overlayBatches.back().vertCount;
	
	if( numOverlayVerts == 0 ) return;
	
	// Upload overlay vertices
	gRDI->updateBufferData( _overlayVB, 0, MaxNumOverlayVerts * sizeof( OverlayVert ), _overlayVerts );

	gRDI->setVertexBuffer( 0, _overlayVB, 0, sizeof( OverlayVert ) );
	gRDI->setIndexBuffer( _quadIdxBuf, IDXFMT_16 );
	ASSERT( QuadIndexBufCount >= MaxNumOverlayVerts * 6 );

	float aspect = (float)_curCamera->_vpWidth / (float)_curCamera->_vpHeight;
	setupViewMatrices( Matrix4f(), Matrix4f::OrthoMat( 0, aspect, 1, 0, -1, 1 ) );
	
	MaterialResource *curMatRes = 0x0;
	
	for( size_t i = 0, s = _overlayBatches.size(); i < s; ++i )
	{
		OverlayBatch &ob = _overlayBatches[i];
		
		if( curMatRes != ob.materialRes )
		{
			if( !setMaterial( ob.materialRes, shaderContext ) )
			{
				// Unsuccessful material setting probably has destroyed the last setted material
				curMatRes = 0x0;
				continue;
			}
			gRDI->setVertexLayout( _vlOverlay );
			curMatRes = ob.materialRes;
		}
		
		if( _curShader->uni_olayColor >= 0 )
			gRDI->setShaderConst( _curShader->uni_olayColor, CONST_FLOAT4, ob.colRGBA );
		
		// Draw batch
		gRDI->drawIndexed( PRIM_TRILIST, ob.firstVert * 6/4, ob.vertCount * 6/4, ob.firstVert, ob.vertCount );
	}
}


// =================================================================================================
// Pipeline Functions
// =================================================================================================

void Renderer::bindPipeBuffer( uint32 rbObj, const string &sampler, uint32 bufIndex )
{
	if( rbObj == 0 )
	{
		// Clear buffer bindings
		_pipeSamplerBindings.resize( 0 );
	}
	else
	{
		// Check if binding is already existing
		for( size_t i = 0, s = _pipeSamplerBindings.size(); i < s; ++i )
		{
			if( strcmp( _pipeSamplerBindings[i].sampler, sampler.c_str() ) == 0 )
			{
				_pipeSamplerBindings[i].rbObj = rbObj;
				_pipeSamplerBindings[i].bufIndex = bufIndex;
				return;
			}
		}
		
		// Add binding
		_pipeSamplerBindings.push_back( PipeSamplerBinding() );
		size_t len = std::min( sampler.length(), (size_t)63 );
		strncpy_s( _pipeSamplerBindings.back().sampler, 63, sampler.c_str(), len );
		_pipeSamplerBindings.back().sampler[len] = '\0';
		_pipeSamplerBindings.back().rbObj = rbObj;
		_pipeSamplerBindings.back().bufIndex = bufIndex;
	}
}


void Renderer::clear( bool depth, bool buf0, bool buf1, bool buf2, bool buf3,
                      float r, float g, float b, float a )
{
	float clrColor[] = { r, g, b, a };

	gRDI->setBlendMode( false );  // Clearing floating point buffers causes problems when blending is enabled on Radeon 9600
	gRDI->setDepthMask( true );

	uint32 clearFlags = 0;
	if( depth ) clearFlags |= CLR_DEPTH;
	if( buf0 ) clearFlags |= CLR_COLOR_RT0;
	if( buf1 ) clearFlags |= CLR_COLOR_RT1;
	if( buf2 ) clearFlags |= CLR_COLOR_RT2;
	if( buf3 ) clearFlags |= CLR_COLOR_RT3;
	
	if( gRDI->_curRendBuf == 0x0 )
	{
		gRDI->setScissorRect( _curCamera->_vpX, _curCamera->_vpY, _curCamera->_vpWidth, _curCamera->_vpHeight );
		gRDI->setScissorTest( true );
	}
	
	gRDI->clear( clearFlags, clrColor, 1.f );
	gRDI->setScissorTest( false );
}


void Renderer::drawFSQuad( Resource *matRes, const string &shaderContext )
{
	if( matRes == 0x0 || matRes->getType() != ResourceTypes::Material ) return;

	setupViewMatrices( _curCamera->getViewMat(), Matrix4f::OrthoMat( 0, 1, 0, 1, -1, 1 ) );
	
	if( !setMaterial( (MaterialResource *)matRes, shaderContext ) ) return;

	gRDI->setVertexBuffer( 0, _vbFSPoly, 0, 12 );
	gRDI->setIndexBuffer( 0, IDXFMT_16 );
	gRDI->setVertexLayout( _vlPosOnly );

	gRDI->draw( PRIM_TRILIST, 0, 3 );
}


void Renderer::drawGeometry( const string &shaderContext, const string &theClass,
                             RenderingOrder::List order, int occSet )
{
	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), 0x0, order,
	                                  SceneNodeFlags::NoDraw , false, true );
	
	setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
	drawRenderables( shaderContext, theClass, false, &_curCamera->getFrustum(), 0x0, order, occSet );
}


void Renderer::drawLightGeometry( const string &shaderContext, const string &theClass,
                                  bool noShadows, RenderingOrder::List order, int occSet )
{
	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), 0x0, RenderingOrder::None,
	                                  SceneNodeFlags::NoDraw, true, false );
	
	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::FwdLightsGPUTime );
	if( Modules::config().gatherTimeStats ) timer->beginQuery( _frameID );
	
	for( size_t i = 0, s = Modules::sceneMan().getLightQueue().size(); i < s; ++i )
	{
		_curLight = (LightNode *)Modules::sceneMan().getLightQueue()[i];

		// Check if light is not visible
		if( _curCamera->getFrustum().cullFrustum( _curLight->getFrustum() ) ) continue;

		// Check if light is occluded
		if( occSet >= 0 )
		{
			if( occSet > (int)_curLight->_occQueries.size() - 1 )
			{
				_curLight->_occQueries.resize( occSet + 1, 0 );
				_curLight->_lastVisited.resize( occSet + 1, 0 );
			}
			if( _curLight->_occQueries[occSet] == 0 )
			{
				_curLight->_occQueries[occSet] = gRDI->createOcclusionQuery();
				_curLight->_lastVisited[occSet] = 0;
			}
			else
			{
				if( _curLight->_lastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					_curLight->_lastVisited[occSet] = Modules::renderer().getFrameID();
				
					Vec3f bbMin, bbMax;
					_curLight->getFrustum().calcAABB( bbMin, bbMax );
					
					// Check that viewer is outside light bounds
					if( nearestDistToAABB( _curCamera->getFrustum().getOrigin(), bbMin, bbMax ) > 0 )
					{
						Modules::renderer().pushOccProxy( 1, bbMin, bbMax, _curLight->_occQueries[occSet] );

						// Check query result from previous frame
						if( gRDI->getQueryResult( _curLight->_occQueries[occSet] ) < 1 )
						{
							continue;
						}
					}
				}
			}
		}
	
		// Update shadow map
		if( !noShadows && _curLight->_shadowMapCount > 0 )
		{
			timer->endQuery();
			GPUTimer *timerShadows = Modules::stats().getGPUTimer( EngineStats::ShadowsGPUTime );
			if( Modules::config().gatherTimeStats ) timerShadows->beginQuery( _frameID );

			updateShadowMap();
			setupShadowMap( false );

			timerShadows->endQuery();
			if( Modules::config().gatherTimeStats ) timer->beginQuery( _frameID );
		}
		else
		{
			setupShadowMap( true );
		}
		
		// Calculate light screen space position
		float bbx, bby, bbw, bbh;
		_curLight->calcScreenSpaceAABB( _curCamera->getProjMat() * _curCamera->getViewMat(),
		                                bbx, bby, bbw, bbh );

		// Set scissor rectangle
		if( bbx != 0 || bby != 0 || bbw != 1 || bbh != 1 )
		{
			gRDI->setScissorRect( ftoi_r( bbx * gRDI->_fbWidth ), ftoi_r( bby * gRDI->_fbHeight ),
			                      ftoi_r( bbw * gRDI->_fbWidth ), ftoi_r( bbh * gRDI->_fbHeight ) );
			gRDI->setScissorTest( true );
		}
		
		// Render
		Modules::sceneMan().updateQueues( _curCamera->getFrustum(), &_curLight->getFrustum(),
		                                  order, SceneNodeFlags::NoDraw, false, true );
		setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
		drawRenderables( shaderContext.empty() ? _curLight->_lightingContext : shaderContext,
		                 theClass, false, &_curCamera->getFrustum(),
		                 &_curLight->getFrustum(), order, occSet );
		Modules().stats().incStat( EngineStats::LightPassCount, 1 );

		// Reset
		gRDI->setScissorTest( false );
	}

	_curLight = 0x0;

	timer->endQuery();

	// Draw occlusion proxies
	if( occSet >= 0 )
	{
		setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
		Modules::renderer().drawOccProxies( OCCPROXYLIST_LIGHTS );
	}
}


void Renderer::drawLightShapes( const string &shaderContext, bool noShadows, int occSet )
{
	MaterialResource *curMatRes = 0x0;
	
	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), 0x0, RenderingOrder::None,
	                                  SceneNodeFlags::NoDraw, true, false );
	
	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::DefLightsGPUTime );
	if( Modules::config().gatherTimeStats ) timer->beginQuery( _frameID );
	
	for( size_t i = 0, s = Modules::sceneMan().getLightQueue().size(); i < s; ++i )
	{
		_curLight = (LightNode *)Modules::sceneMan().getLightQueue()[i];
		
		// Check if light is not visible
		if( _curCamera->getFrustum().cullFrustum( _curLight->getFrustum() ) ) continue;
		
		// Check if light is occluded
		if( occSet >= 0 )
		{
			if( occSet > (int)_curLight->_occQueries.size() - 1 )
			{
				_curLight->_occQueries.resize( occSet + 1, 0 );
				_curLight->_lastVisited.resize( occSet + 1, 0 );
			}
			if( _curLight->_occQueries[occSet] == 0 )
			{
				_curLight->_occQueries[occSet] = gRDI->createOcclusionQuery();
				_curLight->_lastVisited[occSet] = 0;
			}
			else
			{
				if( _curLight->_lastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					_curLight->_lastVisited[occSet] = Modules::renderer().getFrameID();
				
					Vec3f bbMin, bbMax;
					_curLight->getFrustum().calcAABB( bbMin, bbMax );
					
					// Check that viewer is outside light bounds
					if( nearestDistToAABB( _curCamera->getFrustum().getOrigin(), bbMin, bbMax ) > 0 )
					{
						Modules::renderer().pushOccProxy( 1, bbMin, bbMax, _curLight->_occQueries[occSet] );

						// Check query result from previous frame
						if( gRDI->getQueryResult( _curLight->_occQueries[occSet] ) < 1 )
						{
							continue;
						}
					}
				}
			}
		}
		
		// Update shadow map
		if( !noShadows && _curLight->_shadowMapCount > 0 )
		{	
			timer->endQuery();
			GPUTimer *timerShadows = Modules::stats().getGPUTimer( EngineStats::ShadowsGPUTime );
			if( Modules::config().gatherTimeStats ) timerShadows->beginQuery( _frameID );
			
			updateShadowMap();
			setupShadowMap( false );
			curMatRes = 0x0;
			
			timerShadows->endQuery();
			if( Modules::config().gatherTimeStats ) timer->beginQuery( _frameID );
		}
		else
		{
			setupShadowMap( true );
		}

		setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );

		if( curMatRes != _curLight->_materialRes )
		{
			if( !setMaterial( _curLight->_materialRes,
				              shaderContext.empty() ? _curLight->_lightingContext : shaderContext ) )
			{
				continue;
			}
			curMatRes = _curLight->_materialRes;
		}
		else
		{
			commitGeneralUniforms();
		}

		gRDI->setCullMode( RS_CULL_FRONT );
		gRDI->setDepthTest( false );

		if( _curLight->_fov < 180 )
		{
			float r = _curLight->_radius * tanf( degToRad( _curLight->_fov / 2 ) );
			drawCone( _curLight->_radius, r, _curLight->_absTrans );
		}
		else
		{
			drawSphere( _curLight->_absPos, _curLight->_radius );
		}

		Modules().stats().incStat( EngineStats::LightPassCount, 1 );

		// Reset
		gRDI->setCullMode( RS_CULL_BACK );
		gRDI->setDepthTest( true );
	}

	_curLight = 0x0;

	timer->endQuery();

	// Draw occlusion proxies
	if( occSet >= 0 )
	{
		setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
		Modules::renderer().drawOccProxies( OCCPROXYLIST_LIGHTS );
	}
}


// =================================================================================================
// Scene Node Rendering Functions
// =================================================================================================

void Renderer::drawRenderables( const string &shaderContext, const string &theClass, bool debugView,
                                const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order,
                                int occSet )
{
	ASSERT( _curCamera != 0x0 );
	
	const RenderQueue &renderQueue = Modules::sceneMan().getRenderQueue();
	uint32 queueSize = (uint32)renderQueue.size();
	if( queueSize == 0 ) return;

	// Set global render states
	if( Modules::config().wireframeMode && !Modules::config().debugViewMode )
	{
		gRDI->setCullMode( RS_CULL_NONE );
		gRDI->setFillMode( RS_FILL_WIREFRAME );
	}

	// Process all render queue items
	uint32 firstItem = 0, lastItem = 0;
	do
	{
		lastItem = firstItem;
		while( (lastItem + 1 < queueSize) && (renderQueue[firstItem].type == renderQueue[lastItem + 1].type) )
		{
			++lastItem;
		}
		
		for( uint32 i = 0, si = (uint32)_renderFuncRegistry.size(); i < si; ++i )
		{
			if( _renderFuncRegistry[i].nodeType == renderQueue[firstItem].type )
			{
				_renderFuncRegistry[i].renderFunc(
					firstItem, lastItem, shaderContext, theClass, debugView, frust1, frust2, order, occSet );
				break;
			}
		}

		firstItem = lastItem + 1;
	} while( firstItem < queueSize );

	// Reset states
	if( Modules::config().wireframeMode && !Modules::config().debugViewMode )
	{
		gRDI->setFillMode( RS_FILL_SOLID );
	}
}


void Renderer::drawMeshes( uint32 firstItem, uint32 lastItem, const string &shaderContext, const string &theClass,
                           bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order,
                           int occSet )
{
	if( frust1 == 0x0 ) return;
	
	const RenderQueue &renderQueue = Modules::sceneMan().getRenderQueue();
	GeometryResource *curGeoRes = 0x0;
	MaterialResource *curMatRes = 0x0;

	// Loop over mesh queue
	for( size_t i = firstItem; i <= lastItem; ++i )
	{
		MeshNode *meshNode = (MeshNode *)renderQueue[i].node;
		ModelNode *modelNode = meshNode->getParentModel();
		
		// Check that mesh is valid
		if( modelNode->getGeometryResource() == 0x0 )
			continue;
		if( meshNode->getBatchStart() + meshNode->getBatchCount() > modelNode->getGeometryResource()->_indexCount )
			continue;
		
		bool modelChanged = true;
		uint32 queryObj = 0;

		// Occlusion culling
		if( occSet >= 0 )
		{
			if( occSet > (int)meshNode->_occQueries.size() - 1 )
			{
				meshNode->_occQueries.resize( occSet + 1, 0 );
				meshNode->_lastVisited.resize( occSet + 1, 0 );
			}
			if( meshNode->_occQueries[occSet] == 0 )
			{
				queryObj = gRDI->createOcclusionQuery();
				meshNode->_occQueries[occSet] = queryObj;
				meshNode->_lastVisited[occSet] = 0;
			}
			else
			{
				if( meshNode->_lastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					meshNode->_lastVisited[occSet] = Modules::renderer().getFrameID();
				
					// Check query result (viewer must be outside of bounding box)
					if( nearestDistToAABB( frust1->getOrigin(), meshNode->getBBox().min,
					                       meshNode->getBBox().max ) > 0 &&
						gRDI->getQueryResult( meshNode->_occQueries[occSet] ) < 1 )
					{
						Modules::renderer().pushOccProxy( 0, meshNode->getBBox().min, meshNode->getBBox().max,
						                                  meshNode->_occQueries[occSet] );
						continue;
					}
					else
						queryObj = meshNode->_occQueries[occSet];
				}
			}
		}
		
		// Bind geometry
		if( curGeoRes != modelNode->getGeometryResource() )
		{
			curGeoRes = modelNode->getGeometryResource();
			ASSERT( curGeoRes != 0x0 );
		
			// Indices
			gRDI->setIndexBuffer( curGeoRes->getIndexBuf(),
			                      curGeoRes->_16BitIndices ? IDXFMT_16 : IDXFMT_32 );

			// Vertices
			uint32 posVBuf = curGeoRes->getPosVBuf();
			uint32 tanVBuf = curGeoRes->getTanVBuf();
			uint32 staticVBuf = curGeoRes->getStaticVBuf();
			
			gRDI->setVertexBuffer( 0, posVBuf, 0, sizeof( Vec3f ) );
			gRDI->setVertexBuffer( 1, tanVBuf, 0, sizeof( VertexDataTan ) );
			gRDI->setVertexBuffer( 2, tanVBuf, sizeof( Vec3f ), sizeof( VertexDataTan ) );
			gRDI->setVertexBuffer( 3, staticVBuf, 0, sizeof( VertexDataStatic ) );
		}

		gRDI->setVertexLayout( Modules::renderer()._vlModel );
		
		ShaderCombination *prevShader = Modules::renderer().getCurShader();
		
		if( !debugView )
		{
			if( !meshNode->getMaterialRes()->isOfClass( theClass ) ) continue;
			
			// Set material
			if( curMatRes != meshNode->getMaterialRes() )
			{
				if( !Modules::renderer().setMaterial( meshNode->getMaterialRes(), shaderContext ) )
				{	
					curMatRes = 0x0;
					continue;
				}
				curMatRes = meshNode->getMaterialRes();
			}
		}
		else
		{
			Modules::renderer().setShaderComb( &Modules::renderer()._defColorShader );
			Modules::renderer().commitGeneralUniforms();
			
			uint32 curLod = meshNode->getLodLevel();
			Vec4f color;
			if( curLod == 0 ) color = Vec4f( 0.5f, 0.75f, 1, 1 );
			else if( curLod == 1 ) color = Vec4f( 0.25f, 0.75, 0.75f, 1 );
			else if( curLod == 2 ) color = Vec4f( 0.25f, 0.75, 0.5f, 1 );
			else if( curLod == 3 ) color = Vec4f( 0.5f, 0.5f, 0.25f, 1 );
			else color = Vec4f( 0.75f, 0.5, 0.25f, 1 );

			// Darken models with skeleton so that bones are more noticable
			if( !modelNode->_jointList.empty() ) color = color * 0.3f;

			gRDI->setShaderConst( Modules::renderer()._defColShader_color, CONST_FLOAT4, &color.x );
		}

		ShaderCombination *curShader = Modules::renderer().getCurShader();
		
		if( modelChanged || curShader != prevShader )
		{
			// Skeleton
			if( curShader->uni_skinMatRows >= 0 && !modelNode->_skinMatRows.empty() )
			{
				// Note:	OpenGL 2.1 supports mat4x3 but it is internally realized as mat4 on most
				//			hardware so it would require 4 instead of 3 uniform slots per joint
				
				gRDI->setShaderConst( curShader->uni_skinMatRows, CONST_FLOAT4,
				                      &modelNode->_skinMatRows[0], (int)modelNode->_skinMatRows.size() );
			}

			modelChanged = false;
		}

		// World transformation
		if( curShader->uni_worldMat >= 0 )
		{
			gRDI->setShaderConst( curShader->uni_worldMat, CONST_FLOAT44, &meshNode->_absTrans.x[0] );
		}
		if( curShader->uni_worldNormalMat >= 0 )
		{
			// TODO: Optimize this
			Matrix4f normalMat4 = meshNode->_absTrans.inverted().transposed();
			float normalMat[9] = { normalMat4.x[0], normalMat4.x[1], normalMat4.x[2],
			                       normalMat4.x[4], normalMat4.x[5], normalMat4.x[6],
			                       normalMat4.x[8], normalMat4.x[9], normalMat4.x[10] };
			gRDI->setShaderConst( curShader->uni_worldNormalMat, CONST_FLOAT33, normalMat );
		}
		if( curShader->uni_nodeId >= 0 )
		{
			float id = (float)meshNode->getHandle();
			gRDI->setShaderConst( curShader->uni_nodeId, CONST_FLOAT, &id );
		}
		if( curShader->uni_customInstData >= 0 )
		{
			gRDI->setShaderConst( curShader->uni_customInstData, CONST_FLOAT4,
			                      &modelNode->_customInstData[0].x, ModelCustomVecCount );
		}

		if( queryObj )
			gRDI->beginQuery( queryObj );
		
		// Render
		gRDI->drawIndexed( PRIM_TRILIST, meshNode->getBatchStart(), meshNode->getBatchCount(),
		                   meshNode->getVertRStart(), meshNode->getVertREnd() - meshNode->getVertRStart() + 1 );
		Modules::stats().incStat( EngineStats::BatchCount, 1 );
		Modules::stats().incStat( EngineStats::TriCount, meshNode->getBatchCount() / 3.0f );

		if( queryObj )
			gRDI->endQuery( queryObj );
	}

	// Draw occlusion proxies
	if( occSet >= 0 )
		Modules::renderer().drawOccProxies( OCCPROXYLIST_RENDERABLES );

	gRDI->setVertexLayout( 0 );
}


void Renderer::drawParticles( uint32 firstItem, uint32 lastItem, const string &shaderContext, const string &theClass,
                              bool debugView, const Frustum *frust1, const Frustum * /*frust2*/, RenderingOrder::List /*order*/,
                              int occSet )
{
	if( frust1 == 0x0 || Modules::renderer().getCurCamera() == 0x0 ) return;
	if( debugView ) return;  // Don't render particles in debug view

	const RenderQueue &renderQueue = Modules::sceneMan().getRenderQueue();
	MaterialResource *curMatRes = 0x0;

	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::ParticleGPUTime );
	if( Modules::config().gatherTimeStats ) timer->beginQuery( Modules::renderer().getFrameID() );

	// Bind particle geometry
	gRDI->setVertexBuffer( 0, Modules::renderer().getParticleVBO(), 0, sizeof( ParticleVert ) );
	gRDI->setIndexBuffer( Modules::renderer().getQuadIdxBuf(), IDXFMT_16 );
	ASSERT( QuadIndexBufCount >= ParticlesPerBatch * 6 );

	// Loop through emitter queue
	for( uint32 i = firstItem; i <= lastItem; ++i )
	{
		EmitterNode *emitter = (EmitterNode *)renderQueue[i].node;
		
		if( emitter->_particleCount == 0 ) continue;
		if( !emitter->_materialRes->isOfClass( theClass ) ) continue;
		
		// Occlusion culling
		uint32 queryObj = 0;
		if( occSet >= 0 )
		{
			if( occSet > (int)emitter->_occQueries.size() - 1 )
			{
				emitter->_occQueries.resize( occSet + 1, 0 );
				emitter->_lastVisited.resize( occSet + 1, 0 );
			}
			if( emitter->_occQueries[occSet] == 0 )
			{
				queryObj = gRDI->createOcclusionQuery();
				emitter->_occQueries[occSet] = queryObj;
				emitter->_lastVisited[occSet] = 0;
			}
			else
			{
				if( emitter->_lastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					emitter->_lastVisited[occSet] = Modules::renderer().getFrameID();
				
					// Check query result (viewer must be outside of bounding box)
					if( nearestDistToAABB( frust1->getOrigin(), emitter->getBBox().min,
					                       emitter->getBBox().max ) > 0 &&
						gRDI->getQueryResult( emitter->_occQueries[occSet] ) < 1 )
					{
						Modules::renderer().pushOccProxy( 0, emitter->getBBox().min,
							emitter->getBBox().max, emitter->_occQueries[occSet] );
						continue;
					}
					else
						queryObj = emitter->_occQueries[occSet];
				}
			}
		}
		
		// Set material
		if( curMatRes != emitter->_materialRes )
		{
			if( !Modules::renderer().setMaterial( emitter->_materialRes, shaderContext ) ) continue;
			curMatRes = emitter->_materialRes;
		}

		// Set vertex layout
		gRDI->setVertexLayout( Modules::renderer()._vlParticle );
		
		if( queryObj )
			gRDI->beginQuery( queryObj );
		
		// Shader uniforms
		ShaderCombination *curShader = Modules::renderer().getCurShader();
		if( curShader->uni_nodeId >= 0 )
		{
			float id = (float)emitter->getHandle();
			gRDI->setShaderConst( curShader->uni_nodeId, CONST_FLOAT, &id );
		}

		// Divide particles in batches and render them
		for( uint32 j = 0; j < emitter->_particleCount / ParticlesPerBatch; ++j )
		{
			// Check if batch needs to be rendered
			bool allDead = true;
			for( uint32 k = 0; k < ParticlesPerBatch; ++k )
			{
				if( emitter->_particles[j*ParticlesPerBatch + k].life > 0 )
				{
					allDead = false;
					break;
				}
			}
			if( allDead ) continue;

			// Render batch
			if( curShader->uni_parPosArray >= 0 )
				gRDI->setShaderConst( curShader->uni_parPosArray, CONST_FLOAT3,
				                      (float *)emitter->_parPositions + j*ParticlesPerBatch*3, ParticlesPerBatch );
			if( curShader->uni_parSizeAndRotArray >= 0 )
				gRDI->setShaderConst( curShader->uni_parSizeAndRotArray, CONST_FLOAT2,
				                      (float *)emitter->_parSizesANDRotations + j*ParticlesPerBatch*2, ParticlesPerBatch );
			if( curShader->uni_parColorArray >= 0 )
				gRDI->setShaderConst( curShader->uni_parColorArray, CONST_FLOAT4,
				                      (float *)emitter->_parColors + j*ParticlesPerBatch*4, ParticlesPerBatch );

			gRDI->drawIndexed( PRIM_TRILIST, 0, ParticlesPerBatch * 6, 0, ParticlesPerBatch * 4 );
			Modules::stats().incStat( EngineStats::BatchCount, 1 );
			Modules::stats().incStat( EngineStats::TriCount, ParticlesPerBatch * 2.0f );
		}

		uint32 count = emitter->_particleCount % ParticlesPerBatch;
		if( count > 0 )
		{
			uint32 offset = (emitter->_particleCount / ParticlesPerBatch) * ParticlesPerBatch;
			
			// Check if batch needs to be rendered
			bool allDead = true;
			for( uint32 k = 0; k < count; ++k )
			{
				if( emitter->_particles[offset + k].life > 0 )
				{
					allDead = false;
					break;
				}
			}
			
			if( !allDead )
			{
				// Render batch
				if( curShader->uni_parPosArray >= 0 )
					gRDI->setShaderConst( curShader->uni_parPosArray, CONST_FLOAT3,
					                      (float *)emitter->_parPositions + offset*3, count );
				if( curShader->uni_parSizeAndRotArray >= 0 )
					gRDI->setShaderConst( curShader->uni_parSizeAndRotArray, CONST_FLOAT2,
					                      (float *)emitter->_parSizesANDRotations + offset*2, count );
				if( curShader->uni_parColorArray >= 0 )
					gRDI->setShaderConst( curShader->uni_parColorArray, CONST_FLOAT4,
					                      (float *)emitter->_parColors + offset*4, count );
				
				gRDI->drawIndexed( PRIM_TRILIST, 0, count * 6, 0, count * 4 );
				Modules::stats().incStat( EngineStats::BatchCount, 1 );
				Modules::stats().incStat( EngineStats::TriCount, count * 2.0f );
			}
		}

		if( queryObj )
			gRDI->endQuery( queryObj );
	}

	timer->endQuery();

	// Draw occlusion proxies
	if( occSet >= 0 )
		Modules::renderer().drawOccProxies( OCCPROXYLIST_RENDERABLES );
	
	gRDI->setVertexLayout( 0 );
}


// =================================================================================================
// Main Rendering Functions
// =================================================================================================

void Renderer::render( CameraNode *camNode )
{
	_curCamera = camNode;
	if( _curCamera == 0x0 ) return;

	// Build sampler anisotropy mask from anisotropy value
	int maxAniso = Modules::config().maxAnisotropy;
	if( maxAniso <= 1 ) _maxAnisoMask = SS_ANISO1;
	else if( maxAniso <= 2 ) _maxAnisoMask = SS_ANISO2;
	else if( maxAniso <= 4 ) _maxAnisoMask = SS_ANISO4;
	else if( maxAniso <= 8 ) _maxAnisoMask = SS_ANISO8;
	else _maxAnisoMask = SS_ANISO16;
	gRDI->beginRendering();
	gRDI->setViewport( _curCamera->_vpX, _curCamera->_vpY, _curCamera->_vpWidth, _curCamera->_vpHeight );
	if( Modules::config().debugViewMode || _curCamera->_pipelineRes == 0x0 )
	{
		renderDebugView();
		finishRendering();
		return;
	}
	
	// Initialize
	gRDI->_outputBufferIndex = _curCamera->_outputBufferIndex;
	if( _curCamera->_outputTex != 0x0 )
		gRDI->setRenderBuffer( _curCamera->_outputTex->getRBObject() );
	else 
		gRDI->setRenderBuffer( 0 );

	// Process pipeline commands
	for( uint32 i = 0; i < _curCamera->_pipelineRes->_stages.size(); ++i )
	{
		PipelineStage &stage = _curCamera->_pipelineRes->_stages[i];
		if( !stage.enabled ) continue;
		_curStageMatLink = stage.matLink;
		
		for( uint32 j = 0; j < stage.commands.size(); ++j )
		{
			PipelineCommand &pc = stage.commands[j];
			RenderTarget *rt;

			switch( pc.command )
			{
			case PipelineCommands::SwitchTarget:
				// Unbind all textures
				bindPipeBuffer( 0x0, "", 0 );
				
				// Bind new render target
				rt = (RenderTarget *)pc.params[0].getPtr();
				_curRenderTarget = rt;

				if( rt != 0x0 )
				{
					RDIRenderBuffer &rendBuf = gRDI->_rendBufs.getRef( rt->rendBuf );
					gRDI->_outputBufferIndex = _curCamera->_outputBufferIndex;
					gRDI->setViewport( 0, 0, rendBuf.width, rendBuf.height );
					gRDI->setRenderBuffer( rt->rendBuf );
				}
				else
				{
					gRDI->setViewport( _curCamera->_vpX, _curCamera->_vpY, _curCamera->_vpWidth, _curCamera->_vpHeight );
					gRDI->setRenderBuffer( _curCamera->_outputTex != 0x0 ?
					                       _curCamera->_outputTex->getRBObject() : 0 );
				}
				break;

			case PipelineCommands::BindBuffer:
				rt = (RenderTarget *)pc.params[0].getPtr();
				bindPipeBuffer( rt->rendBuf, pc.params[1].getString(), (uint32)pc.params[2].getInt() );
				break;

			case PipelineCommands::UnbindBuffers:
				bindPipeBuffer( 0x0, "", 0 );
				break;

			case PipelineCommands::ClearTarget:
				clear( pc.params[0].getBool(), pc.params[1].getBool(), pc.params[2].getBool(),
				       pc.params[3].getBool(), pc.params[4].getBool(), pc.params[5].getFloat(),
				       pc.params[6].getFloat(), pc.params[7].getFloat(), pc.params[8].getFloat() );
				break;

			case PipelineCommands::DrawGeometry:
				drawGeometry( pc.params[0].getString(), pc.params[1].getString(),
				              (RenderingOrder::List)pc.params[2].getInt(), _curCamera->_occSet );
				break;

			case PipelineCommands::DrawOverlays:
				drawOverlays( pc.params[0].getString() );
				break;

			case PipelineCommands::DrawQuad:
				drawFSQuad( pc.params[0].getResource(), pc.params[1].getString() );
			break;

			case PipelineCommands::DoForwardLightLoop:
				drawLightGeometry( pc.params[0].getString(), pc.params[1].getString(),
				                   pc.params[2].getBool(), (RenderingOrder::List)pc.params[3].getInt(),
					_curCamera->_occSet );
				break;

			case PipelineCommands::DoDeferredLightLoop:
				drawLightShapes( pc.params[0].getString(), pc.params[1].getBool(), _curCamera->_occSet );
				break;

			case PipelineCommands::SetUniform:
				if( pc.params[0].getResource() && pc.params[0].getResource()->getType() == ResourceTypes::Material )
				{
					((MaterialResource *)pc.params[0].getResource())->setUniform( pc.params[1].getString(),
						pc.params[2].getFloat(), pc.params[3].getFloat(),
						pc.params[4].getFloat(), pc.params[5].getFloat() );
				}
				break;
			}
		}
	}
	
	finishRendering();
}


void Renderer::finalizeFrame()
{
	++_frameID;
	
	// Reset frame timer
	Timer *timer = Modules::stats().getTimer( EngineStats::FrameTime );
	ASSERT( timer != 0x0 );
	Modules::stats().getStat( EngineStats::FrameTime, true );  // Reset
	Modules::stats().incStat( EngineStats::FrameTime, timer->getElapsedTimeMS() );
	timer->reset();
}


void Renderer::renderDebugView()
{
	float color[4] = { 0 };
	
	gRDI->setRenderBuffer( 0 );
	setMaterial( 0x0, "" );
	gRDI->setFillMode( RS_FILL_WIREFRAME );

	gRDI->clear( CLR_DEPTH | CLR_COLOR_RT0 );

	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), 0x0, RenderingOrder::None,
	                                  SceneNodeFlags::NoDraw, true, true );

	// Draw renderable nodes as wireframe
	setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
	drawRenderables( "", "", true, &_curCamera->getFrustum(), 0x0, RenderingOrder::None, -1 );

	// Draw bounding boxes
	gRDI->setCullMode( RS_CULL_NONE );
	setMaterial( 0x0, "" );
	setShaderComb( &_defColorShader );
	commitGeneralUniforms();
	gRDI->setShaderConst( _defColorShader.uni_worldMat, CONST_FLOAT44, &Matrix4f().x[0] );
	color[0] = 0.4f; color[1] = 0.4f; color[2] = 0.4f; color[3] = 1;
	gRDI->setShaderConst( Modules::renderer()._defColShader_color, CONST_FLOAT4, color );
	for( uint32 i = 0, s = (uint32)Modules::sceneMan().getRenderQueue().size(); i < s; ++i )
	{
		SceneNode *sn = Modules::sceneMan().getRenderQueue()[i].node;
		
		drawAABB( sn->_bBox.min, sn->_bBox.max );
	}
	gRDI->setCullMode( RS_CULL_BACK );

	// Draw light volumes
	gRDI->setFillMode( RS_FILL_SOLID );
	gRDI->setBlendMode( true, BS_BLEND_SRC_ALPHA, BS_BLEND_ONE );
	gRDI->setCullMode( RS_CULL_FRONT );
	color[0] = 1; color[1] = 1; color[2] = 0; color[3] = 0.25f;
	gRDI->setShaderConst( Modules::renderer()._defColShader_color, CONST_FLOAT4, color );
	for( size_t i = 0, s = Modules::sceneMan().getLightQueue().size(); i < s; ++i )
	{
		LightNode *lightNode = (LightNode *)Modules::sceneMan().getLightQueue()[i];
		
		if( lightNode->_fov < 180 )
		{
			float r = lightNode->_radius * tanf( degToRad( lightNode->_fov / 2 ) );
			drawCone( lightNode->_radius, r, lightNode->_absTrans );
		}
		else
		{
			drawSphere( lightNode->_absPos, lightNode->_radius );
		}
	}
	gRDI->setCullMode( RS_CULL_BACK );
	gRDI->setBlendMode( false );
}


void Renderer::finishRendering()
{
	gRDI->setRenderBuffer( 0 );
	setMaterial( 0x0, "" );
	gRDI->resetStates();
}

}  // namespace
