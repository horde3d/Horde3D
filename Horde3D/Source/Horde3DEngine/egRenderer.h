// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2021 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _egRenderer_H_
#define _egRenderer_H_

#include "egPrerequisites.h"
#include "egRendererBase.h"
#include "egPrimitives.h"
#include "egModel.h"
#include <vector>
#include <algorithm>
#include <string>
#include <functional>

namespace Horde3D {

class MaterialResource;
class LightNode;
class CameraNode;
struct ShaderContext;

const uint32 ParticlesPerBatch = 64;	// Warning: The GPU must have enough registers
const uint32 QuadIndexBufCount = ParticlesPerBatch * 6;

#define OCCPROXYLIST_RENDERABLES 0
#define OCCPROXYLIST_LIGHTS 1

extern const char *vsOccBox;
extern const char *fsOccBox;
	

// =================================================================================================
// Renderer
// =================================================================================================

typedef void (*RenderFunc)( uint32 firstItem, uint32 lastItem, const std::string &shaderContext,
                            int theClass, bool debugView, const Frustum *frust1,
                            const Frustum *frust2, RenderingOrder::List order, int occSet );

struct RenderFuncListItem
{
	int         nodeType;
	RenderFunc  renderFunc;
};

struct RenderBackendType
{
	enum List
	{
		OpenGL2 = 2,
		OpenGL4 = 4,
		OpenGLES3 = 8
	};
};

// =================================================================================================

struct ParticleVert
{
	float  u, v;         // Texture coordinates
	float  index;        // Index in property array

	ParticleVert() {}

	ParticleVert( float u, float v ):
		u( u ), v( v ), index( 0 )
	{
	}
};

// =================================================================================================

struct OccProxy
{
	Vec3f   bbMin, bbMax;
	uint32  queryObj;

	OccProxy() {}
	OccProxy( const Vec3f &bbMin, const Vec3f &bbMax, uint32 queryObj ) :
		bbMin( bbMin ), bbMax( bbMax ), queryObj( queryObj )
	{
	}
};

struct PipeSamplerBinding
{
	char    sampler[64];
	uint32  rbObj;
	uint32  bufIndex;
};

struct EngineUniform
{
	std::string uniformName;
	size_t	hash;

	EngineUniform( const char *uniName ) 
	{
		uniformName.assign( uniName );
		hash = std::hash< std::string >{}( uniformName );
	}

	bool operator==( const EngineUniform &uni ) const
	{
		return hash == uni.hash;
	}
};

struct DefaultShaderUniforms
{
	int                 frameBufSize = -1;
	int                 viewMat = -1, viewMatInv = -1, projMat = -1, viewProjMat = -1, 
						viewProjMatInv = -1, viewerPos = -1;

	int                 worldMat = -1, worldNormalMat = -1, nodeId = -1, customInstData = -1;
	int                 skinMatRows = -1;
	int                 lightPos = -1, lightDir = -1, lightColor = -1;
	int                 shadowSplitDists = -1, shadowMats = -1, shadowMapSize = -1, shadowBias = -1;
	int                 parPosArray = -1, parSizeAndRotArray = -1, parColorArray = -1;
};

struct DefaultVertexLayouts
{
	enum List
	{
		Position = 0,
		Particle,
		Model
	};
};

struct ShadowParameters
{
	Matrix4f                           lightMats[ 4 ];
	Matrix4f						   lightProjMatrix[ 4 ];
	float                              splitPlanes[ 5 ] = { 0 };

	int								   viewID[ 4 ] = { 0 };
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	// Misc functions
	bool init( RenderBackendType::List type );
	void initStates();

	void registerRenderFunc( int nodeType, RenderFunc rf );

	unsigned char *useScratchBuf( uint32 minSize, uint32 alignment );
	void setupViewMatrices( const Matrix4f &viewMat, const Matrix4f &projMat );

	// Uniform handling
	int registerEngineUniform( const char *uniName );
	int getEngineUniform( const char *uniName );
	uint32 totalEngineUniforms() { return ( uint32 )_engineUniforms.size(); }
	DefaultShaderUniforms &getDefShaderUniIndices() { return _uni; }

	// Shader & material handling
	bool createShaderComb( ShaderCombination &sc, const char *vertexShader, const char *fragmentShader, const char *geometryShader,
						   const char *tessControlShader, const char *tessEvaluationShader, const char *computeShader );
	void releaseShaderComb( ShaderCombination &sc );
	void setShaderComb( ShaderCombination *sc );
	void commitGeneralUniforms();
	bool setMaterial( MaterialResource *materialRes, const std::string &shaderContext );
	
	bool createShadowRB( uint32 width, uint32 height );
	void releaseShadowRB();

	// Occlusion culling
	int registerOccSet();
	void unregisterOccSet( int occSet );
	void drawOccProxies( uint32 list );
	void pushOccProxy( uint32 list, const Vec3f &bbMin, const Vec3f &bbMax, uint32 queryObj )
		{ _occProxies[list].push_back( OccProxy( bbMin, bbMax, queryObj ) ); }
	
