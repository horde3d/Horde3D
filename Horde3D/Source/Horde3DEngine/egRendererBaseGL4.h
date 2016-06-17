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

#ifndef _egRendererBaseGL4_H_
#define _egRendererBaseGL4_H_

#include "egRendererBase.h"
#include "utOpenGL.h"
#include <string>
#include <vector>


namespace Horde3D {
namespace RDI_GL4 {

const uint32 MaxNumVertexLayouts = 32;


// =================================================================================================
// GPUTimer
// =================================================================================================

class GPUTimerGL4 : public GPUTimer
{
public:
	GPUTimerGL4();
	~GPUTimerGL4();
	
	void beginQuery( uint32 frameID );
	void endQuery();
	bool updateResults();
	
	void reset();
//	float getTimeMS() const { return _time; }

private:
	std::vector < uint32 >  _queryPool;
	uint32                  _numQueries;
	uint32                  _queryFrame;
//	float                   _time;
	bool                    _activeQuery;
};


// =================================================================================================
// Render Device Interface
// =================================================================================================

// ---------------------------------------------------------
// Buffers
// ---------------------------------------------------------

struct RDIBufferGL4
{
	uint32  type;
	uint32  glObj;
	uint32  size;
};

struct RDIVertBufSlotGL4
{
	uint32  vbObj;
	uint32  offset;
	uint32  stride;

	RDIVertBufSlotGL4() : vbObj( 0 ), offset( 0 ), stride( 0 ) {}
	RDIVertBufSlotGL4( uint32 vbObj, uint32 offset, uint32 stride ) :
		vbObj( vbObj ), offset( offset ), stride( stride ) {}
};

struct RDIGeometryInfoGL4
{
	std::vector< RDIVertBufSlotGL4 > vertexBufInfo;
	uint32 vao;
	uint32 indexBuf;
	uint32 layout;
	bool indexBuf32Bit;
	bool atrribsBinded;

	RDIGeometryInfoGL4() : vao( 0 ), indexBuf( 0 ), layout( 0 ), indexBuf32Bit( false ), atrribsBinded( false ) {}
};

// ---------------------------------------------------------
// Textures
// ---------------------------------------------------------

struct RDITextureGL4
{
	uint32                glObj;
	uint32                glFmt;
	int                   type;
	TextureFormats::List  format;
	int                   width, height, depth;
	int                   memSize;
	uint32                samplerState;
	bool                  sRGB;
	bool                  hasMips, genMips;
};

struct RDITexSlotGL4
{
	uint32  texObj;
	uint32  samplerState;

	RDITexSlotGL4() : texObj( 0 ), samplerState( 0 ) {}
	RDITexSlotGL4( uint32 texObj, uint32 samplerState ) :
		texObj( texObj ), samplerState( samplerState ) {}
};


// ---------------------------------------------------------
// Shaders
// ---------------------------------------------------------

struct RDIInputLayoutGL4
{
	bool  valid;
	int8  attribIndices[16];
};

struct RDIShaderGL4
{
	uint32          oglProgramObj;
	RDIInputLayoutGL4  inputLayouts[MaxNumVertexLayouts];
};


// ---------------------------------------------------------
// Render buffers
// ---------------------------------------------------------

struct RDIRenderBufferGL4
{
	static const uint32 MaxColorAttachmentCount = 4;

	uint32  fbo, fboMS;  // fboMS: Multisampled FBO used when samples > 0
	uint32  width, height;
	uint32  samples;

	uint32  depthTex, colTexs[MaxColorAttachmentCount];
	uint32  depthBuf, colBufs[MaxColorAttachmentCount];  // Used for multisampling

	RDIRenderBufferGL4() : fbo( 0 ), fboMS( 0 ), width( 0 ), height( 0 ), depthTex( 0 ), depthBuf( 0 ), samples( 0 )
	{
		for( uint32 i = 0; i < MaxColorAttachmentCount; ++i ) colTexs[i] = colBufs[i] = 0;
	}
};

// =================================================================================================


class RenderDeviceGL4 : public RenderDeviceInterface
{
public:

	RenderDeviceGL4();
	~RenderDeviceGL4();
	
	void initStates();
	bool init();
	
// -----------------------------------------------------------------------------
// Resources
// -----------------------------------------------------------------------------

	// Vertex layouts
	uint32 registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs );
	
	// Buffers
	void beginRendering();
	uint32 beginCreatingGeometry( uint32 vlObj );
	void finishCreatingGeometry( uint32 geoObj );
	void setGeomVertexParams( uint32 geoObj, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride );
	void setGeomIndexParams( uint32 geoObj, uint32 indBuf, RDIIndexFormat format );
	void destroyGeometry( uint32 geoObj );

	uint32 createVertexBuffer( uint32 size, const void *data );
	uint32 createIndexBuffer( uint32 size, const void *data );
	void destroyBuffer( uint32 bufObj );
	void updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data );
	uint32 getBufferMem() const { return _bufferMem; }

