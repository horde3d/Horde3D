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

#ifndef _egRendererBase_H_
#define _egRendererBase_H_

#include "egPrerequisites.h"
#include "utMath.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

namespace Horde3D {

// =================================================================================================

//
// Delegate interface inspired by works of marcmo (github.com/marcmo/delegates) 
// and Stefan Reinalter (https://blog.molecular-matters.com/)
//
template <typename T>
class RDIDelegate {};

template <typename R, typename... Params>
class RDIDelegate< R (Params...) >
{
	typedef void* instancePtr;
	typedef R (*internalFunction) (void *instance, Params... );
	typedef std::pair< void *, internalFunction > methodStub;

	template <class C, R ( C::*func )( Params... ) >
	static H3D_INLINE R classMethodStub( void* instance, Params... args )
	{
		return ( static_cast< C* >( instance )->*func )( args... );
	}

public:

	RDIDelegate() : stub( nullptr, nullptr )
	{}

	template <class C, R ( C::*func )( Params... ) >
	void bind( C *instance )
	{
		stub.first = instance;
		stub.second = &classMethodStub< C, func >;
	}

	R invoke( Params... args )
	{
		return stub.second( stub.first, args...  );
	}
private:

	methodStub stub;
};

// =================================================================================================
// GPUTimer
// =================================================================================================

class GPUTimer
{
protected:

	GPUTimer()
	{

	}

	// Delegate interface
	RDIDelegate< void( uint32 ) > _beginQuery;
	RDIDelegate< void() >		  _endQuery;
	RDIDelegate< bool()	>		  _updateResults;
	RDIDelegate< void() >		  _reset;

public:
	
	// -----------------------------------------------------------------------------
	// Interface
	// -----------------------------------------------------------------------------
	virtual ~GPUTimer()
	{

	}
	
	void beginQuery( uint32 frameID )
	{
		_beginQuery.invoke( frameID );
	}
	void endQuery()
	{
		_endQuery.invoke();
	}
	bool updateResults()
	{
		return _updateResults.invoke();
	}
	
	void reset()
	{
		_reset.invoke();
	}

	float getTimeMS() const { return _time; }

protected:
	float                   _time;
};


// =================================================================================================
// Render Device Interface
// =================================================================================================

// ---------------------------------------------------------
// General
// ---------------------------------------------------------

template< class T > class RDIObjects
{
public:

	uint32 add( const T &obj )
	{
		if( !_freeList.empty() )
		{
			uint32 index = _freeList.back();
			_freeList.pop_back();
			_objects[index] = obj;
			return index + 1;
		}
		else
		{
			_objects.push_back( obj );
			return (uint32)_objects.size();
		}
	}

	void remove( uint32 handle )
	{
		ASSERT( handle > 0 && handle <= _objects.size() );
		ASSERT( std::find( _freeList.begin(), _freeList.end(), handle - 1 ) == _freeList.end() );
		
		_objects[handle - 1] = T();  // Destruct and replace with default object
		_freeList.push_back( handle - 1 );
	}

	T &getRef( uint32 handle )
	{
		ASSERT( handle > 0 && handle <= _objects.size() );
		
		return _objects[handle - 1];
	}

private:
	std::vector< T >       _objects;
	std::vector< uint32 >  _freeList;
};


struct DeviceCaps
{
	uint16	maxJointCount;
	uint16	maxTexUnitCount;
	bool	texFloat;
	bool	texNPOT;
	bool	rtMultisampling;
	bool	geometryShaders;
	bool	tesselation;
	bool	computeShaders;
	bool	instancing;
	bool	texDXT;
	bool	texETC2;
	bool	texASTC;
	bool	texBPTC;
};


// ---------------------------------------------------------
// Vertex layout
// ---------------------------------------------------------

struct VertexLayoutAttrib
{
	std::string  semanticName;
	uint32       vbSlot;
	uint32       size;
	uint32       offset;
};

struct RDIVertexLayout
{
	uint32              numAttribs;
	VertexLayoutAttrib  attribs[16];
};


// ---------------------------------------------------------
// Buffers
// ---------------------------------------------------------

// struct RDIBufferTypes
// {
// 	enum List
// 	{
// 		VertexBuffer = 0,
// 		IndexBuffer,
// 		TextureBuffer
// 	};
// };
enum RDIBufferMappingTypes
{
	Read = 0,
	Write,
	ReadWrite
};

// ---------------------------------------------------------
// Textures
// ---------------------------------------------------------

struct TextureTypes
{
	enum List
	{
		Tex2D = 0,
		Tex3D,
		TexCube
	};
};

struct TextureFormats
{
	enum List
	{
		Unknown,
		R8,
		R16F,
		R32F,
		RG8,
		RG16F,
		RG32F,
		BGRA8,
		RGBA16F,
		RGBA32F,
		RGBA32UI,
		DXT1,
		DXT3,
		DXT5,
		ETC1,
		RGB8_ETC2,
		RGBA8_ETC2,
		BC6_UF16,
		BC6_SF16,
		BC7,
		ASTC_4x4,
		ASTC_5x4,
		ASTC_5x5,
		ASTC_6x5,
		ASTC_6x6,
		ASTC_8x5,
		ASTC_8x6,
		ASTC_8x8,
		ASTC_10x5,
		ASTC_10x6,
		ASTC_10x8,
		ASTC_10x10,
		ASTC_12x10,
		ASTC_12x12,
		DEPTH
	};
};

struct TextureUsage
{
	enum List
	{
		Texture = 0,
		ComputeImageRO, // read-only image
		ComputeImageWO, // write-only image
		ComputeImageRW  // read-write image
	};
};

struct RDITexSlot
{
	uint32  texObj;
	uint32  samplerState;
	uint32  usage;