	// Drawing
	void drawAABB( const Vec3f &bbMin, const Vec3f &bbMax );
	void drawSphere( const Vec3f &pos, float radius );
	void drawCone( float height, float fov, const Matrix4f &transMat );

	static void drawMeshes( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, int theClass,
		bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );
	static void drawParticles( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, int theClass,
		bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );

#ifndef __EMSCRIPTEN__ // Compute shaders are not supported under emscripten
	static void drawComputeResults( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, int theClass,
									bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );
    void dispatchCompute( MaterialResource *materialRes, const std::string &context, uint32 groups_x, uint32 groups_y, uint32 groups_z );

#endif

	void render( CameraNode *camNode );
	void finalizeFrame();

	// Getters
	uint32 getFrameID() const { return _frameID; }
	ShaderCombination *getCurShader() const { return _curShader; }
	CameraNode *getCurCamera() const { return _curCamera; }
	uint32 getQuadIdxBuf() const { return _quadIdxBuf; }
	uint32 getParticleVBO() const { return _particleVBO; }
	uint32 getParticleGeometry() const { return _particleGeo; }
	uint32 getDefaultVertexLayout( DefaultVertexLayouts::List vl ) const;

	inline RenderDeviceInterface *getRenderDevice() const { return _renderDevice; }
	int getRenderDeviceType() { return _renderDeviceType; }

protected:
	
	void createPrimitives();
	
	bool setMaterialRec( MaterialResource *materialRes, const std::string &shaderContext, ShaderResource *shaderRes );
	
	void prepareRenderViews();

	// Shadows
	void setupShadowMap( bool noShadows );

	Matrix4f calcCropMatrixOld( const Frustum &frustSlice, const Vec3f lightPos, const Matrix4f &lightViewProjMat );
	Matrix4f calcCropMatrix( int renderView, const LightNode *light, const Matrix4f &lightViewProjMat );
	
	int prepareCropFrustum( const LightNode *light, const BoundingBox &viewBB );
	bool prepareShadowMapFrustum( const LightNode *light, int shadowView );
	void updateShadowMap();
	void updateShadowMapOld();

	// Drawing functions
	void bindPipeBuffer( uint32 rbObj, const std::string &sampler, uint32 bufIndex );
	void clear( bool depth, bool buf0, bool buf1, bool buf2, bool buf3, float r, float g, float b, float a );
	void drawFSQuad( Resource *matRes, const std::string &shaderContext );
	void drawGeometry( const std::string &shaderContext, int theClass,
	                   RenderingOrder::List order, int occSet );
	void drawLightGeometry( const std::string &shaderContext, int theClass,
	                        bool noShadows, RenderingOrder::List order, int occSet );
	void drawLightShapes( const std::string &shaderContext, bool noShadows, int occSet );
	
	void drawRenderables( const std::string &shaderContext, int theClass, bool debugView,
		const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );
	
	void renderDebugView();
	void finishRendering();

	RenderDeviceInterface	*createRenderDevice( int type );
	void					releaseRenderDevice();

protected:
	
	RenderDeviceInterface		       *_renderDevice;

	DefaultShaderUniforms			   _uni; // indices of engine uniforms in ShaderCombination uniLoc array

	std::vector< RenderFuncListItem >  _renderFuncRegistry;
	
	std::vector< PipeSamplerBinding >  _pipeSamplerBindings;
	std::vector< char >                _occSets;  // Actually bool
	std::vector< OccProxy >            _occProxies[2];  // 0: renderables, 1: lights

	std::vector< EngineUniform >	   _engineUniforms; // uniforms, that are used internally by the engine and extensions
	std::vector< ShadowParameters >	   _shadowParams; // shadow lightmaps and project matrices

	Matrix4f                           _viewMat, _viewMatInv, _projMat, _viewProjMat, _viewProjMatInv;

	unsigned char                      *_scratchBuf;
	uint32                             _scratchBufSize;

	// standard geometry
	uint32								_particleGeo;
	uint32								_cubeGeo;
	uint32								_sphereGeo;
	uint32								_coneGeo;
	uint32								_FSPolyGeo;

	uint32                             _shadowRB;
	uint32                             _frameID;
	uint32                             _defShadowMap;
	uint32                             _quadIdxBuf;
	uint32                             _particleVBO;

	MaterialResource                   *_curStageMatLink;
	CameraNode                         *_curCamera;
	LightNode                          *_curLight;
	ShaderCombination                  *_curShader;
	RenderTarget                       *_curRenderTarget;
	uint32                             _curShaderUpdateStamp;
	
	uint32                             _maxAnisoMask;
	float                              _smSize;

	float                              _splitPlanes[5];
	Matrix4f                           _lightMats[4];

	uint32                             _vlPosOnly, _vlModel, _vlParticle;
	ShaderCombination                  _defColorShader;
	int                                _defColShader_color;  // Uniform location
	
	uint32                             _vbCube, _ibCube, _vbSphere, _ibSphere;
	uint32                             _vbCone, _ibCone, _vbFSPoly;
	
	int									_renderDeviceType;

};

}
#endif // _egRenderer_H_
