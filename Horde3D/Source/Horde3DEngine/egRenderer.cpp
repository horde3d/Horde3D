// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
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
#include "egComputeNode.h"
#include <cstring>

#include "utDebug.h"

#if defined ( DESKTOP_OPENGL_AVAILABLE ) && !defined( H3D_USE_GLES3 )
#	if defined( H3D_USE_GL2 )
#		include "egRendererBaseGL2.h"
#	endif
#	if defined( H3D_USE_GL4 )
#		include "egRendererBaseGL4.h"
#	endif
#else
#	include "egRendererBaseGLES3.h"
#endif

// Constants
constexpr int defaultCameraView = 0;

namespace Horde3D {

using namespace std;

Renderer::Renderer()
{
	_scratchBuf = 0x0;
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
	_curStageMatLink = 0;
	_maxAnisoMask = 0;
	_smSize = 0;
	_shadowRB = 0;
	_vlPosOnly = 0;
	_vlModel = 0;
	_vlParticle = 0;

	_particleGeo = 0;
	_cubeGeo = 0;
	_sphereGeo = 0;
	_coneGeo = 0;
	_FSPolyGeo = 0;

	// reserve memory for occlusion culling proxies
	_occProxies[ 0 ].reserve( 200 ); // meshes
	_occProxies[ 1 ].reserve( 100 ); // lights

	// create default engine uniforms that will be automatically searched for in every shader
	_engineUniforms.reserve( 64 );

	// reserve memory for shadow parameters
	_shadowParams.reserve( 128 );

	// General uniforms
//	registerEngineUniform( "shadowMap" );

	// Misc general uniforms
	_uni.frameBufSize = registerEngineUniform( "frameBufSize" );

	// View/projection uniforms
	_uni.viewMat = registerEngineUniform( "viewMat" );
	_uni.viewMatInv = registerEngineUniform( "viewMatInv" );
	_uni.projMat = registerEngineUniform( "projMat" );
	_uni.viewProjMat = registerEngineUniform( "viewProjMat" );
	_uni.viewProjMatInv = registerEngineUniform( "viewProjMatInv" );
	_uni.viewerPos = registerEngineUniform( "viewerPos" );

	// Per-instance uniforms
	_uni.worldMat = registerEngineUniform( "worldMat" );
	_uni.worldNormalMat = registerEngineUniform( "worldNormalMat" );
	_uni.nodeId = registerEngineUniform( "nodeId" );
	_uni.customInstData = registerEngineUniform( "customInstData[0]" );
	_uni.skinMatRows = registerEngineUniform( "skinMatRows[0]" );

	// Lighting uniforms
	_uni.lightPos = registerEngineUniform( "lightPos" );
	_uni.lightDir = registerEngineUniform( "lightDir" );
	_uni.lightColor = registerEngineUniform( "lightColor" );
	_uni.shadowSplitDists = registerEngineUniform( "shadowSplitDists" );
	_uni.shadowMats = registerEngineUniform( "shadowMats" );
	_uni.shadowMapSize = registerEngineUniform( "shadowMapSize" );
	_uni.shadowBias = registerEngineUniform( "shadowBias" );

	// Particle-specific uniforms
	_uni.parPosArray = registerEngineUniform( "parPosArray" );
	_uni.parSizeAndRotArray = registerEngineUniform( "parSizeAndRotArray" );
	_uni.parColorArray = registerEngineUniform( "parColorArray" );

	_renderDevice = 0x0;
}


Renderer::~Renderer()
{
	if ( _renderDevice )
	{
		releaseShadowRB();
		_renderDevice->destroyTexture( _defShadowMap );
		releaseShaderComb( _defColorShader );

		_renderDevice->destroyGeometry( _particleGeo );
		_renderDevice->destroyGeometry( _cubeGeo );
		_renderDevice->destroyGeometry( _sphereGeo );
		_renderDevice->destroyGeometry( _coneGeo );
		_renderDevice->destroyGeometry( _FSPolyGeo );

		releaseRenderDevice();
	}
	
	delete[] _scratchBuf;
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


int Renderer::registerEngineUniform( const char *uniName )
{
	EngineUniform uni( uniName );
	auto it = std::find( _engineUniforms.begin(), _engineUniforms.end(), uni );
	if ( it != _engineUniforms.end() )
	{
		return ( int ) ( it - _engineUniforms.begin() ); // get index of the already registered uniform
	} 
	else
	{
		_engineUniforms.emplace_back( uni );

		return ( int) ( _engineUniforms.size() - 1 );
	}
}


int Renderer::getEngineUniform( const char *uniName )
{
	EngineUniform uni( uniName );
	auto it = std::find( _engineUniforms.begin(), _engineUniforms.end(), uni );
	if ( it != _engineUniforms.end() )
	{
		return ( int ) ( it - _engineUniforms.begin() ); // get index of the already registered uniform
	}

	return -1;
}


unsigned char * Renderer::useScratchBuf( uint32 minSize, uint32 alignment )
{
	if( _scratchBufSize < minSize )
	{
		delete[] _scratchBuf;

		uint32 padding = alignment > 1 ? alignment - 1 : 0;
		_scratchBuf = new unsigned char[ minSize + padding ];
		_scratchBufSize = minSize;
	}

	if ( alignment > 1 )
	{
		return _scratchBuf + ( size_t ) _scratchBuf % alignment;
	} 
	else
	{
		return _scratchBuf;
	}
}


bool Renderer::init( RenderBackendType::List type )
{
	if ( _renderDevice == 0x0 ) _renderDevice = createRenderDevice( type );
	if ( !_renderDevice ) return false;

	// Init Render Device Interface
	if( !_renderDevice->init() ) 
	{
		releaseRenderDevice();

		if ( type == RenderBackendType::OpenGL4 )
		{
			// try to use legacy OpenGL renderer backend
			_renderDevice = createRenderDevice( RenderBackendType::OpenGL2 );
			if ( !_renderDevice ) return false;

			if ( !_renderDevice->init() )
			{
				releaseRenderDevice();
				return false;
			}
			type = RenderBackendType::OpenGL2;
		}
		else
			return false;
	}

	// Check capabilities
	if( !_renderDevice->getCaps().texFloat )
		Modules::log().writeWarning( "Renderer: No floating point texture support available" );
	if( !_renderDevice->getCaps().texNPOT )
		Modules::log().writeWarning( "Renderer: No non-Power-of-two texture support available" );
	if( !_renderDevice->getCaps().rtMultisampling )
		Modules::log().writeWarning( "Renderer: No multisampling for render targets available" );
	
	// Create vertex layouts
	VertexLayoutAttrib attribsPosOnly[1] = {
		{"vertPos", 0, 3, 0}
	};
	_vlPosOnly = _renderDevice->registerVertexLayout( 1, attribsPosOnly );

	VertexLayoutAttrib attribsModel[7] = {
		{"vertPos", 0, 3, 0},
		{"normal", 1, 3, 0},
		{"tangent", 2, 4, 0},
		{"joints", 3, 4, 8},
		{"weights", 3, 4, 24},
		{"texCoords0", 3, 2, 0},
		{"texCoords1", 3, 2, 40}
	};
	_vlModel = _renderDevice->registerVertexLayout( 7, attribsModel );

	VertexLayoutAttrib attribsParticle[2] = {
		{"texCoords0", 0, 2, 0},
		{"parIdx", 0, 1, 8}
	};
	_vlParticle = _renderDevice->registerVertexLayout( 2, attribsParticle );
	
	// Upload default shaders
	if ( !createShaderComb( _defColorShader, _renderDevice->getDefaultVSCode(), _renderDevice->getDefaultFSCode(), 0, 0, 0, 0 ) )
	{
		Modules::log().writeError( "Failed to compile default shaders" );
		return false;
	}

	// Cache common uniforms
	_defColShader_color = _renderDevice->getShaderConstLoc( _defColorShader.shaderObj, "color" );
	
	// Create shadow map render target
	if( !createShadowRB( Modules::config().shadowMapSize, Modules::config().shadowMapSize ) )
	{
		Modules::log().writeError( "Failed to create shadow map" );
		return false;
	}

	// Create default shadow map
	float shadowTex[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	_defShadowMap = _renderDevice->createTexture( TextureTypes::Tex2D, 4, 4, 1, TextureFormats::DEPTH, 0, false, false, false );
	_renderDevice->uploadTextureData( _defShadowMap, 0, 0, shadowTex );

	// Create index buffer used for drawing particles
	uint16 *quadIndices = new uint16[ QuadIndexBufCount ];
	for( uint32 i = 0; i < QuadIndexBufCount / 6; ++i )
	{
		quadIndices[i*6+0] = i * 4 + 0; quadIndices[i*6+1] = i * 4 + 1; quadIndices[i*6+2] = i * 4 + 2;
		quadIndices[i*6+3] = i * 4 + 2; quadIndices[i*6+4] = i * 4 + 3; quadIndices[i*6+5] = i * 4 + 0;
	}
	_quadIdxBuf = _renderDevice->createIndexBuffer( QuadIndexBufCount * sizeof( uint16 ), quadIndices );
	delete[] quadIndices; quadIndices = 0x0;
	
	// Create particle geometry array
	ParticleVert v0( 0, 0 );
	ParticleVert v1( 1, 0 );
	ParticleVert v2( 1, 1 );
	ParticleVert v3( 0, 1 );
	
	ParticleVert *parVerts = new ParticleVert[ ParticlesPerBatch * 4 ];
	for( uint32 i = 0; i < ParticlesPerBatch; ++i )
	{
		parVerts[ i * 4 + 0 ] = v0; parVerts[ i * 4 + 0 ].index = ( float ) i;
		parVerts[ i * 4 + 1 ] = v1; parVerts[ i * 4 + 1 ].index = ( float ) i;
		parVerts[ i * 4 + 2 ] = v2; parVerts[ i * 4 + 2 ].index = ( float ) i;
		parVerts[ i * 4 + 3 ] = v3; parVerts[ i * 4 + 3 ].index = ( float ) i;
	}

	_particleGeo = _renderDevice->beginCreatingGeometry( _vlParticle );
	_particleVBO = _renderDevice->createVertexBuffer( ParticlesPerBatch * 4 * sizeof( ParticleVert ), ( float * ) parVerts );
	_renderDevice->setGeomVertexParams( _particleGeo, _particleVBO, 0, 0, sizeof( ParticleVert ) );
	_renderDevice->setGeomIndexParams( _particleGeo, _quadIdxBuf, IDXFMT_16 );
	_renderDevice->finishCreatingGeometry( _particleGeo );

	delete[] parVerts; parVerts = 0x0;

	// Create unit primitives
	createPrimitives();

	// Init scratch buffer with some default size
	useScratchBuf( 4 * 1024*1024, 16 );

	// Reset states
	finishRendering();

	// Start frame timer
	Timer *timer = Modules::stats().getTimer( EngineStats::FrameTime );
	ASSERT( timer != 0x0 );
	timer->setEnabled( true );
	
	// Save render device type for later use, particularly by shaders
	_renderDeviceType = type;

	return true;
}


void Renderer::initStates()
{
	_renderDevice->initStates();
}

RenderDeviceInterface *Renderer::createRenderDevice( int type )
{
	switch ( type )
	{
#if defined( DESKTOP_OPENGL_AVAILABLE ) && defined ( H3D_USE_GL4 ) 
		case RenderBackendType::OpenGL4:
		{
			return new RDI_GL4::RenderDeviceGL4();
		}
#endif
#if defined( DESKTOP_OPENGL_AVAILABLE ) && defined ( H3D_USE_GL2 )
		case RenderBackendType::OpenGL2:
		{
			return new RDI_GL2::RenderDeviceGL2();
		}
#endif
#if defined ( H3D_USE_GLES3 )	
		case RenderBackendType::OpenGLES3:
		{
			return new RDI_GLES3::RenderDeviceGLES3();
		}
#endif
		default:
			Modules::log().writeError( "Incorrect render interface type or type not specified. Renderer cannot be initialized." );
			break;
	}

	return 0;
}

void Renderer::releaseRenderDevice()
{
	delete _renderDevice; _renderDevice = 0;
}

// =================================================================================================
// Misc Helper Functions
// =================================================================================================

void Renderer::setupViewMatrices( const Matrix4f &viewMat, const Matrix4f &projMat )
{
	// Note: The viewing matrices should be set before a material is set, otherwise the general
	//       uniforms need to be committed manually
	
	_viewMat = viewMat;
	_viewMatInv = viewMat.inverted();
	_projMat = projMat;
	_viewProjMat = projMat * viewMat;
	_viewProjMatInv = _viewProjMat.inverted();

	++_curShaderUpdateStamp;
}


uint32 Renderer::getDefaultVertexLayout( DefaultVertexLayouts::List vl ) const
{
	switch ( vl )
	{
		case DefaultVertexLayouts::Position:
			return _vlPosOnly;
			break;
		case DefaultVertexLayouts::Particle:
			return _vlParticle;
			break;
		case DefaultVertexLayouts::Model:
			return _vlModel;
			break;
		default:
			break;
	}

	return 0;
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

	_cubeGeo = _renderDevice->beginCreatingGeometry( _vlPosOnly );
	_vbCube = _renderDevice->createVertexBuffer( 8 * 3 * sizeof( float ), cubeVerts );
	_ibCube = _renderDevice->createIndexBuffer( 36 * sizeof( uint16 ), cubeInds );
	_renderDevice->setGeomVertexParams( _cubeGeo, _vbCube, 0, 0, 12 );
	_renderDevice->setGeomIndexParams( _cubeGeo, _ibCube, IDXFMT_16 );
	_renderDevice->finishCreatingGeometry( _cubeGeo );

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

	_sphereGeo = _renderDevice->beginCreatingGeometry( _vlPosOnly );
	_vbSphere = _renderDevice->createVertexBuffer( 126 * sizeof( Vec3f ), spVerts );
	_ibSphere = _renderDevice->createIndexBuffer( 128 * 3 * sizeof( uint16 ), spInds );
	_renderDevice->setGeomVertexParams( _sphereGeo, _vbSphere, 0, 0, 12 );
	_renderDevice->setGeomIndexParams( _sphereGeo, _ibSphere, IDXFMT_16 );
	_renderDevice->finishCreatingGeometry( _sphereGeo );

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

	_coneGeo = _renderDevice->beginCreatingGeometry( _vlPosOnly );
	_vbCone = _renderDevice->createVertexBuffer( 13 * 3 * sizeof( float ), coneVerts );
	_ibCone = _renderDevice->createIndexBuffer( 22 * 3 * sizeof( uint16 ), coneInds );
	_renderDevice->setGeomVertexParams( _coneGeo, _vbCone, 0, 0, 12 );
	_renderDevice->setGeomIndexParams( _coneGeo, _ibCone, IDXFMT_16 );
	_renderDevice->finishCreatingGeometry( _coneGeo );

	// Fullscreen polygon
	float fsVerts[3 * 3] = {  // x, y, z
		0.f, 0.f, 1.f,   2.f, 0.f, 1.f,   0.f, 2.f, 1.f
	};

	_FSPolyGeo = _renderDevice->beginCreatingGeometry( _vlPosOnly );
	_vbFSPoly = _renderDevice->createVertexBuffer( 3 * 3 * sizeof( float ), fsVerts );
	_renderDevice->setGeomVertexParams( _FSPolyGeo, _vbFSPoly, 0, 0, 12 );
	_renderDevice->finishCreatingGeometry( _FSPolyGeo );
}


void Renderer::drawAABB( const Vec3f &bbMin, const Vec3f &bbMax )
{
	ASSERT( _curShader != 0x0 );
	
	Matrix4f mat = Matrix4f::TransMat( bbMin.x, bbMin.y, bbMin.z ) *
		Matrix4f::ScaleMat( bbMax.x - bbMin.x, bbMax.y - bbMin.y, bbMax.z - bbMin.z );
	_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.worldMat ], CONST_FLOAT44, &mat.x[ 0 ] );
	
	_renderDevice->setGeometry( _cubeGeo );

	_renderDevice->drawIndexed( PRIM_TRILIST, 0, 36, 0, 8 );
}


void Renderer::drawSphere( const Vec3f &pos, float radius )
{
	ASSERT( _curShader != 0x0 );

	Matrix4f mat = Matrix4f::TransMat( pos.x, pos.y, pos.z ) *
	               Matrix4f::ScaleMat( radius, radius, radius );
	_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.worldMat ], CONST_FLOAT44, &mat.x[ 0 ] );

	_renderDevice->setGeometry( _sphereGeo );

	_renderDevice->drawIndexed( PRIM_TRILIST, 0, 128 * 3, 0, 126 );
}