	RDITexSlot() : texObj( 0 ), samplerState( 0 ), usage( 0 ) {}
	RDITexSlot( uint32 texObj, uint32 samplerState, uint32 usage ) :
		texObj( texObj ), samplerState( samplerState ), usage( usage ) {}
};


// ---------------------------------------------------------
// Shaders
// ---------------------------------------------------------

enum RDIShaderConstType
{
	CONST_FLOAT,
	CONST_FLOAT2,
	CONST_FLOAT3,
	CONST_FLOAT4,
	CONST_FLOAT44,
	CONST_FLOAT33,
	CONST_INT,
	CONST_INT2,
	CONST_INT3,
	CONST_INT4
};

// ---------------------------------------------------------
// Render states
// ---------------------------------------------------------

// Note: Render states use unions to provide a hash value. Writing to and reading from different members of a
//       union is not guaranteed to work by the C++ standard but is common practice and supported by many compilers.

enum RDISamplerState
{
	SS_FILTER_BILINEAR   = 0x0,
	SS_FILTER_TRILINEAR  = 0x0001,
	SS_FILTER_POINT      = 0x0002,
	SS_ANISO1            = 0x0,
	SS_ANISO2            = 0x0004,
	SS_ANISO4            = 0x0008,
	SS_ANISO8            = 0x0010,
	SS_ANISO16           = 0x0020,
	SS_ADDRU_CLAMP       = 0x0,
	SS_ADDRU_WRAP        = 0x0040,
	SS_ADDRU_CLAMPCOL    = 0x0080,
	SS_ADDRV_CLAMP       = 0x0,
	SS_ADDRV_WRAP        = 0x0100,
	SS_ADDRV_CLAMPCOL    = 0x0200,
	SS_ADDRW_CLAMP       = 0x0,
	SS_ADDRW_WRAP        = 0x0400,
	SS_ADDRW_CLAMPCOL    = 0x0800,
	SS_ADDR_CLAMP        = SS_ADDRU_CLAMP | SS_ADDRV_CLAMP | SS_ADDRW_CLAMP,
	SS_ADDR_WRAP         = SS_ADDRU_WRAP | SS_ADDRV_WRAP | SS_ADDRW_WRAP,
	SS_ADDR_CLAMPCOL     = SS_ADDRU_CLAMPCOL | SS_ADDRV_CLAMPCOL | SS_ADDRW_CLAMPCOL,
	SS_COMP_LEQUAL       = 0x1000
};

const uint32 SS_FILTER_START = 0;
const uint32 SS_FILTER_MASK = SS_FILTER_BILINEAR | SS_FILTER_TRILINEAR | SS_FILTER_POINT;
const uint32 SS_ANISO_START = 2;
const uint32 SS_ANISO_MASK = SS_ANISO1 | SS_ANISO2 | SS_ANISO4 | SS_ANISO8 | SS_ANISO16;
const uint32 SS_ADDRU_START = 6;
const uint32 SS_ADDRU_MASK = SS_ADDRU_CLAMP | SS_ADDRU_WRAP | SS_ADDRU_CLAMPCOL;
const uint32 SS_ADDRV_START = 8;
const uint32 SS_ADDRV_MASK = SS_ADDRV_CLAMP | SS_ADDRV_WRAP | SS_ADDRV_CLAMPCOL;
const uint32 SS_ADDRW_START = 10;
const uint32 SS_ADDRW_MASK = SS_ADDRW_CLAMP | SS_ADDRW_WRAP | SS_ADDRW_CLAMPCOL;
const uint32 SS_ADDR_START = 6;
const uint32 SS_ADDR_MASK = SS_ADDR_CLAMP | SS_ADDR_WRAP | SS_ADDR_CLAMPCOL;


enum RDIFillMode
{
	RS_FILL_SOLID = 0,
	RS_FILL_WIREFRAME = 1
};

enum RDICullMode
{
	RS_CULL_BACK = 0,
	RS_CULL_FRONT,
	RS_CULL_NONE,
};

struct RDIRasterState
{
	union
	{
		uint32  hash;
		struct
		{
			uint32  fillMode : 1;  // RDIFillMode
			uint32  cullMode : 2;  // RDICullMode
			uint32  scissorEnable : 1;
			uint32  multisampleEnable : 1;
			uint32  renderTargetWriteMask : 1;
		};
	};
};

enum RDIBlendFunc
{
	BS_BLEND_ZERO = 0,
	BS_BLEND_ONE,
	BS_BLEND_SRC_ALPHA,
	BS_BLEND_INV_SRC_ALPHA,
	BS_BLEND_DEST_ALPHA,
	BS_BLEND_INV_DEST_ALPHA,
	BS_BLEND_DEST_COLOR,
	BS_BLEND_SRC_COLOR,
	BS_BLEND_INV_DEST_COLOR,
	BS_BLEND_INV_SRC_COLOR
};

struct RDIBlendState
{
	union
	{
		uint32  hash;
		struct
		{
			uint32  alphaToCoverageEnable : 1;
			uint32  blendEnable : 1;
			uint32  srcBlendFunc : 4;
			uint32  destBlendFunc : 4;
		};
	};
};

enum RDIDepthFunc
{
	DSS_DEPTHFUNC_LESS_EQUAL = 0,
	DSS_DEPTHFUNC_LESS,
	DSS_DEPTHFUNC_EQUAL,
	DSS_DEPTHFUNC_GREATER,
	DSS_DEPTHFUNC_GREATER_EQUAL,
	DSS_DEPTHFUNC_ALWAYS
};

struct RDIDepthStencilState
{
	union
	{
		uint32  hash;
		struct
		{
			uint32  depthWriteMask : 1;
			uint32  depthEnable : 1;
			uint32  depthFunc : 4;  // RDIDepthFunc
		};
	};
};

// ---------------------------------------------------------
// Draw calls and clears
// ---------------------------------------------------------

enum RDIClearFlags
{
	CLR_COLOR_RT0 = 0x00000001,
	CLR_COLOR_RT1 = 0x00000002,
	CLR_COLOR_RT2 = 0x00000004,
	CLR_COLOR_RT3 = 0x00000008,
	CLR_DEPTH = 0x00000010
};

enum RDIIndexFormat
{
	IDXFMT_16 = 0,
	IDXFMT_32
};

enum RDIPrimType
{
	PRIM_TRILIST = 0,
	PRIM_TRISTRIP,
	PRIM_LINELIST,
	PRIM_POINTS,
	PRIM_PATCHES
};

enum RDIDrawBarriers
{
	NotSet = 0,
	VertexBufferBarrier,		// Wait till vertex buffer is updated by shaders
	IndexBufferBarrier,			// Wait till index buffer is updated by shaders
	ImageBarrier				// Wait till image is updated by shaders
};

class RenderDeviceInterface
{
// -----------------------------------------------------------------------------
// Delegate interface
// -----------------------------------------------------------------------------
protected:
	
