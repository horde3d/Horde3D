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

#ifndef _egRendererBaseNull_H_
#define _egRendererBaseNull_H_

#include "egRendererBase.h"
#include <string.h>

#include <chrono>

namespace Horde3D {
namespace RDI_Null {

const uint32 MaxNumVertexLayouts = 64;
const uint32 MaxComputeImages = 8;

// =================================================================================================
// GPUTimer
// =================================================================================================

class GPUTimerNull : public GPUTimer
{
public:
	GPUTimerNull();
	~GPUTimerNull();
	
	void beginQuery( uint32 frameID );
	void endQuery();
	bool updateResults();
	
	void reset();
//	float getTimeMS() const { return _time; }

private:
	
    std::chrono::time_point< std::chrono::steady_clock > _t0, _t1;
    
	uint32                  _queryFrame;
	float                   _time;
	bool                    _activeQuery;
};


// =================================================================================================
// Render Device Interface
// =================================================================================================

// ---------------------------------------------------------
// Buffers
// ---------------------------------------------------------

struct RDIBufferNull
{
	uint32  type;
	uint32  glObj;
	uint32  size;
	int		geometryRefCount;

	RDIBufferNull() : type( 0 ), glObj( 0 ), size( 0 ), geometryRefCount( 0 ) {}
};

struct RDIVertBufSlotNull
{
	uint32  vbObj;
	uint32  offset;
	uint32  stride;

	RDIVertBufSlotNull() : vbObj( 0 ), offset( 0 ), stride( 0 ) {}
	RDIVertBufSlotNull( uint32 vbObj, uint32 offset, uint32 stride ) :
		vbObj( vbObj ), offset( offset ), stride( stride ) {}
};

struct RDIGeometryInfoNull
{
	std::vector< RDIVertBufSlotNull > vertexBufInfo;
	uint32 vao;
	uint32 indexBuf;
	uint32 layout;
	bool indexBuf32Bit;
	bool atrribsBinded;

	RDIGeometryInfoNull() : vao( 0 ), indexBuf( 0 ), layout( 0 ), indexBuf32Bit( false ), atrribsBinded( false ) {}
};

struct RDIShaderStorageNull
{
	uint32 	oglObject;
	uint8 	slot;

	RDIShaderStorageNull( uint8 targetSlot, uint32 glObj ) : oglObject( glObj ), slot( targetSlot )
	{

	}
};

// ---------------------------------------------------------
// Textures
// ---------------------------------------------------------

struct RDITextureNull
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

	RDITextureNull() : glObj( 0 ), glFmt( 0 ), type( 0 ), format( TextureFormats::Unknown ), width( 0 ), height( 0 ),
					  depth( 0 ), memSize( 0 ), samplerState( 0 ), sRGB( false ), hasMips( false ), genMips( false )
	{

	}
};

struct RDITexSlotNull
{
	uint32  texObj;
	uint32  samplerState;

	RDITexSlotNull() : texObj( 0 ), samplerState( 0 ) {}
	RDITexSlotNull( uint32 texObj, uint32 samplerState ) :
		texObj( texObj ), samplerState( samplerState ) {}
};

struct RDITextureBufferNull
{
	uint32  bufObj;
	uint32  glFmt;
	uint32	glTexID;

	RDITextureBufferNull() : bufObj( 0 ), glFmt( 0 ), glTexID( 0 ) {}
};

// ---------------------------------------------------------
// Shaders
// ---------------------------------------------------------

struct RDIInputLayoutNull 
{
	bool  valid;
	int8  attribIndices[16];

	RDIInputLayoutNull() : valid( false )
	{
		memset( attribIndices, 0, sizeof( attribIndices ) );
	}
};

struct RDIShaderNull 
{
	uint32				oglProgramObj;
	RDIInputLayoutNull	inputLayouts[MaxNumVertexLayouts];

	RDIShaderNull() : oglProgramObj( 0 )
	{
		
	}
};


// ---------------------------------------------------------
// Render buffers
// ---------------------------------------------------------

struct RDIRenderBufferNull 
{
	static const uint32 MaxColorAttachmentCount = 4;

	uint32  fbo, fboMS;  // fboMS: Multisampled FBO used when samples > 0
	uint32  width, height;
	uint32  samples;