void Renderer::drawCone( float height, float radius, const Matrix4f &transMat )
{
	ASSERT( _curShader != 0x0 );

	Matrix4f mat = transMat * Matrix4f::ScaleMat( radius, radius, height );
	_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.worldMat ], CONST_FLOAT44, &mat.x[ 0 ] );

	_renderDevice->setGeometry( _coneGeo );

	_renderDevice->drawIndexed( PRIM_TRILIST, 0, 22 * 3, 0, 13 );
}


void Renderer::prepareRenderViews()
{
	SceneManager &scm = Modules::sceneMan();

	Timer *timer = Modules::stats().getTimer( EngineStats::CullingTime );
	if ( Modules::config().gatherTimeStats ) timer->setEnabled( true );

	// Clear old views 
	scm.clearRenderViews();

	// WARNING! Currently lighting will not be present in the first frame, because scene update will happen
	// after lights addition to render views. If that behavior is not desirable uncomment the following statement (may reduce performance a bit)
//	scm.updateNodes();

	//
	// Step 1. Add views for camera and lights based on their frustums
	//
	scm.addRenderView( RenderViewType::Camera, _curCamera, _curCamera->getFrustum() );

	SceneNode *node = nullptr;
	for ( size_t i = 0; i < scm._nodes.size(); ++i )
	{
		node = scm._nodes[ i ];
		if ( !node || node->_type != SceneNodeTypes::Light ) continue;

		// Ignore lights that do not cross the camera frustum and are disabled
		LightNode *light = ( LightNode * ) node;
		if ( _curCamera->getFrustum().cullFrustum( light->getFrustum() ) || light->_flags & SceneNodeFlags::NoDraw ) continue;

		// Light is in current camera view, so add it as a render view 
		// Light's view should be culled with the camera frustum, so link the camera view
		// Also, for shadows we have to cull additional objects that do not cast shadows
		light->_renderViewID = scm.addRenderView( RenderViewType::Light, light, light->getFrustum(), 
												  defaultCameraView, SceneNodeFlags::NoCastShadow );
	}

	// Generate render queue for camera and lights
	scm.updateQueues( SceneNodeFlags::NoDraw );

	//
	// Step 2. Create temporary crop shadow frustums that are used for creating tighter shadow frustums to increase shadow quality
	//
	auto &views = scm.getRenderViews(); auto count = scm.getActiveRenderViewCount();
	int shadowViewStartID = count;
	int processedLightsCount = 0;
	for ( size_t i = 0; i < count; ++i )
	{
		RenderView *view = &views[ i ];
		if ( view->type != RenderViewType::Light ) continue;

		// Skip lights that do not produce shadows
		LightNode *light = ( LightNode * ) view->node;
		if ( light->_shadowMapCount == 0 ) continue;

		// Calculate temporary crop shadow frustums
		// We need to send AABB with only shadow casting objects
		light->_shadowRenderParamsID = prepareCropFrustum( light, view->auxObjectsAABB );
		processedLightsCount++;
	}

	// Prepare render queues for crop frustums
	scm.updateQueues( SceneNodeFlags::NoDraw | SceneNodeFlags::NoCastShadow );

	//
	// Step 3. Calculate final shadow frustums
	//

	// Generate shadow frustums and their render queues
	int start = shadowViewStartID;
	for ( int i = 0; i < processedLightsCount; ++i )
	{
		RenderView *view = &views[ start + i ];
		if ( view->type != RenderViewType::Shadow ) continue;

		LightNode *light = ( LightNode * ) view->node;

		prepareShadowMapFrustum( light, start + i );
		start += light->_shadowMapCount;
	}

	// Shadow frustums are ready, prepare render queues for them
	scm.updateQueues( SceneNodeFlags::NoDraw | SceneNodeFlags::NoCastShadow );

	timer->setEnabled( false );
}