	RDIDelegate< bool () >												_delegate_init;
	RDIDelegate< void () >												_delegate_initStates;
	RDIDelegate< bool() >												_delegate_enableDebugOutput;
	RDIDelegate< bool() >												_delegate_disableDebugOutput;
	RDIDelegate< uint32( uint32, VertexLayoutAttrib *) >				_delegate_registerVertexLayout;
	RDIDelegate< void () >												_delegate_beginRendering;

	RDIDelegate< uint32 ( uint32 ) >									_delegate_beginCreatingGeometry;
	RDIDelegate< void ( uint32 ) >										_delegate_finishCreatingGeometry;
	RDIDelegate< void ( uint32 &, bool ) >								_delegate_destroyGeometry;
	RDIDelegate< void ( uint32, uint32, uint32, uint32, uint32 ) >		_delegate_setGeomVertexParams;
	RDIDelegate< void ( uint32, uint32, RDIIndexFormat ) >				_delegate_setGeomIndexParams;
	
	RDIDelegate< uint32 ( uint32, const void * ) >						_delegate_createVertexBuffer;
	RDIDelegate< uint32 ( uint32, const void * ) >						_delegate_createIndexBuffer;
	RDIDelegate< uint32 ( TextureFormats::List, uint32, const void * ) > _delegate_createTextureBuffer;
	RDIDelegate< uint32 ( uint32, const void * ) >						_delegate_createShaderStorageBuffer;
	RDIDelegate< void ( uint32 & ) >									_delegate_destroyBuffer;
	RDIDelegate< void ( uint32 & ) >									_delegate_destroyTextureBuffer;
	RDIDelegate< void ( uint32, uint32, uint32, uint32, void *data ) >	_delegate_updateBufferData;
	RDIDelegate< void* ( uint32, uint32, uint32, uint32, RDIBufferMappingTypes ) > _delegate_mapBuffer;
	RDIDelegate< void ( uint32, uint32 ) >								_delegate_unmapBuffer;

