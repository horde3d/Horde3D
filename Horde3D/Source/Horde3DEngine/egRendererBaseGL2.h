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

#ifndef _egRendererBaseGL2_H_
#define _egRendererBaseGL2_H_

#include "egRendererBase.h"
#include <string.h>


namespace Horde3D {

namespace RDI_GL2 {

const uint32 MaxNumVertexLayouts = 16;


// =================================================================================================
// GPUTimer
// =================================================================================================

class GPUTimerGL2 : public GPUTimer
{
public:
	GPUTimerGL2();
	~GPUTimerGL2();
	
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


// struct DeviceCaps
// {
// 	bool  texFloat;
// 	bool  texNPOT;
// 	bool  rtMultisampling;
// };
// 
// 

// ---------------------------------------------------------
// Buffers
// ---------------------------------------------------------

struct RDIBufferGL2
{
	uint32  type;
	uint32  glObj;
	uint32  size;
	int		geometryRefCount;

	RDIBufferGL2() : type( 0 ), glObj( 0 ), size( 0 ), geometryRefCount( 0 ) {}
};

struct RDIVertBufSlotGL2
{
	uint32  vbObj;
	uint32  offset;
	uint32  stride;

	RDIVertBufSlotGL2() : vbObj( 0 ), offset( 0 ), stride( 0 ) {}
	RDIVertBufSlotGL2( uint32 vbObj, uint32 offset, uint32 stride ) :
		vbObj( vbObj ), offset( offset ), stride( stride ) {}
};

struct RDIGeometryInfoGL2
{
	std::vector< RDIVertBufSlotGL2 > vertexBufInfo;
	uint32 indexBufIdx;
	uint32 layout;
	bool indexBuf32Bit;
	
	RDIGeometryInfoGL2() : indexBufIdx( 0 ), layout( 0 ), indexBuf32Bit( false ) {}
};

// ---------------------------------------------------------
// Textures
// ---------------------------------------------------------

struct RDITextureGL2
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

	RDITextureGL2() : glObj( 0 ), glFmt( 0 ), type( 0 ), format( TextureFormats::Unknown ), width( 0 ), height( 0 ),
		depth( 0 ), memSize( 0 ), samplerState( 0 ), sRGB( false ), hasMips( false ), genMips( false )
	{

	}
};

struct RDITexSlotGL2
{
	uint32  texObj;
	uint32  samplerState;

	RDITexSlotGL2() : texObj( 0 ), samplerState( 0 ) {}
	RDITexSlotGL2( uint32 texObj, uint32 samplerState ) :
		texObj( texObj ), samplerState( samplerState ) {}
};

struct RDITextureBufferGL2
{
	uint32  bufObj;
	uint32  glFmt;
	uint32	glTexID;

	RDITextureBufferGL2() : bufObj( 0 ), glFmt( 0 ), glTexID( 0 ) {}
};

// ---------------------------------------------------------
// Shaders
// ---------------------------------------------------------

struct RDIInputLayoutGL2
{
	bool  valid;
	int8  attribIndices[16];

	RDIInputLayoutGL2() : valid( false )
	{
		memset( attribIndices, 0, sizeof( attribIndices ) );
	}
};

struct RDIShaderGL2
{
	uint32				oglProgramObj;
	RDIInputLayoutGL2	inputLayouts[MaxNumVertexLayouts];

	RDIShaderGL2() : oglProgramObj( 0 )
	{

	}
};


// ---------------------------------------------------------
// Render buffers
// ---------------------------------------------------------

struct RDIRenderBufferGL2
{
	static const uint32 MaxColorAttachmentCount = 4;

	uint32  fbo, fboMS;  // fboMS: Multisampled FBO used when samples > 0
	uint32  width, height;
	uint32  samples;

	uint32  depthTex, colTexs[MaxColorAttachmentCount];
	uint32  depthBuf, colBufs[MaxColorAttachmentCount];  // Used for multisampling