// =================================================================================================
// Material System
// =================================================================================================

bool Renderer::createShaderComb( ShaderCombination &sc, const char *vertexShader, const char *fragmentShader, const char *geometryShader,
								 const char *tessControlShader, const char *tessEvaluationShader, const char *computeShader )
{
	// Create shader program
	uint32 shdObj = _renderDevice->createShader( vertexShader, fragmentShader, geometryShader, tessControlShader, tessEvaluationShader, computeShader );
	if( shdObj == 0 ) return false;
	
	sc.shaderObj = shdObj;
	_renderDevice->bindShader( shdObj );
	
	// Set standard uniforms
	int loc =_renderDevice-> getShaderSamplerLoc( shdObj, "shadowMap" );
	if( loc >= 0 ) _renderDevice->setShaderSampler( loc, 12 );

	sc.uniLocs.reserve( _engineUniforms.size() );

	for ( size_t i = 0; i < _engineUniforms.size(); ++i ) 
	{
		sc.uniLocs.emplace_back( _renderDevice->getShaderSamplerLoc( shdObj, _engineUniforms[ i ].uniformName.c_str() ) );
	}

// 	Misc general uniforms
// 	sc.uni_frameBufSize = _renderDevice->getShaderConstLoc( shdObj, "frameBufSize" );
// 	
// 	// View/projection uniforms
// 	sc.uni_viewMat = _renderDevice->getShaderConstLoc( shdObj, "viewMat" );
// 	sc.uni_viewMatInv = _renderDevice->getShaderConstLoc( shdObj, "viewMatInv" );
// 	sc.uni_projMat = _renderDevice->getShaderConstLoc( shdObj, "projMat" );
// 	sc.uni_viewProjMat = _renderDevice->getShaderConstLoc( shdObj, "viewProjMat" );
// 	sc.uni_viewProjMatInv = _renderDevice->getShaderConstLoc( shdObj, "viewProjMatInv" );
// 	sc.uni_viewerPos = _renderDevice->getShaderConstLoc( shdObj, "viewerPos" );
// 	
// 	// Per-instance uniforms
// 	sc.uni_worldMat = _renderDevice->getShaderConstLoc( shdObj, "worldMat" );
// 	sc.uni_worldNormalMat = _renderDevice->getShaderConstLoc( shdObj, "worldNormalMat" );
// 	sc.uni_nodeId = _renderDevice->getShaderConstLoc( shdObj, "nodeId" );
// 	sc.uni_customInstData = _renderDevice->getShaderConstLoc( shdObj, "customInstData[0]" );
// 	sc.uni_skinMatRows = _renderDevice->getShaderConstLoc( shdObj, "skinMatRows[0]" );
// 	
// 	// Lighting uniforms
// 	sc.uni_lightPos = _renderDevice->getShaderConstLoc( shdObj, "lightPos" );
// 	sc.uni_lightDir = _renderDevice->getShaderConstLoc( shdObj, "lightDir" );
// 	sc.uni_lightColor = _renderDevice->getShaderConstLoc( shdObj, "lightColor" );
// 	sc.uni_shadowSplitDists = _renderDevice->getShaderConstLoc( shdObj, "shadowSplitDists" );
// 	sc.uni_shadowMats = _renderDevice->getShaderConstLoc( shdObj, "shadowMats" );
// 	sc.uni_shadowMapSize = _renderDevice->getShaderConstLoc( shdObj, "shadowMapSize" );
// 	sc.uni_shadowBias = _renderDevice->getShaderConstLoc( shdObj, "shadowBias" );
// 	
// 	// Particle-specific uniforms
// 	sc.uni_parPosArray = _renderDevice->getShaderConstLoc( shdObj, "parPosArray" );
// 	sc.uni_parSizeAndRotArray = _renderDevice->getShaderConstLoc( shdObj, "parSizeAndRotArray" );
// 	sc.uni_parColorArray = _renderDevice->getShaderConstLoc( shdObj, "parColorArray" );
// 	
// 	// Uniforms, requested by extensions

	return true;
}


void Renderer::releaseShaderComb( ShaderCombination &sc )
{
	_renderDevice->destroyShader( sc.shaderObj );
}


void Renderer::setShaderComb( ShaderCombination *sc )
{
	if( _curShader != sc )
	{
		if( sc == 0x0 ) _renderDevice->bindShader( 0 );
		else _renderDevice->bindShader( sc->shaderObj );

		_curShader = sc;
	}
}