	// Textures
	uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth );
	uint32 createTexture( TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
	                      bool hasMips, bool genMips, bool compress, bool sRGB );
	void uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	void destroyTexture( uint32 texObj );
	void updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	bool getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer );
	uint32 getTextureMem() const { return _textureMem; }

	// Shaders
	uint32 createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
						 const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc );
	void destroyShader( uint32 shaderId );
	void bindShader( uint32 shaderId );
	std::string getShaderLog() const { return _shaderLog; }
	int getShaderConstLoc( uint32 shaderId, const char *name );
	int getShaderSamplerLoc( uint32 shaderId, const char *name );
	void setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count = 1 );
	void setShaderSampler( int loc, uint32 texUnit );
	const char *getDefaultVSCode();
	const char *getDefaultFSCode();
	void runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim );

	// Renderbuffers
	uint32 createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
	                           bool depth, uint32 numColBufs, uint32 samples );
	void destroyRenderBuffer( uint32 rbObj );
	uint32 getRenderBufferTex( uint32 rbObj, uint32 bufIndex );
	void setRenderBuffer( uint32 rbObj );
	bool getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
	                          int *compCount, void *dataBuffer, int bufferSize );
	void getRenderBufferDimensions( uint32 rbObj, int *width, int *height );

	// Queries
	uint32 createOcclusionQuery();
	void destroyQuery( uint32 queryObj );
	void beginQuery( uint32 queryObj );
	void endQuery( uint32 queryObj );
	uint32 getQueryResult( uint32 queryObj );

	// Render Device dependent GPU Timer
	GPUTimer *createGPUTimer()
	{
		return new GPUTimerGL4(); 
	}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------
// 	
// 	void setViewport( int x, int y, int width, int height )
// 		{ _vpX = x; _vpY = y; _vpWidth = width; _vpHeight = height; _pendingMask |= PM_VIEWPORT; }
// 	void setScissorRect( int x, int y, int width, int height )
// 		{ _scX = x; _scY = y; _scWidth = width; _scHeight = height; _pendingMask |= PM_SCISSOR; }
// 	void setIndexBuffer( uint32 bufObj, RDIIndexFormat idxFmt )
// 		{ _indexFormat = (uint32)idxFmt; _newIndexBuf = bufObj; _pendingMask |= PM_INDEXBUF; }
// 	void setVertexBuffer( uint32 slot, uint32 vbObj, uint32 offset, uint32 stride )
// 		{ ASSERT( slot < 16 ); _vertBufSlots[slot] = RDIVertBufSlotGL4( vbObj, offset, stride );
// 	      _pendingMask |= PM_VERTLAYOUT; }
// 	void setVertexLayout( uint32 vlObj )
// 		{ _newVertLayout = vlObj; }
// 	void setTexture( uint32 slot, uint32 texObj, uint16 samplerState )
// 		{ ASSERT( slot < 16 ); _texSlots[slot] = RDITexSlotGL4( texObj, samplerState );
// 	      _pendingMask |= PM_TEXTURES; }
// 	
// 	// Render states
// 	void setColorWriteMask( bool enabled )
// 		{ _newRasterState.renderTargetWriteMask = enabled; _pendingMask |= PM_RENDERSTATES; }
// 	void getColorWriteMask( bool &enabled ) const
// 		{ enabled = _newRasterState.renderTargetWriteMask; }
// 	void setFillMode( RDIFillMode fillMode )
// 		{ _newRasterState.fillMode = fillMode; _pendingMask |= PM_RENDERSTATES; }
// 	void getFillMode( RDIFillMode &fillMode ) const
// 		{ fillMode = (RDIFillMode)_newRasterState.fillMode; }
// 	void setCullMode( RDICullMode cullMode )
// 		{ _newRasterState.cullMode = cullMode; _pendingMask |= PM_RENDERSTATES; }
// 	void getCullMode( RDICullMode &cullMode ) const
// 		{ cullMode = (RDICullMode)_newRasterState.cullMode; }
// 	void setScissorTest( bool enabled )
// 		{ _newRasterState.scissorEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
// 	void getScissorTest( bool &enabled ) const
// 		{ enabled = _newRasterState.scissorEnable; }
// 	void setMulisampling( bool enabled )
// 		{ _newRasterState.multisampleEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
// 	void getMulisampling( bool &enabled ) const
// 		{ enabled = _newRasterState.multisampleEnable; }
// 	void setAlphaToCoverage( bool enabled )
// 		{ _newBlendState.alphaToCoverageEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
// 	void getAlphaToCoverage( bool &enabled ) const
// 		{ enabled = _newBlendState.alphaToCoverageEnable; }
// 	void setBlendMode( bool enabled, RDIBlendFunc srcBlendFunc = BS_BLEND_ZERO, RDIBlendFunc destBlendFunc = BS_BLEND_ZERO )
// 		{ _newBlendState.blendEnable = enabled; _newBlendState.srcBlendFunc = srcBlendFunc;
// 		  _newBlendState.destBlendFunc = destBlendFunc; _pendingMask |= PM_RENDERSTATES; }
// 	void getBlendMode( bool &enabled, RDIBlendFunc &srcBlendFunc, RDIBlendFunc &destBlendFunc ) const
// 		{ enabled = _newBlendState.blendEnable; srcBlendFunc = (RDIBlendFunc)_newBlendState.srcBlendFunc;
// 		  destBlendFunc = (RDIBlendFunc)_newBlendState.destBlendFunc; }
// 	void setDepthMask( bool enabled )
// 		{ _newDepthStencilState.depthWriteMask = enabled; _pendingMask |= PM_RENDERSTATES; }
// 	void getDepthMask( bool &enabled ) const
// 		{ enabled = _newDepthStencilState.depthWriteMask; }
// 	void setDepthTest( bool enabled )
// 		{ _newDepthStencilState.depthEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
// 	void getDepthTest( bool &enabled ) const
// 		{ enabled = _newDepthStencilState.depthEnable; }
// 	void setDepthFunc( RDIDepthFunc depthFunc )
// 		{ _newDepthStencilState.depthFunc = depthFunc; _pendingMask |= PM_RENDERSTATES; }
// 	void getDepthFunc( RDIDepthFunc &depthFunc ) const
// 		{ depthFunc = (RDIDepthFunc)_newDepthStencilState.depthFunc; }

	bool commitStates( uint32 filter = 0xFFFFFFFF );
	void resetStates();
	
	// Draw calls and clears
	void clear( uint32 flags, float *colorRGBA = 0x0, float depth = 1.0f );
	void draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts );
	void drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
	                  uint32 firstVert, uint32 numVerts );