	uint32  depthTex, colTexs[MaxColorAttachmentCount];
	uint32  depthBuf, colBufs[MaxColorAttachmentCount];  // Used for multisampling

	RDIRenderBufferNull() : fbo( 0 ), fboMS( 0 ), width( 0 ), height( 0 ), samples( 0 ), depthTex( 0 ), depthBuf( 0 )
	{
		for( uint32 i = 0; i < MaxColorAttachmentCount; ++i ) colTexs[i] = colBufs[i] = 0;
	}
};

// =================================================================================================


class RenderDeviceNull : public RenderDeviceInterface
{
public:

	RenderDeviceNull();
	~RenderDeviceNull();
	
	void initStates();
	bool init();
	
	bool enableDebugOutput();
	bool disableDebugOutput();

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
	void destroyGeometry(uint32 &geoObj, bool destroyBindedBuffers );

	uint32 createVertexBuffer( uint32 size, const void *data );
	uint32 createIndexBuffer( uint32 size, const void *data );
	uint32 createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data );
	uint32 createShaderStorageBuffer( uint32 size, const void *data );
	void destroyBuffer(uint32 &bufObj );
	void destroyTextureBuffer( uint32& bufObj );
	void updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data );
	void *mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType );
	void unmapBuffer( uint32 geoObj, uint32 bufObj );

	// Textures
// 	uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth );
	uint32 createTexture( TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
	                      int maxMipLevel, bool genMips, bool compress, bool sRGB );
	void generateTextureMipmap( uint32 texObj );
	void uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	void destroyTexture( uint32 &texObj );
	void updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	bool getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer );
	uint32 getTextureMem() const { return _textureMem; }
	void bindImageToTexture( uint32 texObj, void* eglImage );

	// Shaders
	uint32 createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
						 const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc );
	void destroyShader(uint32 &shaderId );
	void bindShader( uint32 shaderId );
	std::string getShaderLog() const { return _shaderLog; }
	int getShaderConstLoc( uint32 shaderId, const char *name );
	int getShaderSamplerLoc( uint32 shaderId, const char *name );
	int getShaderBufferLoc( uint32 shaderId, const char *name );
	void setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count = 1 );
	void setShaderSampler( int loc, uint32 texUnit );
	const char *getDefaultVSCode();
	const char *getDefaultFSCode();
	void runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim );

	// Renderbuffers
	uint32 createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
	                           bool depth, uint32 numColBufs, uint32 samples, uint32 maxMipLevel );
	void destroyRenderBuffer(uint32 &rbObj );
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
		return new GPUTimerNull(); 
	}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------
	void setStorageBuffer( uint8 slot, uint32 bufObj );

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

	// WARNING: Modifying internal states may lead to unexpected behavior and/or crashes
	RDIBufferNull &getBuffer( uint32 bufObj ) { return _buffers.getRef( bufObj ); }
	RDITextureNull &getTexture( uint32 texObj ) { return _textures.getRef( texObj ); }
	RDIRenderBufferNull &getRenderBuffer( uint32 rbObj ) { return _rendBufs.getRef( rbObj ); }

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
	bool applyVertexLayout( RDIGeometryInfoNull &geo );
	void applySamplerState( RDITextureNull &tex );
	void applyRenderStates();

	inline uint32 createBuffer( uint32 type, uint32 size, const void *data );

	inline void	  decreaseBufferRefCount( uint32 bufObj );

	bool isCompressedTextureFormat( TextureFormats::List fmt );

	void initRDIFuncs();
protected:

	RDIVertexLayout                    _vertexLayouts[MaxNumVertexLayouts];
	RDIObjects< RDIBufferNull >         _buffers;
	RDIObjects< RDITextureNull >        _textures;
	RDIObjects< RDITextureBufferNull >  _textureBuffs;
	RDIObjects< RDIShaderNull >         _shaders;
	RDIObjects< RDIRenderBufferNull >   _rendBufs;
	std::vector< RDIShaderStorageNull > _storageBufs;
    RDIObjects< RDIGeometryInfoNull >	_geometryInfo;


 	uint32                             _indexFormat;
 	uint32                             _activeVertexAttribsMask;

	uint32                             _lastTessPatchVertsValue;
	uint32                             _maxComputeBufferAttachments;

	bool                               _doubleBuffered;
};

} // namespace RDI_Null
} // namespace Horde3D

#endif // _egRendererBaseNull_H_