	RDIDelegate< uint32 ( TextureTypes::List, int, int, int, TextureFormats::List, int, bool, bool, bool ) > _delegate_createTexture;
	RDIDelegate< void ( uint32 ) >										_delegate_generateTextureMipmap;
	RDIDelegate< void ( uint32, int, int, const void * ) >				_delegate_uploadTextureData;
	RDIDelegate< void ( uint32 & ) >									_delegate_destroyTexture;
	RDIDelegate< void ( uint32, int, int, const void * ) >				_delegate_updateTextureData;
	RDIDelegate< bool ( uint32, int, int, void * ) >					_delegate_getTextureData;
	RDIDelegate< void ( uint32, void * ) >								_delegate_bindImageToTexture;

	RDIDelegate< uint32 ( const char *, const char *, const char *, const char *, const char *, const char * ) > _delegate_createShader;
	RDIDelegate< void ( uint32 & ) >									_delegate_destroyShader;
	RDIDelegate< void ( uint32 ) >										_delegate_bindShader;
	RDIDelegate< int ( uint32, const char * ) >							_delegate_getShaderConstLoc;
	RDIDelegate< int ( uint32, const char * ) >							_delegate_getShaderSamplerLoc;
	RDIDelegate< int ( uint32, const char * ) >							_delegate_getShaderBufferLoc;
	RDIDelegate< void ( uint32, uint32, uint32, uint32 ) >				_delegate_runComputeShader;
	RDIDelegate< void ( int, RDIShaderConstType, void *values, uint32 ) > _delegate_setShaderConst;
	RDIDelegate< void ( int, uint32 ) >									_delegate_setShaderSampler;
	RDIDelegate< const char *() >										_delegate_getDefaultVSCode;
	RDIDelegate< const char *() >										_delegate_getDefaultFSCode;

	RDIDelegate< uint32 ( uint32, uint32, TextureFormats::List, bool, uint32, uint32, uint32 ) > _delegate_createRenderBuffer;
	RDIDelegate< void ( uint32 & ) >									_delegate_destroyRenderBuffer;
	RDIDelegate< uint32( uint32, uint32 ) >								_delegate_getRenderBufferTex;
	RDIDelegate< void ( uint32 ) >										_delegate_setRenderBuffer;
	RDIDelegate< bool ( uint32, int, int *, int *, int *, void *, int ) > _delegate_getRenderBufferData;
	RDIDelegate< void ( uint32, int *, int * ) >						_delegate_getRenderBufferDimensions;

	RDIDelegate< uint32 () >											_delegate_createOcclusionQuery;
	RDIDelegate< void ( uint32 ) >										_delegate_destroyQuery;
	RDIDelegate< void ( uint32 ) >										_delegate_beginQuery;
	RDIDelegate< void ( uint32 ) >										_delegate_endQuery;
	RDIDelegate< uint32 ( uint32 ) >									_delegate_getQueryResult;

	RDIDelegate< GPUTimer * () >										_delegate_createGPUTimer;

	RDIDelegate< bool ( uint32 ) >										_delegate_commitStates;
	RDIDelegate< void () >												_delegate_resetStates;