void Renderer::commitGeneralUniforms()
{
	ASSERT( _curShader != 0x0 );

	// Note: Make sure that all functions which modify one of the following params increase the stamp
	if( _curShader->lastUpdateStamp != _curShaderUpdateStamp )
	{
		if( _curShader->uniLocs[ _uni.frameBufSize ] >= 0 )
		{
			float dimensions[2] = { (float)_renderDevice->_fbWidth, (float)_renderDevice->_fbHeight };
			_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.frameBufSize ], CONST_FLOAT2, dimensions );
		}
		
		// Viewer params
		if( _curShader->uniLocs[ _uni.viewMat ] >= 0 )
			_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.viewMat ], CONST_FLOAT44, _viewMat.x );
		
		if( _curShader->uniLocs[ _uni.viewMatInv ] >= 0 )
			_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.viewMatInv ], CONST_FLOAT44, _viewMatInv.x );
		
		if( _curShader->uniLocs[ _uni.projMat ] >= 0 )
			_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.projMat ], CONST_FLOAT44, _projMat.x );
		
		if( _curShader->uniLocs[ _uni.viewProjMat ] >= 0 )
			_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.viewProjMat ], CONST_FLOAT44, _viewProjMat.x );

		if( _curShader->uniLocs[ _uni.viewProjMatInv ] >= 0 )
			_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.viewProjMatInv ], CONST_FLOAT44, _viewProjMatInv.x );
		
		if( _curShader->uniLocs[ _uni.viewerPos ] >= 0 )
			_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.viewerPos ], CONST_FLOAT3, &_viewMatInv.x[12] );
		
		// Light params
		if( _curLight != 0x0 )
		{
			if( _curShader->uniLocs[ _uni.lightPos ] >= 0 )
			{
				float data[4] = { _curLight->_absPos.x, _curLight->_absPos.y,
				                  _curLight->_absPos.z, _curLight->_radius };
				_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.lightPos ], CONST_FLOAT4, data );
			}
			
			if( _curShader->uniLocs[ _uni.lightDir ] >= 0 )
			{
				float data[4] = { _curLight->_spotDir.x, _curLight->_spotDir.y,
				                  _curLight->_spotDir.z, cosf( degToRad( _curLight->_fov / 2.0f ) ) };
				_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.lightDir ], CONST_FLOAT4, data );
			}
			
			if( _curShader->uniLocs[ _uni.lightColor ] >= 0 )
			{
				Vec3f col = _curLight->_diffuseCol * _curLight->_diffuseColMult;
				_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.lightColor ], CONST_FLOAT3, &col.x );
			}
			
			if( _curShader->uniLocs[ _uni.shadowSplitDists ] >= 0 )
				_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.shadowSplitDists ], CONST_FLOAT4, &_splitPlanes[1] );

			if( _curShader->uniLocs[ _uni.shadowMats ] >= 0 )
				_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.shadowMats ], CONST_FLOAT44, &_lightMats[0].x[0], 4 );
			
			if( _curShader->uniLocs[ _uni.shadowMapSize ] >= 0 )
				_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.shadowMapSize ], CONST_FLOAT, &_smSize );
			
			if( _curShader->uniLocs[ _uni.shadowBias ] >= 0 )
				_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.shadowBias ], CONST_FLOAT, &_curLight->_shadowMapBias );
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
		if( _curShader == 0x0 || _renderDevice->_curShaderId == 0 ) return false;

		// Setup standard shader uniforms
		commitGeneralUniforms();

		// Configure depth mask
		_renderDevice->setDepthMask( context->writeDepth );

		// Configure cull mode
		if( !Modules::config().wireframeMode )
		{
			_renderDevice->setCullMode( (RDICullMode)context->cullMode );
		}
		
		// Configure blending
		_renderDevice->setBlendMode( context->blendingEnabled, ( RDIBlendFunc ) context->blendStateSrc, ( RDIBlendFunc ) context->blendStateDst );

		// Configure depth test
		_renderDevice->setDepthTest( context->depthTest );
		_renderDevice->setDepthFunc( (RDIDepthFunc)context->depthFunc );

		// Configure alpha-to-coverage
		_renderDevice->setAlphaToCoverage( context->alphaToCoverage && Modules::config().sampleCount > 0 );

		// Configure how many vertices form a patch in tesselation shader
		if ( context->tessVerticesInPatchCount > 1 ) _renderDevice->setTessPatchVertices( context->tessVerticesInPatchCount );
	}

	// Setup texture samplers
	for( size_t i = 0, si = shaderRes->_samplers.size(); i < si; ++i )
	{
		if( _curShader->samplersLocs[i] < 0 ) continue;
		
		ShaderSampler &sampler = shaderRes->_samplers[i];
		TextureResource *texRes = 0x0;

		// Use default texture
		if( firstRec) texRes = sampler.defTex;
		
		// Find sampler in material
		for( size_t j = 0, sj = materialRes->_samplers.size(); j < sj; ++j )
		{
			if( materialRes->_samplers[j].name == sampler.id )
			{
				if( materialRes->_samplers[j].texRes && materialRes->_samplers[j].texRes->isLoaded() )
					texRes = materialRes->_samplers[j].texRes;
				break;
			}
		}

		uint32 sampState = shaderRes->_samplers[i].sampState;
		if( (sampState & SS_FILTER_TRILINEAR) && !Modules::config().trilinearFiltering )
			sampState = (sampState & ~SS_FILTER_TRILINEAR) | SS_FILTER_BILINEAR;
		if( (sampState & SS_ANISO_MASK) > _maxAnisoMask )
			sampState = (sampState & ~SS_ANISO_MASK) | _maxAnisoMask;

		// specify how texture is used (as texture or as read/write buffer)
		uint32 usage = shaderRes->_samplers[ i ].usage;

		// Bind texture
		if( texRes != 0x0 )
		{
			if( texRes->getTexType() != sampler.type ) break;  // Wrong type
			
			if( texRes->getTexType() == TextureTypes::Tex2D )
			{
				if( texRes->getRBObject() == 0 )
				{
					_renderDevice->setTexture( shaderRes->_samplers[i].texUnit, texRes->getTexObject(), sampState, usage);
				}
				else if( texRes->getRBObject() != _renderDevice->_curRendBuf )
				{
					_renderDevice->setTexture( shaderRes->_samplers[i].texUnit,
					                  _renderDevice->getRenderBufferTex( texRes->getRBObject(), 0 ), sampState, 0 );
				}
				else  // Trying to bind active render buffer as texture
				{
					_renderDevice->setTexture( shaderRes->_samplers[i].texUnit, TextureResource::defTex2DObject, 0, 0 );
				}
			}
			else
			{
				_renderDevice->setTexture( shaderRes->_samplers[i].texUnit, texRes->getTexObject(), sampState, usage );
			}
		}

		// Find sampler in pipeline
		if( firstRec )
		{
			for( size_t j = 0, sj = _pipeSamplerBindings.size(); j < sj; ++j )
			{
				if( strcmp( _pipeSamplerBindings[j].sampler, sampler.id.c_str() ) == 0 )
				{
					_renderDevice->setTexture( shaderRes->_samplers[i].texUnit, _renderDevice->getRenderBufferTex(
						_pipeSamplerBindings[j].rbObj, _pipeSamplerBindings[j].bufIndex ), sampState, usage );

					break;
				}
			}
		}
	}

	size_t uniOffset = _engineUniforms.size();

	// Set custom uniforms
	for( size_t i = 0, si = shaderRes->_uniforms.size(); i < si; ++i )
	{
		if( _curShader->uniLocs[ i + uniOffset ] < 0 ) continue;
		
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
				_renderDevice->setShaderConst( _curShader->uniLocs[ i + uniOffset ], CONST_FLOAT, unifData );
				break;
			case 4:
				_renderDevice->setShaderConst( _curShader->uniLocs[ i + uniOffset ], CONST_FLOAT4, unifData );
				break;
			}
		}
	}

	// Set custom buffers
	for ( size_t i = 0; i < shaderRes->_buffers.size(); ++i )
	{
		if ( _curShader->bufferLocs[ i ] < 0 ) continue;
		
		ComputeBufferResource *buf = 0;

		// Find buffer in material
		for ( size_t j = 0; j < materialRes->_buffers.size(); ++j )
		{
			MatBuffer &matBuffer = materialRes->_buffers[ j ];

			if ( matBuffer.name == shaderRes->_buffers[ i ].id )
			{
				buf = matBuffer.compBufRes;
				break;
			}
		}

		if ( buf )
		{
			_renderDevice->setStorageBuffer( _curShader->bufferLocs[ i ], buf->_bufferID );
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
		_renderDevice->setBlendMode( false );
		_renderDevice->setAlphaToCoverage( false );
		_renderDevice->setDepthTest( true );
		_renderDevice->setDepthFunc( DSS_DEPTHFUNC_LESS_EQUAL );
		_renderDevice->setDepthMask( true );
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
	_shadowRB = _renderDevice->createRenderBuffer( width, height, TextureFormats::BGRA8, true, 0, 0, 0 );
	
	return _shadowRB != 0;
}


void Renderer::releaseShadowRB()
{
	if( _shadowRB ) _renderDevice->destroyRenderBuffer( _shadowRB );
}


void Renderer::setupShadowMap( bool noShadows )
{
	uint32 sampState = SS_FILTER_BILINEAR | SS_ANISO1 | SS_ADDR_CLAMPCOL | SS_COMP_LEQUAL;
	
	// Bind shadow map
	if( !noShadows && _curLight->_shadowMapCount > 0 )
	{
		_renderDevice->setTexture( 12, _renderDevice->getRenderBufferTex( _shadowRB, 32 ), sampState, TextureUsage::Texture );
		_smSize = (float)Modules::config().shadowMapSize;
	}
	else
	{
		_renderDevice->setTexture( 12, _defShadowMap, sampState, TextureUsage::Texture );
		_smSize = 4;
	}
}


Matrix4f Renderer::calcCropMatrix( int renderView, const LightNode *light, const Matrix4f &lightViewProjMat )
{
	float frustMinX = Math::MaxFloat, bbMinX = Math::MaxFloat;
	float frustMinY = Math::MaxFloat, bbMinY = Math::MaxFloat;
	float frustMinZ = Math::MaxFloat, bbMinZ = Math::MaxFloat;
	float frustMaxX = -Math::MaxFloat, bbMaxX = -Math::MaxFloat;
	float frustMaxY = -Math::MaxFloat, bbMaxY = -Math::MaxFloat;
	float frustMaxZ = -Math::MaxFloat, bbMaxZ = -Math::MaxFloat;

	auto &views = Modules::sceneMan().getRenderViews();
	RenderQueue &renderQueue = views[ renderView ].objects;
	Frustum &frustSlice = views[ renderView ].frustum;
	Vec3f lightPos = light->_absPos;

	for ( size_t i = 0, s = renderQueue.size(); i < s; ++i )
	{
		const BoundingBox &aabb = renderQueue[ i ].node->getBBox();

		// Check if light is inside AABB
		if ( lightPos.x >= aabb.min.x && lightPos.y >= aabb.min.y && lightPos.z >= aabb.min.z &&
			lightPos.x <= aabb.max.x && lightPos.y <= aabb.max.y && lightPos.z <= aabb.max.z )
		{
			bbMinX = bbMinY = bbMinZ = -1;
			bbMaxX = bbMaxY = bbMaxZ = 1;
			break;
		}

		for ( uint32 j = 0; j < 8; ++j )
		{
			Vec4f v1 = lightViewProjMat * Vec4f( aabb.getCorner( j ) );
			v1.w = 1.f / fabsf( v1.w );
			v1.x *= v1.w; v1.y *= v1.w; v1.z *= v1.w;

			if ( v1.x < bbMinX ) bbMinX = v1.x;
			if ( v1.y < bbMinY ) bbMinY = v1.y;
			if ( v1.z < bbMinZ ) bbMinZ = v1.z;
			if ( v1.x > bbMaxX ) bbMaxX = v1.x;
			if ( v1.y > bbMaxY ) bbMaxY = v1.y;
			if ( v1.z > bbMaxZ ) bbMaxZ = v1.z;
		}
	}

	// Find post-projective space AABB of frustum slice if light is not inside
	if ( frustSlice.cullSphere( light->_absPos, 0 ) )
	{
		// Get frustum in post-projective space
		for ( uint32 i = 0; i < 8; ++i )
		{
			// Frustum slice
			Vec4f v1 = lightViewProjMat * Vec4f( frustSlice.getCorner( i ) );
			v1.w = 1.f / fabsf( v1.w );  // Use absolute value to reduce problems with back projection when v1.w < 0
			v1.x *= v1.w; v1.y *= v1.w; v1.z *= v1.w;

			if ( v1.x < frustMinX ) frustMinX = v1.x;
			if ( v1.y < frustMinY ) frustMinY = v1.y;
			if ( v1.z < frustMinZ ) frustMinZ = v1.z;
			if ( v1.x > frustMaxX ) frustMaxX = v1.x;
			if ( v1.y > frustMaxY ) frustMaxY = v1.y;
			if ( v1.z > frustMaxZ ) frustMaxZ = v1.z;
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
	float scaleX = 2.0f / ( maxX - minX );
	float scaleY = 2.0f / ( maxY - minY );
	float scaleZ = 2.0f / ( maxZ - minZ );

	float offsetX = -0.5f * ( maxX + minX ) * scaleX;
	float offsetY = -0.5f * ( maxY + minY ) * scaleY;
	float offsetZ = -0.5f * ( maxZ + minZ ) * scaleZ;

	// Build final matrix
	float cropMat[ 16 ] = { scaleX, 0, 0, 0,
		0, scaleY, 0, 0,
		0, 0, scaleZ, 0,
		offsetX, offsetY, offsetZ, 1 };

	return std::move( Matrix4f( cropMat ) );
}


int Renderer::prepareCropFrustum( const LightNode *light, const BoundingBox &viewBB )
{
	if ( !light )
	{
		Modules::log().writeDebugInfo( "Renderer::prepareShadowMapFrustum. Incorrect light sent to function!" );
		return -1;
	}

	// Find depth range of lit geometry
	float minDist = Math::MaxFloat, maxDist = 0.0f;
	for ( uint32 i = 0; i < 8; ++i )
	{
		float dist = -( _curCamera->getViewMat() * viewBB.getCorner( i ) ).z;
		if ( dist < minDist ) minDist = dist;
		if ( dist > maxDist ) maxDist = dist;
	}

	// Don't adjust near plane; this means less precision if scene is far away from viewer but that
	// shouldn't be too noticeable and brings better performance since the nearer split volumes are empty
	minDist = _curCamera->_frustNear;

	// Calculate split distances using PSSM scheme
	const float nearDist = maxf( minDist, _curCamera->_frustNear );
	const float farDist = maxf( maxDist, minDist + 0.01f );
	const uint32 numMaps = light->_shadowMapCount;
	const float lambda = light->_shadowSplitLambda;

	ShadowParameters params;
	params.splitPlanes[ 0 ] = nearDist;
	params.splitPlanes[ numMaps ] = farDist;

	for ( uint32 i = 1; i < numMaps; ++i )
	{
		float f = ( float ) i / numMaps;
		float logDist = nearDist * powf( farDist / nearDist, f );
		float uniformDist = nearDist + ( farDist - nearDist ) * f;

		params.splitPlanes[ i ] = ( 1 - lambda ) * uniformDist + lambda * logDist;  // Lerp
	}

	// Split viewing frustum into slices and render shadow maps
	Frustum frustum;
	for ( uint32 i = 0; i < numMaps; ++i )
	{
		// Create frustum slice
		if ( !_curCamera->_orthographic )
		{
			float newLeft = _curCamera->_frustLeft * params.splitPlanes[ i ] / _curCamera->_frustNear;
			float newRight = _curCamera->_frustRight * params.splitPlanes[ i ] / _curCamera->_frustNear;
			float newBottom = _curCamera->_frustBottom * params.splitPlanes[ i ] / _curCamera->_frustNear;
			float newTop = _curCamera->_frustTop * params.splitPlanes[ i ] / _curCamera->_frustNear;
			frustum.buildViewFrustum( _curCamera->_absTrans, newLeft, newRight, newBottom, newTop,
				params.splitPlanes[ i ], params.splitPlanes[ i + 1 ] );
		}
		else
		{
			frustum.buildBoxFrustum( _curCamera->_absTrans, _curCamera->_frustLeft, _curCamera->_frustRight,
				_curCamera->_frustBottom, _curCamera->_frustTop,
				-params.splitPlanes[ i ], -params.splitPlanes[ i + 1 ] );
		}

		// Get light projection matrix
		float ymax = _curCamera->_frustNear * tanf( degToRad( light->_fov / 2 ) );
		float xmax = ymax * 1.0f;  // ymax * aspect
		params.lightProjMatrix[ i ] = Matrix4f::PerspectiveMat(
			-xmax, xmax, -ymax, ymax, _curCamera->_frustNear, light->_radius );

		// Create and store view and other shadow parameters
		Modules::sceneMan().addRenderView( RenderViewType::Shadow, ( SceneNode * ) light, frustum, /*linkedLightView*/ -1 );
	}

	// Store complete shadow parameters for one light
	_shadowParams.emplace_back( params );

	return ( int ) _shadowParams.size() - 1;
}


bool Renderer::prepareShadowMapFrustum( const LightNode *light, int shadowView )
{
	if ( !light )
	{
		Modules::log().writeDebugInfo( "Renderer::prepareShadowMapFrustum. Incorrect light sent to function!" );
		return false;
	}

	ShadowParameters &params = _shadowParams[ light->_shadowRenderParamsID ];

	// Split viewing frustum into slices and render shadow maps
	Frustum frustum;
	for ( uint32 i = 0; i < light->_shadowMapCount; ++i )
	{
		// Build optimized light projection matrix
		Matrix4f lightProjMat = params.lightProjMatrix[ i ];
		Matrix4f lightViewProjMat = lightProjMat * light->getViewMat();

		// We have to send corresponding shadow view id in order to calculate crop matrix, 
		// and only id of the first one is sent to this function, therefore shadow map iterator is needed   
		lightProjMat = calcCropMatrix( shadowView + i, light, lightViewProjMat ) * lightProjMat;

		// Generate final frustum with shadow casters for current slice
		frustum.buildViewFrustum( light->getViewMat(), lightProjMat );
		
		params.lightMats[ i ] = lightProjMat * light->getViewMat();
		params.lightProjMatrix[ i ] = lightProjMat;

		// Create and store view and other shadow parameters
		int view = Modules::sceneMan().addRenderView( RenderViewType::Shadow, (SceneNode *) light, frustum, /*linkedLightView*/ -1 );
		params.viewID[ i ] = view;
	}

	return true;
}


void Renderer::updateShadowMap()
{
	if ( _curLight == 0x0 || _curLight->_shadowRenderParamsID == -1 ) return;

	uint32 prevRendBuf = _renderDevice->_curRendBuf;
	int prevVPX = _renderDevice->_vpX, prevVPY = _renderDevice->_vpY, prevVPWidth = _renderDevice->_vpWidth, prevVPHeight = _renderDevice->_vpHeight;

	int shadowRTWidth, shadowRTHeight;
	_renderDevice->getRenderBufferDimensions( _shadowRB, &shadowRTWidth, &shadowRTHeight );

	_renderDevice->setViewport( 0, 0, shadowRTWidth, shadowRTHeight );
	_renderDevice->setRenderBuffer( _shadowRB );

	_renderDevice->setColorWriteMask( false );
	_renderDevice->setDepthMask( true );
	_renderDevice->clear( CLR_DEPTH, 0x0, 1.f );

	// ********************************************************************************************
	// Cascaded Shadow Maps
	// ********************************************************************************************

	// Prepare shadow map rendering
	_renderDevice->setDepthTest( true );
	//_renderDevice->setCullMode( RS_CULL_FRONT );	// Front face culling reduces artefacts but produces more "peter-panning"
	
	const uint32 numMaps = _curLight->_shadowMapCount;
	ShadowParameters &params = _shadowParams[ _curLight->_shadowRenderParamsID ];

	// Copy split planes so that it is passed to shader on material setting
	for ( uint32 i = 0; i < 5; ++i ) _splitPlanes[ i ] = params.splitPlanes[ i ];

	// Split viewing frustum into slices and render shadow maps
	for ( uint32 i = 0; i < numMaps; ++i )
	{
		// Create texture atlas if several splits are enabled
		if ( numMaps > 1 )
		{
			const int hsm = Modules::config().shadowMapSize / 2;
			const int scissorXY[ 8 ] = { 0, 0,  hsm, 0,  hsm, hsm,  0, hsm };
			const float transXY[ 8 ] = { -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, 0.5f,  -0.5f, 0.5f };

			_renderDevice->setScissorTest( true );

			// Select quadrant of shadow map
			params.lightProjMatrix[ i ].scale( 0.5f, 0.5f, 1.0f );
			params.lightProjMatrix[ i ].translate( transXY[ i * 2 ], transXY[ i * 2 + 1 ], 0.0f );
			_renderDevice->setScissorRect( scissorXY[ i * 2 ], scissorXY[ i * 2 + 1 ], hsm, hsm );
		}

		_lightMats[ i ] = params.lightProjMatrix[ i ] * _curLight->getViewMat();
		setupViewMatrices( _curLight->getViewMat(), params.lightProjMatrix[ i ] );

		// Render
		Modules::sceneMan().setCurrentView( params.viewID[ i ] );
		Frustum &f = Modules::sceneMan().getRenderViews()[ params.viewID[ i ] ].frustum;
		drawRenderables( _curLight->_shadowContext, 0, false, &f, 0x0, RenderingOrder::None, -1 );
	}

	// Map from post-projective space [-1,1] to texture space [0,1]
	for ( uint32 i = 0; i < numMaps; ++i )
	{
		_lightMats[ i ].scale( 0.5f, 0.5f, 1.0f );
		_lightMats[ i ].translate( 0.5f, 0.5f, 0.0f );
	}

	// ********************************************************************************************

	_renderDevice->setCullMode( RS_CULL_BACK );
	_renderDevice->setScissorTest( false );

	_renderDevice->setViewport( prevVPX, prevVPY, prevVPWidth, prevVPHeight );
	_renderDevice->setRenderBuffer( prevRendBuf );
	_renderDevice->setColorWriteMask( true );
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
	_renderDevice->getColorWriteMask( prevColorMask );
	_renderDevice->getDepthMask( prevDepthMask );
	
	setMaterial( 0x0, "" );
	_renderDevice->setColorWriteMask( false );
	_renderDevice->setDepthMask( false );
	
	setShaderComb( &Modules::renderer()._defColorShader );
	commitGeneralUniforms();

	_renderDevice->setGeometry( _cubeGeo );

	// Draw occlusion proxies
	for( size_t i = 0, s = _occProxies[list].size(); i < s; ++i )
	{
		OccProxy &proxy = _occProxies[list][i];

		_renderDevice->beginQuery( proxy.queryObj );
		
		Matrix4f mat = Matrix4f::TransMat( proxy.bbMin.x, proxy.bbMin.y, proxy.bbMin.z ) *
			Matrix4f::ScaleMat( proxy.bbMax.x - proxy.bbMin.x, proxy.bbMax.y - proxy.bbMin.y, proxy.bbMax.z - proxy.bbMin.z );
		_renderDevice->setShaderConst( _curShader->uniLocs[ _uni.worldMat ], CONST_FLOAT44, &mat.x[0] );

		// Draw AABB
		_renderDevice->drawIndexed( PRIM_TRILIST, 0, 36, 0, 8 );

		_renderDevice->endQuery( proxy.queryObj );
	}

	setShaderComb( 0x0 );
	_renderDevice->setColorWriteMask( prevColorMask );
	_renderDevice->setDepthMask( prevDepthMask );

	_occProxies[list].resize( 0 );
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
		PipeSamplerBinding binding;
		size_t len = std::min( sampler.length(), ( size_t ) 63 );
		strncpy_s( binding.sampler, 63, sampler.c_str(), len );
		binding.sampler[ len ] = '\0';
		binding.rbObj = rbObj;
		binding.bufIndex = bufIndex;

		_pipeSamplerBindings.push_back( binding );
	}
}


void Renderer::clear( bool depth, bool buf0, bool buf1, bool buf2, bool buf3,
                      float r, float g, float b, float a )
{
	float clrColor[] = { r, g, b, a };

	_renderDevice->setBlendMode( false );  // Clearing floating point buffers causes problems when blending is enabled on Radeon 9600
	_renderDevice->setDepthMask( true );

	uint32 clearFlags = 0;
	if( depth ) clearFlags |= CLR_DEPTH;
	if( buf0 ) clearFlags |= CLR_COLOR_RT0;
	if( buf1 ) clearFlags |= CLR_COLOR_RT1;
	if( buf2 ) clearFlags |= CLR_COLOR_RT2;
	if( buf3 ) clearFlags |= CLR_COLOR_RT3;
	
	if( _renderDevice->_curRendBuf == 0x0 )
	{
		_renderDevice->setScissorRect( _curCamera->_vpX, _curCamera->_vpY, _curCamera->_vpWidth, _curCamera->_vpHeight );
		_renderDevice->setScissorTest( true );
	}
	
	_renderDevice->clear( clearFlags, clrColor, 1.f );
	_renderDevice->setScissorTest( false );
}


void Renderer::drawFSQuad( Resource *matRes, const string &shaderContext )
{
	if( matRes == 0x0 || matRes->getType() != ResourceTypes::Material ) return;

	setupViewMatrices( _curCamera->getViewMat(), Matrix4f::OrthoMat( 0, 1, 0, 1, -1, 1 ) );
	
	if( !setMaterial( (MaterialResource *)matRes, shaderContext ) ) return;

	_renderDevice->setGeometry( _FSPolyGeo );
	_renderDevice->draw( PRIM_TRILIST, 0, 3 );
}


void Renderer::drawGeometry( const string &shaderContext, int theClass,
                             RenderingOrder::List order, int occSet )
{
	Modules::sceneMan().setCurrentView( defaultCameraView );
	Modules::sceneMan().sortViewObjects( order );
	
	setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
	drawRenderables( shaderContext, theClass, false, &_curCamera->getFrustum(), 0x0, order, occSet );
}


void Renderer::drawLightGeometry( const string &shaderContext, int theClass,
                                  bool noShadows, RenderingOrder::List order, int occSet )
{
// 	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), 0x0, RenderingOrder::None,
// 	                                  SceneNodeFlags::NoDraw, true, false );
	
	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::FwdLightsGPUTime );
	if( Modules::config().gatherTimeStats ) timer->beginQuery( _frameID );
	
	for( size_t i = 0, s = Modules::sceneMan().getLightQueue().size(); i < s; ++i )
	{
		_curLight = (LightNode *)Modules::sceneMan().getLightQueue()[i];

		// Check if light is occluded
		if( occSet >= 0 )
		{
			if( occSet > (int)_curLight->_occQueries.size() - 1 )
			{
				_curLight->_occQueries.resize( occSet + 1, 0 );
				_curLight->_occQueriesLastVisited.resize( occSet + 1, 0 );
			}
			if( _curLight->_occQueries[occSet] == 0 )
			{
				_curLight->_occQueries[occSet] = _renderDevice->createOcclusionQuery();
				_curLight->_occQueriesLastVisited[occSet] = 0;
			}
			else
			{
				if( _curLight->_occQueriesLastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					_curLight->_occQueriesLastVisited[occSet] = Modules::renderer().getFrameID();
				
					Vec3f bbMin, bbMax;
					_curLight->getFrustum().calcAABB( bbMin, bbMax );
					
					// Check that viewer is outside light bounds
					if( nearestDistToAABB( _curCamera->getFrustum().getOrigin(), bbMin, bbMax ) > 0 )
					{
						Modules::renderer().pushOccProxy( 1, bbMin, bbMax, _curLight->_occQueries[occSet] );

						// Check query result from previous frame
						if( _renderDevice->getQueryResult( _curLight->_occQueries[occSet] ) < 1 )
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
			_renderDevice->setScissorRect( ftoi_r( bbx * _renderDevice->_fbWidth ), ftoi_r( bby * _renderDevice->_fbHeight ),
			                      ftoi_r( bbw * _renderDevice->_fbWidth ), ftoi_r( bbh * _renderDevice->_fbHeight ) );
			_renderDevice->setScissorTest( true );
		}
		
		// Render
		Modules::sceneMan().setCurrentView( _curLight->_renderViewID );
		Modules::sceneMan().sortViewObjects( order );
// 		Modules::sceneMan().updateQueues( _curCamera->getFrustum(), &_curLight->getFrustum(),
// 		                                  order, SceneNodeFlags::NoDraw, false, true );
		setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
		drawRenderables( shaderContext.empty() ? _curLight->_lightingContext : shaderContext,
		                 theClass, false, &_curCamera->getFrustum(),
		                 &_curLight->getFrustum(), order, occSet );
		Modules().stats().incStat( EngineStats::LightPassCount, 1 );

		// Reset
		_renderDevice->setScissorTest( false );
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
	
// 	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), 0x0, RenderingOrder::None,
// 	                                  SceneNodeFlags::NoDraw, true, false );
	
	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::DefLightsGPUTime );
	if( Modules::config().gatherTimeStats ) timer->beginQuery( _frameID );
	
	auto &views = Modules::sceneMan().getRenderViews();
	for ( size_t i = 0, s = views.size(); i < s; ++i )
	{
		if ( views[ i ].type != RenderViewType::Light ) continue;

		_curLight = ( LightNode * ) views[ i ].node;

		// Check if light is occluded
		if( occSet >= 0 )
		{
			if( occSet > (int)_curLight->_occQueries.size() - 1 )
			{
				_curLight->_occQueries.resize( occSet + 1, 0 );
				_curLight->_occQueriesLastVisited.resize( occSet + 1, 0 );
			}
			if( _curLight->_occQueries[occSet] == 0 )
			{
				_curLight->_occQueries[occSet] = _renderDevice->createOcclusionQuery();
				_curLight->_occQueriesLastVisited[occSet] = 0;
			}
			else
			{
				if( _curLight->_occQueriesLastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					_curLight->_occQueriesLastVisited[occSet] = Modules::renderer().getFrameID();
				
					Vec3f bbMin, bbMax;
					_curLight->getFrustum().calcAABB( bbMin, bbMax );
					
					// Check that viewer is outside light bounds
					if( nearestDistToAABB( _curCamera->getFrustum().getOrigin(), bbMin, bbMax ) > 0 )
					{
						Modules::renderer().pushOccProxy( 1, bbMin, bbMax, _curLight->_occQueries[occSet] );

						// Check query result from previous frame
						if( _renderDevice->getQueryResult( _curLight->_occQueries[occSet] ) < 1 )
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

		_renderDevice->setCullMode( RS_CULL_FRONT );
		_renderDevice->setDepthTest( false );

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
		_renderDevice->setCullMode( RS_CULL_BACK );
		_renderDevice->setDepthTest( true );
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

void Renderer::dispatchCompute( MaterialResource *materialRes, const std::string &context, uint32 groups_x, uint32 groups_y, uint32 groups_z )
{
	if ( !setMaterial( materialRes, context ) ) return;

	ShaderCombination *curShader = Modules::renderer().getCurShader();

	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::ComputeGPUTime );
	if ( Modules::config().gatherTimeStats ) timer->beginQuery( Modules::renderer().getFrameID() );

	_renderDevice->runComputeShader( curShader->shaderObj, groups_x, groups_y, groups_z );

	timer->endQuery();
}

// =================================================================================================
// Scene Node Rendering Functions
// =================================================================================================

void Renderer::drawRenderables( const string &shaderContext, int theClass, bool debugView,
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
		_renderDevice->setCullMode( RS_CULL_NONE );
		_renderDevice->setFillMode( RS_FILL_WIREFRAME );
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
		_renderDevice->setFillMode( RS_FILL_SOLID );
	}
}


void Renderer::drawMeshes( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, int theClass,
                           bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order,
                           int occSet )
{
	if( frust1 == 0x0 ) return;
	
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	const RenderQueue &renderQueue = Modules::sceneMan().getRenderQueue();
	GeometryResource *curGeoRes = 0x0;
	MaterialResource *curMatRes = 0x0;

	DefaultShaderUniforms &uni = Modules::renderer()._uni;

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
				meshNode->_occQueriesLastVisited.resize( occSet + 1, 0 );
			}
			if( meshNode->_occQueries[occSet] == 0 )
			{
				queryObj = rdi->createOcclusionQuery();
				meshNode->_occQueries[occSet] = queryObj;
				meshNode->_occQueriesLastVisited[occSet] = 0;
			}
			else
			{
				if( meshNode->_occQueriesLastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					meshNode->_occQueriesLastVisited[occSet] = Modules::renderer().getFrameID();
				
					// Check query result (viewer must be outside of bounding box)
					if( nearestDistToAABB( frust1->getOrigin(), meshNode->getBBox().min,
					                       meshNode->getBBox().max ) > 0 &&
						rdi->getQueryResult( meshNode->_occQueries[occSet] ) < 1 )
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
		
			rdi->setGeometry( curGeoRes->getGeometryInfo() );
		}

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

			// Darken models with skeleton so that bones are more noticeable
			if( !modelNode->_jointList.empty() ) color = color * 0.3f;

			rdi->setShaderConst( Modules::renderer()._defColShader_color, CONST_FLOAT4, &color.x );
		}

		ShaderCombination *curShader = Modules::renderer().getCurShader();
		
		if( modelChanged || curShader != prevShader )
		{
			// Skeleton
			if( curShader->uniLocs[ uni.skinMatRows ] >= 0 && !modelNode->_skinMatRows.empty() )
			{
				// Note:	OpenGL 2.1 supports mat4x3 but it is internally realized as mat4 on most
				//			hardware so it would require 4 instead of 3 uniform slots per joint
				
				rdi->setShaderConst( curShader->uniLocs[ uni.skinMatRows ], CONST_FLOAT4,
				                      &modelNode->_skinMatRows[0], (int)modelNode->_skinMatRows.size() );
			}

			modelChanged = false;
		}

		// World transformation
		if( curShader->uniLocs[ uni.worldMat ] >= 0 )
		{
			rdi->setShaderConst( curShader->uniLocs[ uni.worldMat ], CONST_FLOAT44, &meshNode->_absTrans.x[0] );
		}
		if( curShader->uniLocs[ uni.worldNormalMat ] >= 0 )
		{
			// TODO: Optimize this
			Matrix4f normalMat4 = meshNode->_absTrans.inverted().transposed();
			float normalMat[9] = { normalMat4.x[0], normalMat4.x[1], normalMat4.x[2],
			                       normalMat4.x[4], normalMat4.x[5], normalMat4.x[6],
			                       normalMat4.x[8], normalMat4.x[9], normalMat4.x[10] };
			rdi->setShaderConst( curShader->uniLocs[ uni.worldNormalMat ], CONST_FLOAT33, normalMat );
		}
		if( curShader->uniLocs[ uni.nodeId ] >= 0 )
		{
			float id = (float)meshNode->getHandle();
			rdi->setShaderConst( curShader->uniLocs[ uni.nodeId ], CONST_FLOAT, &id );
		}
		if( curShader->uniLocs[ uni.customInstData ] >= 0 )
		{
			rdi->setShaderConst( curShader->uniLocs[ uni.customInstData ], CONST_FLOAT4,
			                      &modelNode->_customInstData[0].x, ModelCustomVecCount );
		}

		if( queryObj )
			rdi->beginQuery( queryObj );
		
		// Render
		rdi->drawIndexed( meshNode->getPrimType(), meshNode->getBatchStart(), meshNode->getBatchCount(),
		                  meshNode->getVertRStart(), meshNode->getVertREnd() - meshNode->getVertRStart() + 1 );
		Modules::stats().incStat( EngineStats::BatchCount, 1 );
		Modules::stats().incStat( EngineStats::TriCount, meshNode->getBatchCount() / 3.0f );

		if( queryObj )
			rdi->endQuery( queryObj );
	}

	// Draw occlusion proxies
	if( occSet >= 0 )
		Modules::renderer().drawOccProxies( OCCPROXYLIST_RENDERABLES );
}


void Renderer::drawParticles( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, int theClass,
                              bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order,
                              int occSet )
{
	if( frust1 == 0x0 || Modules::renderer().getCurCamera() == 0x0 ) return;
	if( debugView ) return;  // Don't render particles in debug view

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	const RenderQueue &renderQueue = Modules::sceneMan().getRenderQueue();
	MaterialResource *curMatRes = 0x0;

	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::ParticleGPUTime );
	if( Modules::config().gatherTimeStats ) timer->beginQuery( Modules::renderer().getFrameID() );

	// Bind particle geometry
	rdi->setGeometry( Modules::renderer().getParticleGeometry() );
	ASSERT( QuadIndexBufCount >= ParticlesPerBatch * 6 );

	DefaultShaderUniforms &uni = Modules::renderer()._uni;

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
				emitter->_occQueriesLastVisited.resize( occSet + 1, 0 );
			}
			if( emitter->_occQueries[occSet] == 0 )
			{
				queryObj = rdi->createOcclusionQuery();
				emitter->_occQueries[occSet] = queryObj;
				emitter->_occQueriesLastVisited[occSet] = 0;
			}
			else
			{
				if( emitter->_occQueriesLastVisited[occSet] != Modules::renderer().getFrameID() )
				{
					emitter->_occQueriesLastVisited[occSet] = Modules::renderer().getFrameID();
				
					// Check query result (viewer must be outside of bounding box)
					if( nearestDistToAABB( frust1->getOrigin(), emitter->getBBox().min,
					                       emitter->getBBox().max ) > 0 &&
						rdi->getQueryResult( emitter->_occQueries[occSet] ) < 1 )
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

		if( queryObj )
			rdi->beginQuery( queryObj );
		
		// Shader uniforms
		ShaderCombination *curShader = Modules::renderer().getCurShader();
		if( curShader->uniLocs[ uni.nodeId ] >= 0 )
		{
			float id = (float)emitter->getHandle();
			rdi->setShaderConst( curShader->uniLocs[ uni.nodeId ], CONST_FLOAT, &id );
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
			if( curShader->uniLocs[ uni.parPosArray ] >= 0 )
				rdi->setShaderConst( curShader->uniLocs[ uni.parPosArray ], CONST_FLOAT3,
				                      (float *)emitter->_parPositions + j*ParticlesPerBatch*3, ParticlesPerBatch );
			if( curShader->uniLocs[ uni.parSizeAndRotArray ] >= 0 )
				rdi->setShaderConst( curShader->uniLocs[ uni.parSizeAndRotArray ], CONST_FLOAT2,
				                      (float *)emitter->_parSizesANDRotations + j*ParticlesPerBatch*2, ParticlesPerBatch );
			if( curShader->uniLocs[ uni.parColorArray ] >= 0 )
				rdi->setShaderConst( curShader->uniLocs[ uni.parColorArray ], CONST_FLOAT4,
				                      (float *)emitter->_parColors + j*ParticlesPerBatch*4, ParticlesPerBatch );

			rdi->drawIndexed( PRIM_TRILIST, 0, ParticlesPerBatch * 6, 0, ParticlesPerBatch * 4 );
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
				if( curShader->uniLocs[ uni.parPosArray ] >= 0 )
					rdi->setShaderConst( curShader->uniLocs[ uni.parPosArray ], CONST_FLOAT3,
					                      (float *)emitter->_parPositions + offset*3, count );
				if( curShader->uniLocs[ uni.parSizeAndRotArray ] >= 0 )
					rdi->setShaderConst( curShader->uniLocs[ uni.parSizeAndRotArray ], CONST_FLOAT2,
					                      (float *)emitter->_parSizesANDRotations + offset*2, count );
				if( curShader->uniLocs[ uni.parColorArray ] >= 0 )
					rdi->setShaderConst( curShader->uniLocs[ uni.parColorArray ], CONST_FLOAT4,
					                      (float *)emitter->_parColors + offset*4, count );
				
				rdi->drawIndexed( PRIM_TRILIST, 0, count * 6, 0, count * 4 );
				Modules::stats().incStat( EngineStats::BatchCount, 1 );
				Modules::stats().incStat( EngineStats::TriCount, count * 2.0f );
			}
		}

		if( queryObj )
			rdi->endQuery( queryObj );
	}

	timer->endQuery();

	// Draw occlusion proxies
	if( occSet >= 0 )
		Modules::renderer().drawOccProxies( OCCPROXYLIST_RENDERABLES );	
}


void Renderer::drawComputeResults( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, int theClass,
								   bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order,
								   int occSet )
{
	if ( frust1 == 0x0 ) return;

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
	if ( !rdi->getCaps().computeShaders ) return; 

	const RenderQueue &renderQueue = Modules::sceneMan().getRenderQueue();

	MaterialResource *curMatRes = 0;
	ShaderCombination *curShader = 0;

	GPUTimer *timer = Modules::stats().getGPUTimer( EngineStats::ComputeGPUTime );
	if ( Modules::config().gatherTimeStats ) timer->beginQuery( Modules::renderer().getFrameID() );

	DefaultShaderUniforms &uni = Modules::renderer()._uni;

	// Loop over compute node queue
	for ( size_t i = firstItem; i <= lastItem; ++i )
	{
		ComputeNode *compNode = ( ComputeNode * ) renderQueue[ i ].node;

		// Sanity check
		if ( !compNode->_compBufferRes->_useAsVertexBuf || !compNode->_compBufferRes->_geometryParamsSet || 
			 compNode->_elementsCount == 0 || !compNode->_materialRes->isOfClass( theClass ) )
			continue;

		if ( debugView )
		{
			// render AABB
			Modules::renderer().setShaderComb( &Modules::renderer()._defColorShader );
			Modules::renderer().commitGeneralUniforms();

			Vec4f color = Vec4f( 1.f, 1.f, 1.f, 1 );
			rdi->setShaderConst( Modules::renderer()._defColShader_color, CONST_FLOAT4, &color.x );

			Modules::renderer().drawAABB( compNode->_bBox.min, compNode->_bBox.max );

			continue;
		}

		// Specify drawing type
		RDIPrimType drawType;
		switch ( compNode->_drawType )
		{
			case 0: // Triangles
				drawType = PRIM_TRILIST;
				break;
			case 1: // Lines
				drawType = PRIM_LINELIST;
				break;
			case 2: // Patches
				drawType = PRIM_PATCHES;
				break;
			case 3: // Points
				drawType = PRIM_POINTS;
				break;
			default:
				drawType = PRIM_POINTS;
				break;
		}

		// Set material
		if ( curMatRes != compNode->_materialRes )
		{
			if ( !Modules::renderer().setMaterial( compNode->_materialRes, shaderContext ) ) continue;
			curMatRes = compNode->_materialRes;
		}

		// Set compute buffer to act like vertex buffer
		rdi->setGeometry( compNode->_compBufferRes->_geoID );

		curShader = Modules::renderer().getCurShader();

		// Set uniforms
		// World transformation
		if ( curShader->uniLocs[ uni.worldMat ] >= 0 )
		{
			rdi->setShaderConst( curShader->uniLocs[ uni.worldMat ], CONST_FLOAT44, &compNode->_absTrans.x[ 0 ] );
		}
		if ( curShader->uniLocs[ uni.nodeId ] >= 0 )
		{
			float id = ( float ) compNode->getHandle();
			rdi->setShaderConst( curShader->uniLocs[ uni.nodeId ], CONST_FLOAT, &id );
		}
		
		// Wait for completion of compute operation (writing to buffer)
		rdi->setMemoryBarrier( VertexBufferBarrier );

		// Render
		rdi->draw( drawType, 0, compNode->_elementsCount );
		Modules::stats().incStat( EngineStats::BatchCount, 1 );
		Modules::stats().incStat( EngineStats::TriCount, ( float ) compNode->_elementsCount );
	}

	timer->endQuery();
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
	_renderDevice->beginRendering();
	_renderDevice->setViewport( _curCamera->_vpX, _curCamera->_vpY, _curCamera->_vpWidth, _curCamera->_vpHeight );

	// Perform culling
	prepareRenderViews();

	if( Modules::config().debugViewMode || _curCamera->_pipelineRes == 0x0 )
	{
		renderDebugView();
		finishRendering();
		return;
	}
	
	// Initialize
	_renderDevice->_outputBufferIndex = _curCamera->_outputBufferIndex;
	if( _curCamera->_outputTex != 0x0 )
		_renderDevice->setRenderBuffer( _curCamera->_outputTex->getRBObject() );
	else 
		_renderDevice->setRenderBuffer( 0 );

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
			case DefaultPipelineCommands::SwitchTarget:
				// Unbind all textures
				bindPipeBuffer( 0x0, "", 0 );
				
				// Bind new render target
				rt = (RenderTarget *)pc.params[0].getPtr();
				_curRenderTarget = rt;

				if( rt != 0x0 )
				{
					int width, height;
					_renderDevice->getRenderBufferDimensions( rt->rendBuf, &width, &height );
					_renderDevice->_outputBufferIndex = _curCamera->_outputBufferIndex;
					_renderDevice->setViewport( 0, 0, width, height );
					_renderDevice->setRenderBuffer( rt->rendBuf );
				}
				else
				{
					_renderDevice->setViewport( _curCamera->_vpX, _curCamera->_vpY, _curCamera->_vpWidth, _curCamera->_vpHeight );
					_renderDevice->setRenderBuffer( _curCamera->_outputTex != 0x0 ?
					                       _curCamera->_outputTex->getRBObject() : 0 );
				}
				break;

			case DefaultPipelineCommands::BindBuffer:
				rt = (RenderTarget *)pc.params[0].getPtr();
				bindPipeBuffer( rt->rendBuf, pc.params[1].getString(), (uint32)pc.params[2].getInt() );
				break;

			case DefaultPipelineCommands::UnbindBuffers:
				bindPipeBuffer( 0x0, "", 0 );
				break;

			case DefaultPipelineCommands::ClearTarget:
				clear( pc.params[0].getBool(), pc.params[1].getBool(), pc.params[2].getBool(),
				       pc.params[3].getBool(), pc.params[4].getBool(), pc.params[5].getFloat(),
				       pc.params[6].getFloat(), pc.params[7].getFloat(), pc.params[8].getFloat() );
				break;

			case DefaultPipelineCommands::DrawGeometry:
				drawGeometry( pc.params[0].getString(), pc.params[1].getInt(),
				              (RenderingOrder::List)pc.params[2].getInt(), _curCamera->_occSet );
				break;

			case DefaultPipelineCommands::DrawQuad:
				drawFSQuad( pc.params[0].getResource(), pc.params[1].getString() );
			break;

			case DefaultPipelineCommands::DoForwardLightLoop:
				drawLightGeometry( pc.params[0].getString(), pc.params[1].getInt(),
				                   pc.params[2].getBool(), (RenderingOrder::List)pc.params[3].getInt(),
								   _curCamera->_occSet );
				break;

			case DefaultPipelineCommands::DoDeferredLightLoop:
				drawLightShapes( pc.params[0].getString(), pc.params[1].getBool(), _curCamera->_occSet );
				break;

			case DefaultPipelineCommands::SetUniform:
				if( pc.params[0].getResource() && pc.params[0].getResource()->getType() == ResourceTypes::Material )
				{
					((MaterialResource *)pc.params[0].getResource())->setUniform( pc.params[1].getString(),
						pc.params[2].getFloat(), pc.params[3].getFloat(),
						pc.params[4].getFloat(), pc.params[5].getFloat() );
				}
				break;

			case DefaultPipelineCommands::ExternalCommand:
				Modules::pipeMan().executeCommand( pc );
				break;
			}
		}
	}
	
	// Update mipmaps if necessary
	if( _curCamera->_outputTex != 0x0 && _curCamera->_outputTex->getMaxMipLevel() > 0 )
		_renderDevice->generateTextureMipmap( _curCamera->_outputTex->getTexObject() );

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
	
	_renderDevice->_outputBufferIndex = _curCamera->_outputBufferIndex;
	if( _curCamera->_outputTex != 0x0 )
		_renderDevice->setRenderBuffer( _curCamera->_outputTex->getRBObject() );
	else 
		_renderDevice->setRenderBuffer( 0 );
	setMaterial( 0x0, "" );
	_renderDevice->setFillMode( RS_FILL_WIREFRAME );

	_renderDevice->clear( CLR_DEPTH | CLR_COLOR_RT0 );

// 	Modules::sceneMan().updateQueues( _curCamera->getFrustum(), 0x0, RenderingOrder::None,
// 	                                  SceneNodeFlags::NoDraw, true, true );

	// Draw renderable nodes as wireframe
	setupViewMatrices( _curCamera->getViewMat(), _curCamera->getProjMat() );
	drawRenderables( "", 0, true, &_curCamera->getFrustum(), 0x0, RenderingOrder::None, -1 );

	// Draw bounding boxes
	_renderDevice->setCullMode( RS_CULL_NONE );
	setMaterial( 0x0, "" );
	setShaderComb( &_defColorShader );
	commitGeneralUniforms();
	
	Matrix4f identity;
	_renderDevice->setShaderConst( _defColorShader.uniLocs[ _uni.worldMat ], CONST_FLOAT44, &identity.x[0] );
	color[0] = 0.4f; color[1] = 0.4f; color[2] = 0.4f; color[3] = 1;
	_renderDevice->setShaderConst( Modules::renderer()._defColShader_color, CONST_FLOAT4, color );
	for( uint32 i = 0, s = (uint32)Modules::sceneMan().getRenderQueue().size(); i < s; ++i )
	{
		SceneNode *sn = Modules::sceneMan().getRenderQueue()[i].node;
		
		drawAABB( sn->_bBox.min, sn->_bBox.max );
	}
	_renderDevice->setCullMode( RS_CULL_BACK );

	// Draw light volumes
	_renderDevice->setFillMode( RS_FILL_SOLID );
	_renderDevice->setBlendMode( true, BS_BLEND_SRC_ALPHA, BS_BLEND_ONE );
	_renderDevice->setCullMode( RS_CULL_FRONT );
	color[0] = 1; color[1] = 1; color[2] = 0; color[3] = 0.25f;
	_renderDevice->setShaderConst( Modules::renderer()._defColShader_color, CONST_FLOAT4, color );
	
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

	_renderDevice->setCullMode( RS_CULL_BACK );
	_renderDevice->setBlendMode( false );
}


void Renderer::finishRendering()
{
	_shadowParams.resize( 0 );

	_renderDevice->setRenderBuffer( 0 );
	setMaterial( 0x0, "" );
	_renderDevice->resetStates();
}


}  // namespace
