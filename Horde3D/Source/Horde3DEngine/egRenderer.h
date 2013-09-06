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

#ifndef _egRenderer_H_
#define _egRenderer_H_

#include "egPrerequisites.h"
#include "egRendererBase.h"
#include "egPrimitives.h"
#include "egModel.h"
#include <vector>
#include <algorithm>


namespace Horde3D {

class MaterialResource;
class LightNode;
class CameraNode;
struct ShaderContext;

const uint32 MaxNumOverlayVerts = 2048;
const uint32 ParticlesPerBatch = 64;	// Warning: The GPU must have enough registers
const uint32 QuadIndexBufCount = MaxNumOverlayVerts * 6;

#define OCCPROXYLIST_RENDERABLES 0
#define OCCPROXYLIST_LIGHTS 1

extern const char *vsOccBox;
extern const char *fsOccBox;
	

// =================================================================================================
// Renderer
// =================================================================================================

typedef void (*RenderFunc)( uint32 firstItem, uint32 lastItem, const std::string &shaderContext,
                            const std::string &theClass, bool debugView, const Frustum *frust1,
                            const Frustum *frust2, RenderingOrder::List order, int occSet );

struct RenderFuncListItem
{
	int         nodeType;
	RenderFunc  renderFunc;
};

// =================================================================================================

struct OverlayBatch
{
	PMaterialResource  materialRes;
	uint32             firstVert, vertCount;
	float              colRGBA[4];
	int                flags;
	
	OverlayBatch() {}

	OverlayBatch( uint32 firstVert, uint32 vertCount, float *col, MaterialResource *materialRes, int flags ) :
		materialRes( materialRes ), firstVert( firstVert ), vertCount( vertCount ), flags( flags )
	{
		colRGBA[0] = col[0]; colRGBA[1] = col[1]; colRGBA[2] = col[2]; colRGBA[3] = col[3];
	}
 };

struct OverlayVert
{
	float  x, y;  // Position
	float  u, v;  // Texture coordinates
};


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


class Renderer
{
public:
	Renderer();
	~Renderer();

	void registerRenderFunc( int nodeType, RenderFunc rf );
	
	unsigned char *useScratchBuf( uint32 minSize );
	
	bool init();
	void initStates();

	void drawAABB( const Vec3f &bbMin, const Vec3f &bbMax );
	void drawSphere( const Vec3f &pos, float radius );
	void drawCone( float height, float fov, const Matrix4f &transMat );
	
	bool createShaderComb( const char *vertexShader, const char *fragmentShader, ShaderCombination &sc );
	void releaseShaderComb( ShaderCombination &sc );
	void setShaderComb( ShaderCombination *sc );
	void commitGeneralUniforms();
	bool setMaterial( MaterialResource *materialRes, const std::string &shaderContext );
	
	bool createShadowRB( uint32 width, uint32 height );
	void releaseShadowRB();

	int registerOccSet();
	void unregisterOccSet( int occSet );
	void drawOccProxies( uint32 list );
	void pushOccProxy( uint32 list, const Vec3f &bbMin, const Vec3f &bbMax, uint32 queryObj )
		{ _occProxies[list].push_back( OccProxy( bbMin, bbMax, queryObj ) ); }
	
	void showOverlays( const float *verts, uint32 vertCount, float *colRGBA,
	                   MaterialResource *matRes, int flags );
	void clearOverlays();
	
	static void drawMeshes( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, const std::string &theClass,
		bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );
	static void drawParticles( uint32 firstItem, uint32 lastItem, const std::string &shaderContext, const std::string &theClass,
		bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );

	void render( CameraNode *camNode );
	void finalizeFrame();

	uint32 getFrameID() { return _frameID; }
	ShaderCombination *getCurShader() { return _curShader; }
	CameraNode *getCurCamera() { return _curCamera; }
	uint32 getQuadIdxBuf() { return _quadIdxBuf; }
	uint32 getParticleVBO() { return _particleVBO; }

protected:
	void setupViewMatrices( const Matrix4f &viewMat, const Matrix4f &projMat );
	
	void createPrimitives();
	
	bool setMaterialRec( MaterialResource *materialRes, const std::string &shaderContext, ShaderResource *shaderRes );
	
	void setupShadowMap( bool noShadows );
	Matrix4f calcCropMatrix( const Frustum &frustSlice, const Vec3f lightPos, const Matrix4f &lightViewProjMat );
	void updateShadowMap();

	void drawOverlays( const std::string &shaderContext );

	void bindPipeBuffer( uint32 rbObj, const std::string &sampler, uint32 bufIndex );
	void clear( bool depth, bool buf0, bool buf1, bool buf2, bool buf3, float r, float g, float b, float a );
	void drawFSQuad( Resource *matRes, const std::string &shaderContext );
	void drawGeometry( const std::string &shaderContext, const std::string &theClass,
	                   RenderingOrder::List order, int occSet );
	void drawLightGeometry( const std::string &shaderContext, const std::string &theClass,
	                        bool noShadows, RenderingOrder::List order, int occSet );
	void drawLightShapes( const std::string &shaderContext, bool noShadows, int occSet );
	
	void drawRenderables( const std::string &shaderContext, const std::string &theClass, bool debugView,
		const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );
	
	void renderDebugView();
	void finishRendering();

protected:
	std::vector< RenderFuncListItem >  _renderFuncRegistry;
	
	unsigned char                      *_scratchBuf;
	uint32                             _scratchBufSize;

	Matrix4f                           _viewMat, _viewMatInv, _projMat, _viewProjMat, _viewProjMatInv;
	
	std::vector< PipeSamplerBinding >  _pipeSamplerBindings;
	std::vector< char >                _occSets;  // Actually bool
	std::vector< OccProxy >            _occProxies[2];  // 0: renderables, 1: lights
	
	std::vector< OverlayBatch >        _overlayBatches;
	OverlayVert                        *_overlayVerts;
	uint32                             _overlayVB;
	
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

	uint32                             _vlPosOnly, _vlOverlay, _vlModel, _vlParticle;
	ShaderCombination                  _defColorShader;
	int                                _defColShader_color;  // Uniform location
	
	uint32                             _vbCube, _ibCube, _vbSphere, _ibSphere;
	uint32                             _vbCone, _ibCone, _vbFSPoly;
};

}
#endif // _egRenderer_H_