	RDIDelegate< void ( uint32, float *, float ) >						_delegate_clear;
	RDIDelegate< void ( RDIPrimType, uint32, uint32 ) >					_delegate_draw;
	RDIDelegate< void ( RDIPrimType, uint32, uint32, uint32, uint32 ) >	_delegate_drawIndexed;
	RDIDelegate< void ( uint8, uint32 ) >								_delegate_setStorageBuffer;

// -----------------------------------------------------------------------------
// Main interface
// -----------------------------------------------------------------------------
public:

	RenderDeviceInterface()
	{

	}
 	
	virtual ~RenderDeviceInterface()
	{

	}
	
	void initStates() 
	{
		_delegate_initStates.invoke();
	}
	bool init() 
	{ 
		return _delegate_init.invoke();
	}

	bool enableDebugOutput()
	{
		return _delegate_enableDebugOutput.invoke();
	}
	bool disableDebugOutput()
	{
		return _delegate_disableDebugOutput.invoke();
	}

// -----------------------------------------------------------------------------
// Resources
// -----------------------------------------------------------------------------

	// Vertex layouts
	uint32 registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs ) 
	{ 
		return _delegate_registerVertexLayout.invoke( numAttribs, attribs );
	}
	
	// Buffers
	void beginRendering() 
	{ 
		_delegate_beginRendering.invoke();
	}
	uint32 beginCreatingGeometry( uint32 vlObj )
	{
		return _delegate_beginCreatingGeometry.invoke( vlObj );
	}
	void setGeomVertexParams( uint32 geoObj, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride )
	{
		_delegate_setGeomVertexParams.invoke( geoObj, vbo, vbSlot, offset, stride );
	}
	void setGeomIndexParams( uint32 geoObj, uint32 indBuf, RDIIndexFormat format )
	{
		_delegate_setGeomIndexParams.invoke( geoObj, indBuf, format );
	}
	void finishCreatingGeometry( uint32 geoObj )
	{
		_delegate_finishCreatingGeometry.invoke( geoObj );
	}
	void destroyGeometry( uint32& geoObj, bool destroyBindedBuffers = true )
	{
		_delegate_destroyGeometry.invoke( geoObj, destroyBindedBuffers );
	}
	uint32 createVertexBuffer( uint32 size, const void *data )
	{
		return _delegate_createVertexBuffer.invoke( size, data );
	}
	uint32 createIndexBuffer( uint32 size, const void *data ) 
	{ 
		return _delegate_createIndexBuffer.invoke( size, data );
	}
	uint32 createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data )
	{
		return _delegate_createTextureBuffer.invoke( format, bufSize, data );
	}
	uint32 createShaderStorageBuffer( uint32 size, const void *data )
	{
		return _delegate_createShaderStorageBuffer.invoke( size, data );
	}
	void destroyBuffer( uint32& bufObj )
	{ 
		_delegate_destroyBuffer.invoke( bufObj );
	}
	void destroyTextureBuffer( uint32& bufObj )
	{
		_delegate_destroyTextureBuffer.invoke( bufObj );
	}
	void updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data ) 
	{ 
		_delegate_updateBufferData.invoke( geoObj, bufObj, offset, size, data );
	}
	void *mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType )
	{
		return _delegate_mapBuffer.invoke( geoObj, bufObj, offset, size, mapType );
	}
	void unmapBuffer( uint32 geoObj, uint32 bufObj )
	{
		_delegate_unmapBuffer.invoke( geoObj, bufObj );
	}

	uint32 getBufferMem() const 
	{ 
		return _bufferMem;
	}

	// Textures
	uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth ) 
	{ 
		switch ( format )
		{
			case TextureFormats::R8:
				return width * height * depth * 1;
			case TextureFormats::RG8:
				return width * height * depth * 2;
			case TextureFormats::R16F:
			case TextureFormats::RG16F:
			case TextureFormats::R32F:
			case TextureFormats::RG32F:
			case TextureFormats::BGRA8:
				return width * height * depth * 4;
			case TextureFormats::DXT1:
			case TextureFormats::ETC1:
			case TextureFormats::RGB8_ETC2:
				return idivceil(width, 4) * idivceil(height, 4) * depth * 8;
			case TextureFormats::DXT3:
			case TextureFormats::DXT5:
			case TextureFormats::RGBA8_ETC2:
			case TextureFormats::BC6_SF16:
			case TextureFormats::BC6_UF16:
			case TextureFormats::BC7:
			case TextureFormats::ASTC_4x4:
			case TextureFormats::ASTC_5x4:
			case TextureFormats::ASTC_5x5:
			case TextureFormats::ASTC_6x5:
			case TextureFormats::ASTC_6x6:
			case TextureFormats::ASTC_8x5:
			case TextureFormats::ASTC_8x6:
			case TextureFormats::ASTC_8x8:
			case TextureFormats::ASTC_10x5:
			case TextureFormats::ASTC_10x6:
			case TextureFormats::ASTC_10x8:
			case TextureFormats::ASTC_10x10:
			case TextureFormats::ASTC_12x10:
			case TextureFormats::ASTC_12x12:
				return idivceil(width, 4) * idivceil(height, 4) * depth * 16;
			case TextureFormats::RGBA16F:
				return width * height * depth * 8;
			case TextureFormats::RGBA32F:
				return width * height * depth * 16;
			case TextureFormats::RGBA32UI:
				return width * height * depth * 16;
			default:
				return 0;
		}
	}
	uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth, int maxMipLevel )
	{
		uint32 size = 0;
		for ( int level = 0; level <= maxMipLevel; ++level ) {
			size += calcTextureSize(format, width, height, depth);
			if ( width > 1 ) width >>= 1;
			if ( height > 1 ) height >>= 1;
			if ( depth > 1 ) depth >>= 1;
		}
		return size;
	}
	uint32 createTexture( TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
	                      int maxMipLevel, bool genMips, bool compress, bool sRGB )
	{
		return _delegate_createTexture.invoke( type, width, height, depth, format, maxMipLevel, genMips, compress, sRGB );
	}
	void generateTextureMipmap( uint32 texObj )
	{
		_delegate_generateTextureMipmap.invoke( texObj );
	}
	void uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
	{ 
		_delegate_uploadTextureData.invoke( texObj, slice, mipLevel, pixels );
	}
	void destroyTexture( uint32& texObj )
	{ 
		_delegate_destroyTexture.invoke( texObj );
	}
	void updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels ) 
	{ 
		_delegate_updateTextureData.invoke( texObj, slice, mipLevel, pixels );
	}
	bool getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer )
	{
		return _delegate_getTextureData.invoke( texObj, slice, mipLevel, buffer );
	}
	uint32 getTextureMem() const 
	{
		return _textureMem; 
	}
	void bindImageToTexture( uint32 texObj, void* eglImage )
	{
		return _delegate_bindImageToTexture.invoke( texObj, eglImage );
	}

	// Shaders
	uint32 createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc, 
						 const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc ) 
	{
		return _delegate_createShader.invoke( vertexShaderSrc, fragmentShaderSrc, geometryShaderSrc, 
											  tessControlShaderSrc, tessEvaluationShaderSrc, computeShaderSrc );
	}
	void destroyShader( uint32& shaderId )
	{
		_delegate_destroyShader.invoke( shaderId );
	}
	void bindShader( uint32 shaderId ) 
	{ 
		_delegate_bindShader.invoke( shaderId );
	}
	std::string getShaderLog() const 
	{
		return _shaderLog; 
	}
	int getShaderConstLoc( uint32 shaderId, const char *name ) 
	{ 
		return _delegate_getShaderConstLoc.invoke( shaderId, name );
	}
	int getShaderSamplerLoc( uint32 shaderId, const char *name )
	{ 
		return _delegate_getShaderSamplerLoc.invoke( shaderId, name );
	}
	int getShaderBufferLoc( uint32 shaderId, const char *name )
	{
		return _delegate_getShaderBufferLoc.invoke( shaderId, name );
	}
	void setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count = 1 ) 
	{
		_delegate_setShaderConst.invoke( loc, type, values, count );
	}
	void setShaderSampler( int loc, uint32 texUnit ) 
	{
		_delegate_setShaderSampler.invoke( loc, texUnit );
	}
	const char *getDefaultVSCode() 
	{ 
		return _delegate_getDefaultVSCode.invoke();
	}
	const char *getDefaultFSCode()  
	{ 
		return _delegate_getDefaultFSCode.invoke();
	}
	void runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim )
	{
		_delegate_runComputeShader.invoke( shaderId, xDim, yDim, zDim );
	}

	// Renderbuffers
	uint32 createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
	                           bool depth, uint32 numColBufs, uint32 samples, uint32 maxMipLevel )
	{
		return _delegate_createRenderBuffer.invoke( width, height, format, depth, numColBufs, samples, maxMipLevel);
	}
    void destroyRenderBuffer( uint32& rbObj )
	{
		_delegate_destroyRenderBuffer.invoke( rbObj );
	}
	uint32 getRenderBufferTex( uint32 rbObj, uint32 bufIndex ) 
	{
		return _delegate_getRenderBufferTex.invoke( rbObj, bufIndex );
	}
	void setRenderBuffer( uint32 rbObj ) 
	{ 
		_delegate_setRenderBuffer.invoke( rbObj );
	}
	bool getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
	                          int *compCount, void *dataBuffer, int bufferSize )
	{
		return _delegate_getRenderBufferData.invoke( rbObj, bufIndex, width, height, compCount, dataBuffer, bufferSize );
	}
	void getRenderBufferDimensions( uint32 rbObj, int *width, int *height )
	{
		_delegate_getRenderBufferDimensions.invoke( rbObj, width, height );
	}

	// Queries
	uint32 createOcclusionQuery() 
	{
		return _delegate_createOcclusionQuery.invoke();
	}
	void destroyQuery( uint32 queryObj ) 
	{
		_delegate_destroyQuery.invoke( queryObj );
	}
	void beginQuery( uint32 queryObj ) 
	{ 
		_delegate_beginQuery.invoke( queryObj );
	}
	void endQuery( uint32 queryObj ) 
	{ 
		_delegate_endQuery.invoke( queryObj );
	}
	uint32 getQueryResult( uint32 queryObj )  
	{
		return _delegate_getQueryResult.invoke( queryObj );
	}

	// Render Device dependent GPU Timer
	GPUTimer *createGPUTimer() 
	{ 
		return _delegate_createGPUTimer.invoke();
	}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------
	
	void setViewport( int x, int y, int width, int height )
		{ _vpX = x; _vpY = y; _vpWidth = width; _vpHeight = height; _pendingMask |= PM_VIEWPORT; }
	void setScissorRect( int x, int y, int width, int height )
		{ _scX = x; _scY = y; _scWidth = width; _scHeight = height; _pendingMask |= PM_SCISSOR; }
	void setGeometry( uint32 geoIndex )
		{ _curGeometryIndex = geoIndex;  _pendingMask |= PM_GEOMETRY; }
	void setTexture( uint32 slot, uint32 texObj, uint16 samplerState, uint16 usage )
		{ ASSERT( slot < 16/*_maxTexSlots*/ ); _texSlots[slot] = RDITexSlot( texObj, samplerState, usage );
	      _pendingMask |= PM_TEXTURES; }