	RDIRenderBufferGL2() : fbo( 0 ), fboMS( 0 ), width( 0 ), height( 0 ), samples( 0 ), depthTex( 0 ), depthBuf( 0 )
	{
		for( uint32 i = 0; i < MaxColorAttachmentCount; ++i ) colTexs[i] = colBufs[i] = 0;
	}
};

// =================================================================================================


class RenderDeviceGL2 : public RenderDeviceInterface
{
	// The friend RenderDeviceInterface is needed to give access to the template
	// functions that are defined within the RenderDeviceInterface class while
	// preventing access to the interface member functions defined here
	// by making them private or protected.
	friend class RenderDeviceInterface;

public:

	RenderDeviceGL2();
	~RenderDeviceGL2();
	
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
	void destroyGeometry(uint32 &geoObj, bool destroyBindedBuffers );

	uint32 createVertexBuffer( uint32 size, const void *data );
	uint32 createIndexBuffer( uint32 size, const void *data );
	uint32 createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data );
	uint32 createShaderStorageBuffer( uint32 size, const void *data );
	void destroyBuffer(uint32 &bufObj );
	void destroyTextureBuffer(uint32 &bufObj );
	void updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data );
	void *mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType );
	void unmapBuffer( uint32 geoObj, uint32 bufObj );
	// 	uint32 getBufferMem() const { return _bufferMem; }

	// Textures
// 	uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth );
	uint32 createTexture( TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
	                      bool hasMips, bool genMips, bool compress, bool sRGB );
	void generateTextureMipmap( uint32 texObj );
	void uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	void destroyTexture( uint32& texObj );
	void updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	bool getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer );
// 	uint32 getTextureMem() const { return _textureMem; }
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
	                           bool depth, uint32 numColBufs, uint32 samples, bool hasMipmaps );
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
	GPUTimer *createGPUTimer() { return new GPUTimerGL2(); }

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
	RDIBufferGL2 &getBuffer( uint32 bufObj ) { return _buffers.getRef( bufObj ); }
	RDITextureGL2 &getTexture( uint32 texObj ) { return _textures.getRef( texObj ); }
	RDIRenderBufferGL2 &getRenderBuffer( uint32 rbObj ) { return _rendBufs.getRef( rbObj ); }

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

	uint32 createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc );
	bool linkShaderProgram( uint32 programObj );
	void resolveRenderBuffer( uint32 rbObj );
	inline uint32 createBuffer( uint32 type, uint32 size, const void *data );

	void checkError();
	bool applyVertexLayout( const RDIGeometryInfoGL2 &geo );
	void applySamplerState( RDITextureGL2 &tex );
	void applyRenderStates();

	inline void	  decreaseBufferRefCount( uint32 bufObj );

	bool isCompressedTextureFormat( TextureFormats::List fmt );

	void initRDIFuncs();
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

// 	int                            _defaultFBO;
//     bool                           _defaultFBOMultisampled;

//	uint32                         _numVertexLayouts;
	RDIVertexLayout						_vertexLayouts[MaxNumVertexLayouts];
	RDIObjects< RDIBufferGL2 >			_buffers;
	RDIObjects< RDITextureGL2 >			_textures;
	RDIObjects< RDITextureBufferGL2 >	_textureBuffs;
	RDIObjects< RDIShaderGL2 >			_shaders;
	RDIObjects< RDIRenderBufferGL2 >	_rendBufs;
	RDIObjects< RDIGeometryInfoGL2 >	_geometryInfo;

//  	RDIVertBufSlot						_vertBufSlots[16];
// 	RDITexSlot            _texSlots[16];
// 	RDIRasterState        _curRasterState, _newRasterState;
// 	RDIBlendState         _curBlendState, _newBlendState;
// 	RDIDepthStencilState  _curDepthStencilState, _newDepthStencilState;
//	uint32                _prevShaderId, _curShaderId;
// 	uint32                _curVertLayout, _newVertLayout;
	uint32                _curIndexBuf; //, _newIndexBuf;
 	uint32                _indexFormat;
 	uint32                _activeVertexAttribsMask;
// 	uint32                _pendingMask;
	bool                               _doubleBuffered;
};

} // namespace RDI_GL2
} // namespace Horde3D

#endif // _egRendererBaseGL2_H_