// -----------------------------------------------------------------------------
// Getters
// -----------------------------------------------------------------------------

	const DeviceCaps getCaps() const { return _caps; }
	const RDIBufferGL4 getBuffer( uint32 bufObj ) { return _buffers.getRef( bufObj ); }
	const RDITextureGL4 getTexture( uint32 texObj ) { return _textures.getRef( texObj ); }
	const RDIRenderBufferGL4 getRenderBuffer( uint32 rbObj ) { return _rendBufs.getRef( rbObj ); }

//	friend class Renderer;

protected:

// 	enum RDIPendingMask
// 	{
// 		PM_VIEWPORT      = 0x00000001,
// 		PM_INDEXBUF      = 0x00000002,
// 		PM_VERTLAYOUT    = 0x00000004,
// 		PM_TEXTURES      = 0x00000008,
// 		PM_SCISSOR       = 0x00000010,
// 		PM_RENDERSTATES  = 0x00000020
// 	};

protected:

	uint32 createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc, 
								const char *tessControlShaderSrc, const char *tessEvalShaderSrc, const char *computeShaderSrc );
	bool linkShaderProgram( uint32 programObj );
	void resolveRenderBuffer( uint32 rbObj );

	void checkError();
	bool applyVertexLayout( RDIGeometryInfoGL4 &geo );
	void applySamplerState( RDITextureGL4 &tex );
	void applyRenderStates();

protected:

// 	DeviceCaps    _caps;
// 	
// 	uint32        _depthFormat;
// 	int           _vpX, _vpY, _vpWidth, _vpHeight;
// 	int           _scX, _scY, _scWidth, _scHeight;
// 	int           _fbWidth, _fbHeight;
// 	std::string   _shaderLog;
// 	uint32        _curRendBuf;
// 	int           _outputBufferIndex;  // Left and right eye for stereo rendering
// 	uint32        _textureMem, _bufferMem;
// 
// 	int                            _defaultFBO;
//     bool                           _defaultFBOMultisampled;

//	uint32							  _numVertexLayouts;
	RDIVertexLayout		              _vertexLayouts[MaxNumVertexLayouts];
	RDIObjects< RDIBufferGL4 >        _buffers;
	RDIObjects< RDITextureGL4 >       _textures;
	RDIObjects< RDIShaderGL4 >        _shaders;
	RDIObjects< RDIRenderBufferGL4 >  _rendBufs;
	RDIObjects< RDIGeometryInfoGL4 >  _vaos;

//	RDIGeometryInfoGL4				  _tempGeometry;

// 	RDIVertBufSlotGL4        _vertBufSlots[16];
// 	RDITexSlotGL4            _texSlots[16];
// 	RDIRasterState        _curRasterState, _newRasterState;
// 	RDIBlendState         _curBlendState, _newBlendState;
// 	RDIDepthStencilState  _curDepthStencilState, _newDepthStencilState;
	uint32                _prevShaderId, _curShaderId;
// 	uint32                _curVertLayout, _newVertLayout;
// 	uint32                _curIndexBuf, _newIndexBuf;
 	uint32                _indexFormat;
 	uint32                _activeVertexAttribsMask;
// 	uint32                _pendingMask;
};

} // namespace RDI_GL4
} // namespace Horde3D

#endif // _egRendererBaseGL4_H_