// 	void setTextureBuffer( uint32 bufObj )
// 	{	_curTextureBuf = bufObj; _pendingMask |= PM_TEXTUREBUFFER; }
	void setMemoryBarrier( RDIDrawBarriers barrier )
	{	_memBarriers = barrier; _pendingMask |= PM_BARRIER; }
	void setStorageBuffer( uint8 slot, uint32 bufObj )
	{	_delegate_setStorageBuffer.invoke( slot, bufObj ); }

	// Render states
	void setColorWriteMask( bool enabled )
		{ _newRasterState.renderTargetWriteMask = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getColorWriteMask( bool &enabled ) const
		{ enabled = _newRasterState.renderTargetWriteMask; }
	void setFillMode( RDIFillMode fillMode )
		{ _newRasterState.fillMode = fillMode; _pendingMask |= PM_RENDERSTATES; }
	void getFillMode( RDIFillMode &fillMode ) const
		{ fillMode = (RDIFillMode)_newRasterState.fillMode; }
	void setCullMode( RDICullMode cullMode )
		{ _newRasterState.cullMode = cullMode; _pendingMask |= PM_RENDERSTATES; }
	void getCullMode( RDICullMode &cullMode ) const
		{ cullMode = (RDICullMode)_newRasterState.cullMode; }
	void setScissorTest( bool enabled )
		{ _newRasterState.scissorEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getScissorTest( bool &enabled ) const
		{ enabled = _newRasterState.scissorEnable; }
	void setMulisampling( bool enabled )
		{ _newRasterState.multisampleEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getMulisampling( bool &enabled ) const
		{ enabled = _newRasterState.multisampleEnable; }
	void setAlphaToCoverage( bool enabled )
		{ _newBlendState.alphaToCoverageEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getAlphaToCoverage( bool &enabled ) const
		{ enabled = _newBlendState.alphaToCoverageEnable; }
	void setBlendMode( bool enabled, RDIBlendFunc srcBlendFunc = BS_BLEND_ZERO, RDIBlendFunc destBlendFunc = BS_BLEND_ZERO )
		{ _newBlendState.blendEnable = enabled; _newBlendState.srcBlendFunc = srcBlendFunc;
		  _newBlendState.destBlendFunc = destBlendFunc; _pendingMask |= PM_RENDERSTATES; }
	void getBlendMode( bool &enabled, RDIBlendFunc &srcBlendFunc, RDIBlendFunc &destBlendFunc ) const
		{ enabled = _newBlendState.blendEnable; srcBlendFunc = (RDIBlendFunc)_newBlendState.srcBlendFunc;
		  destBlendFunc = (RDIBlendFunc)_newBlendState.destBlendFunc; }
	void setDepthMask( bool enabled )
		{ _newDepthStencilState.depthWriteMask = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getDepthMask( bool &enabled ) const
		{ enabled = _newDepthStencilState.depthWriteMask; }
	void setDepthTest( bool enabled )
		{ _newDepthStencilState.depthEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getDepthTest( bool &enabled ) const
		{ enabled = _newDepthStencilState.depthEnable; }
	void setDepthFunc( RDIDepthFunc depthFunc )
		{ _newDepthStencilState.depthFunc = depthFunc; _pendingMask |= PM_RENDERSTATES; }
	void getDepthFunc( RDIDepthFunc &depthFunc ) const
		{ depthFunc = (RDIDepthFunc)_newDepthStencilState.depthFunc; }
	void setTessPatchVertices( uint16 verts )
		{ _tessPatchVerts = verts; _pendingMask |= PM_RENDERSTATES; }

	bool commitStates( uint32 filter = 0xFFFFFFFF ) 
	{
		return _delegate_commitStates.invoke( filter );
	}
	void resetStates() 
	{
		_delegate_resetStates.invoke();
	}
	
	// Draw calls and clears
	void clear( uint32 flags, float *colorRGBA = 0x0, float depth = 1.0f )
	{
		_delegate_clear.invoke( flags, colorRGBA, depth );
	}
	void draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
	{
		_delegate_draw.invoke( primType, firstVert, numVerts );
	}
	void drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
	                  uint32 firstVert, uint32 numVerts )
	{ 
		_delegate_drawIndexed.invoke( primType, firstIndex, numIndices, firstVert, numVerts );
	}

// -----------------------------------------------------------------------------
// Getters
// -----------------------------------------------------------------------------

	const DeviceCaps &getCaps() const { return _caps; }

	friend class Renderer;

protected:

	enum RDIPendingMask
	{
		PM_VIEWPORT      = 0x00000001,
// 		PM_INDEXBUF      = 0x00000002,
// 		PM_VERTLAYOUT    = 0x00000004,
		PM_TEXTUREBUFFER = 0x00000004,
		PM_TEXTURES      = 0x00000008,
		PM_SCISSOR       = 0x00000010,
		PM_RENDERSTATES  = 0x00000020,
		PM_GEOMETRY		 = 0x00000040,
		PM_BARRIER		 = 0x00000080,
		PM_COMPUTE		 = 0x00000100
	};

protected:

	DeviceCaps					_caps;

	RDITexSlot					_texSlots[ 16 ];
	// 	std::vector< RDITexSlot >	_texSlots;
	RDIRasterState				_curRasterState, _newRasterState;
	RDIBlendState				_curBlendState, _newBlendState;
	RDIDepthStencilState		_curDepthStencilState, _newDepthStencilState;
	RDIDrawBarriers				_memBarriers;

	std::string					_shaderLog;
	uint32						_depthFormat;
	int							_vpX, _vpY, _vpWidth, _vpHeight;
	int							_scX, _scY, _scWidth, _scHeight;
	int							_fbWidth, _fbHeight;
	uint32						_curRendBuf;
	int							_outputBufferIndex;  // Left and right eye for stereo rendering
	uint32						_textureMem, _bufferMem;

	uint32                      _numVertexLayouts;

	uint32						_prevShaderId, _curShaderId;
	uint32						_pendingMask;
	uint32						_curGeometryIndex;
//	uint32						_curTextureBuf;
 	uint32						_maxTexSlots; // specified in inherited render devices

	uint32						_tessPatchVerts; // number of vertices in patch. Used for tesselation.

	int							_defaultFBO;
	bool                        _defaultFBOMultisampled;

};

}
#endif // _egRendererBase_H_
