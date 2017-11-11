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

// Using the SFINAE (Substitution Failure Is Not An Error) technique,
// the following macro creates a template class with typename T and a
// static boolean member "value" that is set to true if the specified
// member function exists in the T class.
// This macro was created based on information that was retrieved from
// the following URLs:
// https://groups.google.com/forum/?fromgroups#!topic/comp.lang.c++/DAq3H8Ph_1k
// http://objectmix.com/c/779528-call-member-function-only-if-exists-vc-9-a.html

#define CreateMemberFunctionChecker( FNNAME )                            \
  template<typename T> struct has_member_##FNNAME;                       \
                                                                         \
  template<typename R, typename C> class has_member_##FNNAME<R C::*> {   \
     private:                                                            \
        template<R C::*> struct helper;                                  \
        template<typename T> static char check(helper<&T::FNNAME>*);     \
        template<typename T> static char (& check(...))[2];              \
     public:                                                             \
        static const bool value = (sizeof(check<C>(0)) == sizeof(char)); \
		  }

// This corresponding macro is used to check the existence of the
// interface function in the derived class.
#define CheckMemberFunction( FNNAME, FNPROTOTYPE ) {                     \
              assert( has_member_##FNNAME<FNPROTOTYPE>::value ); }

//
// Current RenderDeviceInterface and GPUTimer implementation is based on Simulated C++ Interface Template Pattern from
// http://www.codeproject.com/Articles/603818/Cplusplus-Runtime-Polymorphism-without-Virtual-Fun
//
// Tips:
// 1) There are 5 sections to consider: 
// - function checking, that checks the availability of the function in inherited class
// - typedefs that create a representation of the function
// - pointers to functions of the inherited class that will be called by the base class
// - invoker functions that are called by the base class
// - template <typename T> void initFunctions(): a function that should be called by any inherited class that will create pointers to class member functions
// 2) DO NOT FORGET to add empty brackets in CheckMemberFunction macro for functions that have no parameters. Something like this: CheckMemberFunction( reset, void( T::* )( ) );


// =================================================================================================
// GPUTimer
// =================================================================================================

class GPUTimer
{
private:
	// -----------------------------------------------------------------------------
	// Template functions
	// -----------------------------------------------------------------------------

	// Function checking
	CreateMemberFunctionChecker( beginQuery );
	CreateMemberFunctionChecker( endQuery );
	CreateMemberFunctionChecker( updateResults );
	CreateMemberFunctionChecker( reset );
	CreateMemberFunctionChecker( getTimeMS );

	// Typedef's
	typedef void( *PFN_GPUTIMER_BEGINQUERY )( void* const, uint32 frameID );
	typedef void( *PFN_GPUTIMER_ENDQUERY )( void* const );
	typedef bool( *PFN_GPUTIMER_UPDATERESULTS )( void* const );
	typedef void( *PFN_GPUTIMER_RESET )( void* const );

	// pointers to functions
	PFN_GPUTIMER_BEGINQUERY					_pfn_GPUTimer_BeginQuery;
	PFN_GPUTIMER_ENDQUERY					_pfn_GPUTimer_EndQuery;
	PFN_GPUTIMER_UPDATERESULTS				_pfn_GPUTimer_UpdateResults;
	PFN_GPUTIMER_RESET						_pfn_GPUTimer_Reset;

	// Invoker functions
	template<typename T>
	inline static void              beginQuery_Invoker( void* const pObj, uint32 frameID )
	{
		static_cast< T* >( pObj )->beginQuery( frameID );
	}

	template<typename T>
	inline static void              endQuery_Invoker( void* const pObj )
	{
		static_cast< T* >( pObj )->endQuery();
	}

	template<typename T>
	inline static bool              updateResults_Invoker( void* const pObj )
	{
		return static_cast< T* >( pObj )->updateResults();
	}

	template<typename T>
	inline static void              reset_Invoker( void* const pObj )
	{
		static_cast< T* >( pObj )->reset();
	}

protected:

	GPUTimer()
	{

	}

	template <typename T>
	void initFunctions()
	{
		// check for implementation
		CheckMemberFunction( beginQuery, void( T::* )( uint32 ) );
		CheckMemberFunction( endQuery, void( T::* )( ) );
		CheckMemberFunction( updateResults, bool( T::* )( ) );
		CheckMemberFunction( reset, void( T::* )( ) );

		// create pointer to implementation
		_pfn_GPUTimer_BeginQuery = ( PFN_GPUTIMER_BEGINQUERY ) &beginQuery_Invoker < T > ;
		_pfn_GPUTimer_EndQuery = ( PFN_GPUTIMER_ENDQUERY ) &endQuery_Invoker < T > ;
		_pfn_GPUTimer_UpdateResults = ( PFN_GPUTIMER_UPDATERESULTS ) &updateResults_Invoker < T > ;
		_pfn_GPUTimer_Reset = ( PFN_GPUTIMER_RESET ) &reset_Invoker < T > ;
	}

public:
	
	// -----------------------------------------------------------------------------
	// Interface
	// -----------------------------------------------------------------------------
	virtual ~GPUTimer()
	{

	}
	
	void beginQuery( uint32 frameID )
	{
		( *_pfn_GPUTimer_BeginQuery ) ( this, frameID );
	}
	void endQuery()
	{
		( *_pfn_GPUTimer_EndQuery ) ( this );
	}
	bool updateResults()
	{
		return ( *_pfn_GPUTimer_UpdateResults ) ( this );
	}
	
	void reset()
	{
		( *_pfn_GPUTimer_Reset ) ( this );
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
		BGRA8,
		DXT1,
		DXT3,
		DXT5,
		RGBA16F,
		RGBA32F,
		DEPTH,
		R32,
		RG32
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
	CONST_FLOAT33
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
	PRIM_LINES,
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
// Template functions
// -----------------------------------------------------------------------------
private:
	// Function checking
	CreateMemberFunctionChecker( init );
	CreateMemberFunctionChecker( initStates );
	CreateMemberFunctionChecker( registerVertexLayout );
	CreateMemberFunctionChecker( beginRendering );

	CreateMemberFunctionChecker( beginCreatingGeometry );
	CreateMemberFunctionChecker( setGeomVertexParams );
	CreateMemberFunctionChecker( setGeomIndexParams );
	CreateMemberFunctionChecker( finishCreatingGeometry );
	CreateMemberFunctionChecker( destroyGeometry );
	CreateMemberFunctionChecker( createVertexBuffer );
	CreateMemberFunctionChecker( createIndexBuffer );
	CreateMemberFunctionChecker( createTextureBuffer );
	CreateMemberFunctionChecker( createShaderStorageBuffer );
	CreateMemberFunctionChecker( destroyBuffer );
	CreateMemberFunctionChecker( destroyTextureBuffer );
	CreateMemberFunctionChecker( updateBufferData );
	CreateMemberFunctionChecker( mapBuffer );
	CreateMemberFunctionChecker( unmapBuffer );

	CreateMemberFunctionChecker( calcTextureSize );
	CreateMemberFunctionChecker( createTexture );
	CreateMemberFunctionChecker( uploadTextureData );
	CreateMemberFunctionChecker( destroyTexture );
	CreateMemberFunctionChecker( updateTextureData );
	CreateMemberFunctionChecker( getTextureData );

	CreateMemberFunctionChecker( createShader );
	CreateMemberFunctionChecker( destroyShader );
	CreateMemberFunctionChecker( bindShader );
	CreateMemberFunctionChecker( getShaderConstLoc );
	CreateMemberFunctionChecker( getShaderSamplerLoc );
	CreateMemberFunctionChecker( getShaderBufferLoc );
	CreateMemberFunctionChecker( runComputeShader );
	CreateMemberFunctionChecker( setShaderConst );
	CreateMemberFunctionChecker( setShaderSampler );
	CreateMemberFunctionChecker( getDefaultVSCode );
	CreateMemberFunctionChecker( getDefaultFSCode );

	CreateMemberFunctionChecker( createRenderBuffer );
	CreateMemberFunctionChecker( destroyRenderBuffer );
	CreateMemberFunctionChecker( getRenderBufferTex );
	CreateMemberFunctionChecker( setRenderBuffer );
	CreateMemberFunctionChecker( getRenderBufferData );
	CreateMemberFunctionChecker( getRenderBufferDimensions );

	CreateMemberFunctionChecker( createOcclusionQuery );
	CreateMemberFunctionChecker( destroyQuery );
	CreateMemberFunctionChecker( beginQuery );
	CreateMemberFunctionChecker( endQuery );
	CreateMemberFunctionChecker( getQueryResult );

	CreateMemberFunctionChecker( createGPUTimer );

	CreateMemberFunctionChecker( commitStates );
	CreateMemberFunctionChecker( resetStates );
	CreateMemberFunctionChecker( clear );
	CreateMemberFunctionChecker( draw );
	CreateMemberFunctionChecker( drawIndexed );

	CreateMemberFunctionChecker( setStorageBuffer );

	// Typedefs
	typedef bool( *PFN_INIT )( void* const );
	typedef void( *PFN_INITSTATES )( void* const );
	typedef uint32( *PFN_REGISTERVERTEXLAYOUT )( void* const, uint32 numAttribs, VertexLayoutAttrib *attribs );
	typedef void( *PFN_BEGINRENDERING )( void* const );

	typedef uint32( *PFN_BEGINCREATINGGEOMETRY )( void* const, uint32 vlObj );
	typedef void( *PFN_FINISHCREATINGGEOMETRY )( void* const, uint32 geoIndex );
    typedef void( *PFN_DESTROYGEOMETRY )( void* const, uint32& geoIndex, bool destroyBindedBuffers );
	typedef void( *PFN_SETGEOMVERTEXPARAMS )( void* const, uint32 geoIndex, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride );
	typedef void( *PFN_SETGEOMINDEXPARAMS )( void* const, uint32 geoIndex, uint32 idxBuf, RDIIndexFormat format );

	typedef uint32( *PFN_CREATEVERTEXBUFFER )( void* const, uint32 size, const void *data );
	typedef uint32( *PFN_CREATEINDEXBUFFER )( void* const, uint32 size, const void *data );
	typedef uint32( *PFN_CREATETEXTUREBUFFER )( void* const, TextureFormats::List format, uint32 size, const void *data );
	typedef uint32( *PFN_CREATESHADERSTORAGEBUFFER )( void* const, uint32 size, const void *data );
    typedef void( *PFN_DESTROYBUFFER )( void* const, uint32& bufObj );
    typedef void( *PFN_DESTROYTEXTUREBUFFER )( void* const, uint32& bufObj );
	typedef void( *PFN_UPDATEBUFFERDATA )( void* const, uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data );
	typedef void*( *PFN_MAPBUFFER )( void* const, uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType );
	typedef void( *PFN_UNMAPBUFFER )( void* const, uint32 geoObj, uint32 bufObj );

	typedef uint32( *PFN_CALCTEXTURESIZE )( void* const, TextureFormats::List format, int width, int height, int depth );
	typedef uint32( *PFN_CREATETEXTURE )( void* const, TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
										  bool hasMips, bool genMips, bool compress, bool sRGB );
	typedef void( *PFN_UPLOADTEXTUREDATA )( void* const, uint32 texObj, int slice, int mipLevel, const void *pixels );
    typedef void( *PFN_DESTROYTEXTURE )( void* const, uint32& texObj );
	typedef void( *PFN_UPDATETEXTUREDATA )( void* const, uint32 texObj, int slice, int mipLevel, const void *pixels );
	typedef bool( *PFN_GETTEXTUREDATA )( void* const, uint32 texObj, int slice, int mipLevel, void *buffer );
    typedef void( *PFN_BINDIMAGETOTEXTURE )( void* const, uint32 texObj, void* eglImage );
	
	typedef uint32( *PFN_CREATESHADER )( void* const, const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
										 const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc );
    typedef void( *PFN_DESTROYSHADER )( void* const, uint32& shaderId );
	typedef void( *PFN_BINDSHADER )( void* const, uint32 shaderId );
	typedef int( *PFN_GETSHADERCONSTLOC )( void* const, uint32 shaderId, const char *name );
	typedef int( *PFN_GETSHADERSAMPLERLOC )( void* const, uint32 shaderId, const char *name );
	typedef int( *PFN_GETSHADERBUFFERLOC )( void* const, uint32 shaderId, const char *name );
	typedef void( *PFN_RUNCOMPUTESHADER )( void* const, uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim );
	typedef void( *PFN_SETSHADERCONST )( void* const, int loc, RDIShaderConstType type, void *values, uint32 count );
	typedef void( *PFN_SETSHADERSAMPLER )( void* const, int loc, uint32 texUnit );
	typedef const char*( *PFN_GETDEFAULTVSCODE )( void* const );
	typedef const char*( *PFN_GETDEFAULTFSCODE )( void* const );
	
	typedef uint32( *PFN_CREATERENDERBUFFER )( void* const, uint32 width, uint32 height, TextureFormats::List format,
											   bool depth, uint32 numColBufs, uint32 samples );
    typedef void( *PFN_DESTROYRENDERBUFFER )( void* const, uint32& rbObj );
	typedef uint32( *PFN_GETRENDERBUFFERTEX )( void* const, uint32 rbObj, uint32 bufIndex );
	typedef void( *PFN_SETRENDERBUFFER )( void* const, uint32 rbObj );
	typedef bool( *PFN_GETRENDERBUFFERDATA )( void* const, uint32 rbObj, int bufIndex, int *width, int *height,
											  int *compCount, void *dataBuffer, int bufferSize );
	typedef bool( *PFN_GETRENDERBUFFERDIMENSIONS )( void* const, uint32 rbObj, int *width, int *height );

	typedef uint32( *PFN_CREATEOCCLUSIONQUERY )( void *const );
	typedef void( *PFN_DESTROYQUERY )( void* const, uint32 queryObj );
	typedef void( *PFN_BEGINQUERY )( void* const, uint32 queryObj );
	typedef void( *PFN_ENDQUERY )( void* const, uint32 queryObj );
	typedef uint32( *PFN_GETQUERYRESULT )( void* const, uint32 queryObj );

	typedef GPUTimer *( *PFN_CREATEGPUTIMER )( void* const );

	typedef bool( *PFN_COMMITSTATES )( void* const, uint32 filter );
	typedef void( *PFN_RESETSTATES )( void* const );

	typedef void( *PFN_CLEAR )( void* const, uint32 flags, float *colorRGBA, float depth );
	typedef void( *PFN_DRAW )( void* const, RDIPrimType primType, uint32 firstVert, uint32 numVerts );
	typedef void( *PFN_DRAWINDEXED )( void* const, RDIPrimType primType, uint32 firstIndex, uint32 numIndices, uint32 firstVert, uint32 numVerts );

	typedef bool( *PFN_SETSTORAGEBUFFER )( void* const, uint8 slot, uint32 bufObj  );

	// pointers to functions
	// general
	PFN_INIT					_pfnInit;
	PFN_INITSTATES				_pfnInitStates;
	PFN_REGISTERVERTEXLAYOUT	_pfnRegisterVertexLayout;
	PFN_BEGINRENDERING			_pfnBeginRendering;

	// geometry
	PFN_BEGINCREATINGGEOMETRY	_pfnBeginCreatingGeometry;
	PFN_SETGEOMVERTEXPARAMS		_pfnSetGeomVertexParams;
	PFN_SETGEOMINDEXPARAMS		_pfnSetGeomIndexParams;
	PFN_FINISHCREATINGGEOMETRY  _pfnFinishCreatingGeometry;
	PFN_DESTROYGEOMETRY			_pfnDestroyGeometry;
	PFN_CREATEVERTEXBUFFER		_pfnCreateVertexBuffer;
	PFN_CREATEINDEXBUFFER		_pfnCreateIndexBuffer;
	PFN_CREATETEXTUREBUFFER		_pfnCreateTextureBuffer;
	PFN_CREATESHADERSTORAGEBUFFER _pfnCreateShaderStorageBuffer;
	PFN_DESTROYBUFFER			_pfnDestroyBuffer;
	PFN_DESTROYTEXTUREBUFFER	_pfnDestroyTextureBuffer;
	PFN_UPDATEBUFFERDATA		_pfnUpdateBufferData;
	PFN_MAPBUFFER				_pfnMapBuffer;
	PFN_UNMAPBUFFER				_pfnUnmapBuffer;

	// textures
	PFN_CALCTEXTURESIZE			_pfnCalcTextureSize;
	PFN_CREATETEXTURE			_pfnCreateTexture;
	PFN_UPLOADTEXTUREDATA		_pfnUploadTextureData;
	PFN_DESTROYTEXTURE			_pfnDestroyTexture;
	PFN_UPDATETEXTUREDATA		_pfnUpdateTextureData;
	PFN_GETTEXTUREDATA			_pfnGetTextureData;
    PFN_BINDIMAGETOTEXTURE      _pfnBindImageToTexture;

	// shaders
	PFN_CREATESHADER			_pfnCreateShader;
	PFN_DESTROYSHADER			_pfnDestroyShader;
	PFN_BINDSHADER				_pfnBindShader;
	PFN_GETSHADERCONSTLOC		_pfnGetShaderConstLoc;
	PFN_GETSHADERSAMPLERLOC		_pfnGetShaderSamplerLoc;
	PFN_GETSHADERBUFFERLOC		_pfnGetShaderBufferLoc;
	PFN_RUNCOMPUTESHADER		_pfnRunComputeShader;
	PFN_SETSHADERCONST			_pfnSetShaderConst;
	PFN_SETSHADERSAMPLER		_pfnSetShaderSampler;
	PFN_GETDEFAULTVSCODE		_pfnGetDefaultVSCode;
	PFN_GETDEFAULTFSCODE		_pfnGetDefaultFSCode;

	// render bufs
	PFN_CREATERENDERBUFFER		_pfnCreateRenderBuffer;
	PFN_DESTROYRENDERBUFFER		_pfnDestroyRenderBuffer;
	PFN_GETRENDERBUFFERTEX		_pfnGetRenderBufferTex;
	PFN_SETRENDERBUFFER			_pfnSetRenderBuffer;
	PFN_GETRENDERBUFFERDATA		_pfnGetRenderBufferData;
	PFN_GETRENDERBUFFERDIMENSIONS _pfnGetRenderBufferDimensions;

	// queries
	PFN_CREATEOCCLUSIONQUERY	_pfnCreateOcclusionQuery;
	PFN_DESTROYQUERY			_pfnDestroyQuery;
	PFN_BEGINQUERY				_pfnBeginQuery;
	PFN_ENDQUERY				_pfnEndQuery;
	PFN_GETQUERYRESULT			_pfnGetQueryResult;

	PFN_CREATEGPUTIMER			_pfnCreateGPUTimer;

	// states handling
	PFN_COMMITSTATES			_pfnCommitStates;
	PFN_RESETSTATES				_pfnResetStates;
	PFN_CLEAR					_pfnClear;

	// drawing
	PFN_DRAW					_pfnDraw;
	PFN_DRAWINDEXED				_pfnDrawIndexed;
	
	// commands
	PFN_SETSTORAGEBUFFER		_pfnSetStorageBuffer;

	// invoker functions
	// main funcs
	template<typename T>
	inline static bool              init_Invoker( void* const pObj )
	{ 
		return static_cast< T* >( pObj )->init();
	}

	template<typename T>
	inline static void              initStates_Invoker( void* const pObj )
	{
		static_cast< T* >( pObj )->initStates();
	}

	template<typename T>
	inline static uint32            registerVertexLayout_Invoker( void* const pObj, uint32 numAttribs, VertexLayoutAttrib *attribs )
	{ 
		return static_cast< T* >( pObj )->registerVertexLayout( numAttribs, attribs );
	}
	
	template<typename T>
	inline static void              beginRendering_Invoker( void* const pObj )
	{ 
		static_cast< T* >( pObj )->beginRendering();
	}


	// buffers
	template<typename T>
	inline static uint32            createVertexBuffer_Invoker( void* const pObj, uint32 size, const void *data )
	{ 
		return static_cast< T* >( pObj )->createVertexBuffer( size, data ); 
	}

	template<typename T>
	inline static uint32            createIndexBuffer_Invoker( void* const pObj, uint32 size, const void *data )
	{ 
		return static_cast< T* >( pObj )->createIndexBuffer( size, data ); 
	}

	template<typename T>
	inline static uint32            createTextureBuffer_Invoker( void* const pObj, TextureFormats::List format, uint32 bufSize, const void *data )
	{
		return static_cast< T* >( pObj )->createTextureBuffer( format, bufSize, data );
	}

	template<typename T>
	inline static uint32            createShaderStorageBuffer_Invoker( void* const pObj, uint32 size, const void *data )
	{
		return static_cast< T* >( pObj )->createShaderStorageBuffer( size, data );
	}

	template<typename T>
	inline static void				 setGeomVertexParams_Invoker( void* const pObj, uint32 geoIndex, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride )
	{
		static_cast< T* >( pObj )->setGeomVertexParams( geoIndex, vbo, vbSlot, offset, stride );
	}

	template<typename T>
	inline static void				 setGeomIndexParams_Invoker( void* const pObj, uint32 geoIndex, uint32 idxBuf, RDIIndexFormat format )
	{
		static_cast< T* >( pObj )->setGeomIndexParams( geoIndex, idxBuf, format );
	}

	template<typename T>
	inline static uint32            beginCreatingGeometry_Invoker( void* const pObj, uint32 vlObj )
	{
		return static_cast< T* >( pObj )->beginCreatingGeometry( vlObj );
	}

	template<typename T>
	inline static void		         finishCreatingGeometry_Invoker( void* const pObj, uint32 geoIndex )
	{
		static_cast< T* >( pObj )->finishCreatingGeometry( geoIndex );
	}

	template<typename T>
    static void              destroyGeometry_Invoker( void* const pObj, uint32& geoIndex, bool destroyBindedBuffers )
	{
		static_cast< T* >( pObj )->destroyGeometry( geoIndex, destroyBindedBuffers );
	}

	template<typename T>
    static void              destroyBuffer_Invoker( void* const pObj, uint32& bufObj )
	{ 
		static_cast< T* >( pObj )->destroyBuffer( bufObj ); 
	}

	template<typename T>
    static void              destroyTextureBuffer_Invoker( void* const pObj, uint32& bufObj )
	{
		static_cast< T* >( pObj )->destroyTextureBuffer( bufObj );
	}

	template<typename T>
	inline static void              updateBufferData_Invoker( void* const pObj, uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data )
	{
		static_cast< T* >( pObj )->updateBufferData( geoObj, bufObj, offset, size, data );
	}

	template<typename T>
	static void *            mapBuffer_Invoker( void* const pObj, uint32 geoObj, uint32 bufObj, 
												uint32 offset, uint32 size, RDIBufferMappingTypes mapType )
	{
		return static_cast< T* >( pObj )->mapBuffer( geoObj, bufObj, offset, size, mapType );
	}

	template<typename T>
	static void              unmapBuffer_Invoker( void* const pObj, uint32 geoObj, uint32 bufObj )
	{
		static_cast< T* >( pObj )->unmapBuffer( geoObj, bufObj );
	}


	// textures
	template<typename T>
	inline static uint32            calcTextureSize_Invoker( void* const pObj, TextureFormats::List format, int width, int height, int depth )
	{
		return static_cast< T* >( pObj )->calcTextureSize( format, width, height, depth );
	}

	template<typename T>
	inline static uint32            createTexture_Invoker( void* const pObj, TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
													bool hasMips, bool genMips, bool compress, bool sRGB )
	{
		return static_cast< T* >( pObj )->createTexture( type, width, height, depth, format, hasMips, genMips, compress, sRGB );
	}

	template<typename T>
	inline static void              uploadTextureData_Invoker( void* const pObj, uint32 texObj, int slice, int mipLevel, const void *pixels )
	{
		static_cast< T* >( pObj )->uploadTextureData( texObj, slice, mipLevel, pixels );
	}

	template<typename T>
    static void              destroyTexture_Invoker( void* const pObj, uint32& texObj )
	{
		static_cast< T* >( pObj )->destroyTexture( texObj );
	}

	template<typename T>
	inline static void              updateTextureData_Invoker( void* const pObj, uint32 texObj, int slice, int mipLevel, const void *pixels )
	{
		static_cast< T* >( pObj )->updateTextureData( texObj, slice, mipLevel, pixels );
	}

	template<typename T>
	inline static bool              getTextureData_Invoker( void* const pObj, uint32 texObj, int slice, int mipLevel, void *buffer )
	{
		return static_cast< T* >( pObj )->getTextureData( texObj, slice, mipLevel, buffer );
	}

    template<typename T>
    static void              bindImageToTexture_Invoker( void* const pObj, uint32 texObj, void* eglImage )
    {
        static_cast< T* >( pObj )->bindImageToTexture( texObj, eglImage );
    }

	// shaders
	template<typename T>
	inline static uint32            createShader_Invoker( void* const pObj, const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
												   const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc )
	{
		return static_cast< T* >( pObj )->createShader( vertexShaderSrc, fragmentShaderSrc, geometryShaderSrc, tessControlShaderSrc, tessEvaluationShaderSrc, computeShaderSrc );
	}

	template<typename T>
    static void				 destroyShader_Invoker( void* const pObj, uint32& shaderId )
	{
		static_cast< T* >( pObj )->destroyShader( shaderId );
	}

	template<typename T>
	inline static void				bindShader_Invoker( void* const pObj, uint32 shaderId )
	{
		static_cast< T* >( pObj )->bindShader( shaderId );
	}

	template<typename T>
	inline static int				getShaderConstLoc_Invoker( void* const pObj, uint32 shaderId, const char *name )
	{
		return static_cast< T* >( pObj )->getShaderConstLoc( shaderId, name );
	}

	template<typename T>
	inline static int				getShaderSamplerLoc_Invoker( void* const pObj, uint32 shaderId, const char *name )
	{
		return static_cast< T* >( pObj )->getShaderSamplerLoc( shaderId, name );
	}

	template<typename T>
	inline static int				getShaderBufferLoc_Invoker( void* const pObj, uint32 shaderId, const char *name )
	{
		return static_cast< T* >( pObj )->getShaderBufferLoc( shaderId, name );
	}

	template<typename T>
	inline static void				runComputeShader_Invoker( void* const pObj, uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim )
	{
		static_cast< T* >( pObj )->runComputeShader( shaderId, xDim, yDim, zDim );
	}

	template<typename T>
	inline static void				setShaderConst_Invoker( void* const pObj, int loc, RDIShaderConstType type, void *values, uint32 count )
	{
		static_cast< T* >( pObj )->setShaderConst( loc, type, values, count );
	}

	template<typename T>
	inline static void				setShaderSampler_Invoker( void* const pObj, int loc, uint32 texUnit )
	{
		static_cast< T* >( pObj )->setShaderSampler( loc, texUnit );
	}

	template<typename T>
	inline static const char *		getDefaultVSCode_Invoker( void* const pObj )
	{
		return static_cast< T* >( pObj )->getDefaultVSCode();
	}

	template<typename T>
	inline static const char *		getDefaultFSCode_Invoker( void* const pObj )
	{
		return static_cast< T* >( pObj )->getDefaultFSCode();
	}
	

	// Render bufs
	template<typename T>
	inline static uint32            createRenderBuffer_Invoker( void* const pObj, uint32 width, uint32 height, TextureFormats::List format,
														 bool depth, uint32 numColBufs, uint32 samples )
	{
		return static_cast< T* >( pObj )->createRenderBuffer( width, height, format, depth, numColBufs, samples );
	}

	template<typename T>
    static void				 destroyRenderBuffer_Invoker( void* const pObj, uint32& rbObj )
	{
		static_cast< T* >( pObj )->destroyRenderBuffer( rbObj );
	}

	template<typename T>
	inline static uint32            getRenderBufferTex_Invoker( void* const pObj, uint32 rbObj, uint32 bufIndex )
	{
		return static_cast< T* >( pObj )->getRenderBufferTex( rbObj, bufIndex );
	}

	template<typename T>
	inline static void				 setRenderBuffer_Invoker( void* const pObj, uint32 rbObj )
	{
		static_cast< T* >( pObj )->setRenderBuffer( rbObj );
	}
	
	template<typename T>
	inline static bool	             getRenderBufferData_Invoker( void* const pObj, uint32 rbObj, int bufIndex, int *width, int *height,
														  int *compCount, void *dataBuffer, int bufferSize )
	{
		return static_cast< T* >( pObj )->getRenderBufferData( rbObj, bufIndex, width, height, compCount, dataBuffer, bufferSize );
	}
	
	template<typename T>
	inline static void	             getRenderBufferDimensions_Invoker( void* const pObj, uint32 rbObj, int *width, int *height )
	{
		static_cast< T* >( pObj )->getRenderBufferDimensions( rbObj, width, height );
	}


	// Queries
	template<typename T>
	inline static uint32            createOcclusionQuery_Invoker( void* const pObj )
	{
		return static_cast< T* >( pObj )->createOcclusionQuery();
	}

	template<typename T>
	inline static void				 destroyQuery_Invoker( void* const pObj, uint32 queryObj )
	{
		static_cast< T* >( pObj )->destroyQuery( queryObj );
	}

	template<typename T>
	inline static void				 beginQuery_Invoker( void* const pObj, uint32 queryObj )
	{
		static_cast< T* >( pObj )->beginQuery( queryObj );
	}

	template<typename T>
	inline static void				 endQuery_Invoker( void* const pObj, uint32 queryObj )
	{
		static_cast< T* >( pObj )->endQuery( queryObj );
	}

	template<typename T>
	inline static uint32			 getQueryResult_Invoker( void* const pObj, uint32 queryObj )
	{
		return static_cast< T* >( pObj )->getQueryResult( queryObj );
	}

	template<typename T>
	inline static GPUTimer *		 createGPUTimer_Invoker( void* const pObj )
	{
		return static_cast< T* >( pObj )->createGPUTimer();
	}


	// drawing
	template<typename T>
	inline static bool	             commitStates_Invoker( void* const pObj, uint32 filter )
	{
		return static_cast< T* >( pObj )->commitStates( filter );
	}

	template<typename T>
	inline static void				 resetStates_Invoker( void* const pObj )
	{
		static_cast< T* >( pObj )->resetStates();
	}

	template<typename T>
	inline static void				 clear_Invoker( void* const pObj, uint32 flags, float *colorRGBA, float depth )
	{
		static_cast< T* >( pObj )->clear( flags, colorRGBA, depth );
	}

	template<typename T>
	inline static void				 draw_Invoker( void* const pObj, RDIPrimType primType, uint32 firstVert, uint32 numVerts )
	{
		static_cast< T* >( pObj )->draw( primType, firstVert, numVerts );
	}

	template<typename T>
	inline static void				 drawIndexed_Invoker( void* const pObj, RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
												  uint32 firstVert, uint32 numVerts )
	{
		static_cast< T* >( pObj )->drawIndexed( primType, firstIndex, numIndices, firstVert, numVerts );
	}

	template<typename T>
	inline static void				 setStorageBuffer_Invoker( void* const pObj, uint8 slot, uint32 bufObj )
	{
		static_cast< T* >( pObj )->setStorageBuffer( slot, bufObj );
	}

protected:

	template< typename T > void initRDIFunctions()
	{
		// check for implementation
		CheckMemberFunction( initStates, void( T::* )() );
		CheckMemberFunction( init, bool( T::* )() );
		CheckMemberFunction( registerVertexLayout, uint32( T::* )( uint32, VertexLayoutAttrib * ) );
		CheckMemberFunction( beginRendering, void( T::* )() );

		CheckMemberFunction( createVertexBuffer, uint32( T::* )( uint32, const void* ) );
		CheckMemberFunction( createIndexBuffer, uint32( T::* )( uint32, const void* ) );
		CheckMemberFunction( createTextureBuffer, uint32( T::* )( TextureFormats::List, uint32, const void * ) );
		CheckMemberFunction( createShaderStorageBuffer, uint32( T::* )( uint32, const void* ) );
		CheckMemberFunction( beginCreatingGeometry, uint32( T::* )( uint32 ) );
		CheckMemberFunction( setGeomVertexParams, void( T::* )( uint32, uint32, uint32, uint32, uint32 ) );
		CheckMemberFunction( setGeomIndexParams, void( T::* )( uint32, uint32, RDIIndexFormat ) );
		CheckMemberFunction( finishCreatingGeometry, void( T::* )( uint32 ) );
        CheckMemberFunction( destroyGeometry, void( T::* )( uint32&, bool ) );
        CheckMemberFunction( destroyBuffer, void( T::* )( uint32& ) );
        CheckMemberFunction( destroyTextureBuffer, void( T::* )( uint32& ) );
		CheckMemberFunction( updateBufferData, void( T::* )( uint32, uint32, uint32, uint32, void * ) );
		CheckMemberFunction( mapBuffer, void*( T::* )( uint32, uint32, uint32, uint32, RDIBufferMappingTypes ) );
		CheckMemberFunction( unmapBuffer, void( T::* )( uint32, uint32 ) );

		CheckMemberFunction( calcTextureSize, uint32( T::* )( TextureFormats::List, int, int, int ) );
		CheckMemberFunction( createTexture, uint32( T::* )( TextureTypes::List, int, int, int, TextureFormats::List, bool, bool, bool, bool ) );
		CheckMemberFunction( uploadTextureData, void( T::* )( uint32, int, int, const void * ) );
        CheckMemberFunction( destroyTexture, void( T::* )( uint32& ) );
		CheckMemberFunction( updateTextureData, void( T::* )( uint32, int, int, const void * ) );
		CheckMemberFunction( getTextureData, bool( T::* )( uint32, int, int, void * ) );
	
		CheckMemberFunction( createShader, uint32( T::* )( const char *, const char *, const char *, const char *, const char *, const char * ) );
        CheckMemberFunction( destroyShader, void( T::* )( uint32& ) );
		CheckMemberFunction( bindShader, void( T::* )( uint32 ) );
		CheckMemberFunction( getShaderConstLoc, int( T::* )( uint32, const char * ) );
		CheckMemberFunction( getShaderSamplerLoc, int( T::* )( uint32, const char * ) );
		CheckMemberFunction( getShaderBufferLoc, int( T::* )( uint32, const char * ) );
		CheckMemberFunction( runComputeShader, void( T::* )( uint32, uint32, uint32, uint32 ) );
		CheckMemberFunction( setShaderConst, void( T::* )( int, RDIShaderConstType, void *, uint32 ) );
		CheckMemberFunction( setShaderSampler, void( T::* )( int, uint32 ) );
		CheckMemberFunction( getDefaultVSCode, const char *( T::* )() );
		CheckMemberFunction( getDefaultFSCode, const char *( T::* )() );

		CheckMemberFunction( createRenderBuffer, uint32( T::* )( uint32, uint32, TextureFormats::List, bool, uint32, uint32 ) );
        CheckMemberFunction( destroyRenderBuffer, void( T::* )( uint32& ) );
		CheckMemberFunction( getRenderBufferTex, uint32( T::* )( uint32, uint32 ) );
		CheckMemberFunction( setRenderBuffer, void( T::* )( uint32 ) );
		CheckMemberFunction( getRenderBufferData, bool( T::* )( uint32, int, int *, int *, int *, void *, int ) );
		CheckMemberFunction( getRenderBufferDimensions, void( T::* )( uint32, int *, int * ) );

		CheckMemberFunction( createOcclusionQuery, uint32( T::* )() );
		CheckMemberFunction( destroyQuery, void( T::* )( uint32 ) );
		CheckMemberFunction( beginQuery, void( T::* )( uint32 ) );
		CheckMemberFunction( endQuery, void( T::* )( uint32 ) );
		CheckMemberFunction( getQueryResult, uint32( T::* )( uint32 ) );

		CheckMemberFunction( createGPUTimer, GPUTimer *( T::* )() );

		CheckMemberFunction( commitStates, bool( T::* )( uint32 ) );
		CheckMemberFunction( resetStates, void( T::* )() );
		CheckMemberFunction( clear, void( T::* )( uint32, float *, float ) );
		CheckMemberFunction( draw, void( T::* )( RDIPrimType, uint32, uint32 ) );
		CheckMemberFunction( drawIndexed, void( T::* )( RDIPrimType, uint32, uint32, uint32, uint32 ) );

		CheckMemberFunction( setStorageBuffer, void( T::* )( uint8, uint32 ) );

		// create pointer to implementation
		_pfnInit = ( PFN_INIT ) &init_Invoker< T > ;
		_pfnInitStates = ( PFN_INITSTATES ) &initStates_Invoker< T >;
		_pfnRegisterVertexLayout = ( PFN_REGISTERVERTEXLAYOUT ) &registerVertexLayout_Invoker< T >;
		_pfnBeginRendering = ( PFN_BEGINRENDERING ) &beginRendering_Invoker< T >;

		_pfnCreateVertexBuffer = ( PFN_CREATEVERTEXBUFFER ) &createVertexBuffer_Invoker < T >;
		_pfnCreateIndexBuffer = ( PFN_CREATEINDEXBUFFER ) &createIndexBuffer_Invoker < T >;
		_pfnCreateTextureBuffer = ( PFN_CREATETEXTUREBUFFER ) &createTextureBuffer_Invoker < T >;
		_pfnCreateShaderStorageBuffer = ( PFN_CREATESHADERSTORAGEBUFFER ) &createShaderStorageBuffer_Invoker < T >;
		_pfnBeginCreatingGeometry = ( PFN_BEGINCREATINGGEOMETRY ) &beginCreatingGeometry_Invoker < T > ;
		_pfnSetGeomVertexParams = ( PFN_SETGEOMVERTEXPARAMS ) &setGeomVertexParams_Invoker < T > ;
		_pfnSetGeomIndexParams = ( PFN_SETGEOMINDEXPARAMS ) &setGeomIndexParams_Invoker < T > ;
		_pfnFinishCreatingGeometry = ( PFN_FINISHCREATINGGEOMETRY ) &finishCreatingGeometry_Invoker < T > ;
		_pfnDestroyGeometry = ( PFN_DESTROYGEOMETRY ) &destroyGeometry_Invoker < T > ;
		_pfnDestroyBuffer = ( PFN_DESTROYBUFFER ) &destroyBuffer_Invoker < T >;
		_pfnDestroyTextureBuffer = ( PFN_DESTROYTEXTUREBUFFER ) &destroyTextureBuffer_Invoker < T > ;
		_pfnUpdateBufferData = ( PFN_UPDATEBUFFERDATA ) &updateBufferData_Invoker < T >;
		_pfnMapBuffer = ( PFN_MAPBUFFER ) &mapBuffer_Invoker < T >;
		_pfnUnmapBuffer = ( PFN_UNMAPBUFFER ) &unmapBuffer_Invoker < T >;

		_pfnCalcTextureSize = ( PFN_CALCTEXTURESIZE ) &calcTextureSize_Invoker < T >;
		_pfnCreateTexture = ( PFN_CREATETEXTURE ) &createTexture_Invoker < T >;
		_pfnUploadTextureData = ( PFN_UPLOADTEXTUREDATA ) &uploadTextureData_Invoker < T >;
		_pfnDestroyTexture = ( PFN_DESTROYTEXTURE ) &destroyTexture_Invoker < T >;
		_pfnUpdateTextureData = ( PFN_UPDATETEXTUREDATA ) &updateTextureData_Invoker < T >;
		_pfnGetTextureData = ( PFN_GETTEXTUREDATA ) &getTextureData_Invoker < T >;
        _pfnBindImageToTexture = (PFN_BINDIMAGETOTEXTURE ) &bindImageToTexture_Invoker < T >;

		_pfnCreateShader = ( PFN_CREATESHADER ) &createShader_Invoker < T >;
		_pfnDestroyShader = ( PFN_DESTROYSHADER ) &destroyShader_Invoker < T > ;
		_pfnBindShader = ( PFN_BINDSHADER ) &bindShader_Invoker < T > ;
		_pfnGetShaderConstLoc = ( PFN_GETSHADERCONSTLOC ) &getShaderConstLoc_Invoker < T > ;
		_pfnGetShaderSamplerLoc = ( PFN_GETSHADERSAMPLERLOC ) &getShaderSamplerLoc_Invoker < T > ;
		_pfnGetShaderBufferLoc = ( PFN_GETSHADERBUFFERLOC ) &getShaderBufferLoc_Invoker < T >;
		_pfnRunComputeShader = ( PFN_RUNCOMPUTESHADER ) &runComputeShader_Invoker < T > ;
		_pfnSetShaderConst = ( PFN_SETSHADERCONST ) &setShaderConst_Invoker < T > ;
		_pfnSetShaderSampler = ( PFN_SETSHADERSAMPLER ) &setShaderSampler_Invoker < T > ;
		_pfnGetDefaultVSCode = ( PFN_GETDEFAULTVSCODE ) &getDefaultVSCode_Invoker < T > ;
		_pfnGetDefaultFSCode = ( PFN_GETDEFAULTFSCODE ) &getDefaultFSCode_Invoker < T > ;

		_pfnCreateRenderBuffer = ( PFN_CREATERENDERBUFFER ) &createRenderBuffer_Invoker < T > ;
		_pfnDestroyRenderBuffer = ( PFN_DESTROYRENDERBUFFER ) &destroyRenderBuffer_Invoker < T > ;
		_pfnGetRenderBufferTex = ( PFN_GETRENDERBUFFERTEX ) &getRenderBufferTex_Invoker < T > ;
		_pfnSetRenderBuffer = ( PFN_SETRENDERBUFFER ) &setRenderBuffer_Invoker < T > ;
		_pfnGetRenderBufferData = ( PFN_GETRENDERBUFFERDATA ) &getRenderBufferData_Invoker < T > ;
		_pfnGetRenderBufferDimensions = ( PFN_GETRENDERBUFFERDIMENSIONS ) &getRenderBufferDimensions_Invoker < T >;

		_pfnCreateOcclusionQuery = ( PFN_CREATEOCCLUSIONQUERY ) &createOcclusionQuery_Invoker < T > ;
		_pfnDestroyQuery = ( PFN_DESTROYQUERY ) &destroyQuery_Invoker < T > ;
		_pfnBeginQuery = ( PFN_BEGINQUERY ) &beginQuery_Invoker < T > ;
		_pfnEndQuery = ( PFN_ENDQUERY ) &endQuery_Invoker < T > ;
		_pfnGetQueryResult = ( PFN_GETQUERYRESULT ) &getQueryResult_Invoker < T > ;

		_pfnCreateGPUTimer = ( PFN_CREATEGPUTIMER ) &createGPUTimer_Invoker < T > ;

		_pfnCommitStates = ( PFN_COMMITSTATES ) &commitStates_Invoker < T > ;
		_pfnResetStates = ( PFN_RESETSTATES ) &resetStates_Invoker < T > ;
		_pfnClear = ( PFN_CLEAR ) &clear_Invoker < T > ;
		_pfnDraw = ( PFN_DRAW ) &draw_Invoker < T > ;
		_pfnDrawIndexed = ( PFN_DRAWINDEXED ) &drawIndexed_Invoker < T > ;

		_pfnSetStorageBuffer = ( PFN_SETSTORAGEBUFFER ) &setStorageBuffer_Invoker< T >;
	}

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
		( *_pfnInitStates )( this );
	}
	bool init() 
	{ 
		return ( *_pfnInit )( this );
	}

	
// -----------------------------------------------------------------------------
// Resources
// -----------------------------------------------------------------------------

	// Vertex layouts
	uint32 registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs ) 
	{ 
		return ( *_pfnRegisterVertexLayout )( this, numAttribs, attribs );
	}
	
	// Buffers
	void beginRendering() 
	{ 
		( *_pfnBeginRendering )( this );
	}
	uint32 beginCreatingGeometry( uint32 vlObj )
	{
		return ( *_pfnBeginCreatingGeometry ) ( this, vlObj );
	}
	void setGeomVertexParams( uint32 geoObj, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride )
	{
		( *_pfnSetGeomVertexParams ) ( this, geoObj, vbo, vbSlot, offset, stride );
	}
	void setGeomIndexParams( uint32 geoObj, uint32 indBuf, RDIIndexFormat format )
	{
		( *_pfnSetGeomIndexParams ) ( this, geoObj, indBuf, format );
	}
	void finishCreatingGeometry( uint32 geoObj )
	{
		( *_pfnFinishCreatingGeometry ) ( this, geoObj );
	}
	void destroyGeometry( uint32& geoObj, bool destroyBindedBuffers = true )
	{
		( *_pfnDestroyGeometry ) ( this, geoObj, destroyBindedBuffers );
	}
	uint32 createVertexBuffer( uint32 size, const void *data )
	{
		return ( *_pfnCreateVertexBuffer )( this, size, data );
	}
	uint32 createIndexBuffer( uint32 size, const void *data ) 
	{ 
		return ( *_pfnCreateIndexBuffer )( this, size, data );
	}
	uint32 createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data )
	{
		return ( *_pfnCreateTextureBuffer )( this, format, bufSize, data );
	}
	uint32 createShaderStorageBuffer( uint32 size, const void *data )
	{
		return ( *_pfnCreateShaderStorageBuffer )( this, size, data );
	}
    void destroyBuffer( uint32& bufObj )
	{ 
		( *_pfnDestroyBuffer )( this, bufObj );
	}
	void destroyTextureBuffer( uint32& bufObj )
	{
		( *_pfnDestroyTextureBuffer )( this, bufObj );
	}
	void updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data ) 
	{ 
		( *_pfnUpdateBufferData )( this, geoObj, bufObj, offset, size, data );
	}
	void *mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType )
	{
		return ( *_pfnMapBuffer )( this, geoObj, bufObj, offset, size, mapType );
	}
	void unmapBuffer( uint32 geoObj, uint32 bufObj )
	{
		( *_pfnUnmapBuffer )( this, geoObj, bufObj );
	}

	uint32 getBufferMem() const 
	{ 
		return _bufferMem;
	}

	// Textures
	uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth ) 
	{ 
		return ( *_pfnCalcTextureSize )( this, format, width, height, depth );
	}
	uint32 createTexture( TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
	                      bool hasMips, bool genMips, bool compress, bool sRGB )
	{ 
		return ( *_pfnCreateTexture )( this, type, width, height, depth, format, hasMips, genMips, compress, sRGB );
	}
	void uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
	{ 
		( *_pfnUploadTextureData )( this, texObj, slice, mipLevel, pixels );
	}
	void destroyTexture( uint32& texObj )
	{ 
		( *_pfnDestroyTexture )( this, texObj );
	}
	void updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels ) 
	{ 
		( *_pfnUpdateTextureData )( this, texObj, slice, mipLevel, pixels );
	}
	bool getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer )
	{
		return ( *_pfnGetTextureData )( this, texObj, slice, mipLevel, buffer ); 
	}
	uint32 getTextureMem() const 
	{
		return _textureMem; 
	}
    void bindImageToTexture( uint32 texObj, void* eglImage )
    {
        return ( *_pfnBindImageToTexture )( this, texObj, eglImage );
    }

	// Shaders
	uint32 createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc, 
						 const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc ) 
	{
		return ( *_pfnCreateShader )( this, vertexShaderSrc, fragmentShaderSrc, geometryShaderSrc, 
									  tessControlShaderSrc, tessEvaluationShaderSrc, computeShaderSrc );
	}
    void destroyShader( uint32& shaderId )
	{
		( *_pfnDestroyShader )( this, shaderId ); 
	}
	void bindShader( uint32 shaderId ) 
	{ 
		( *_pfnBindShader )( this, shaderId ); 
	}
	std::string getShaderLog() const 
	{
		return _shaderLog; 
	}
	int getShaderConstLoc( uint32 shaderId, const char *name ) 
	{ 
		return ( *_pfnGetShaderConstLoc )( this, shaderId, name );
	}
	int getShaderSamplerLoc( uint32 shaderId, const char *name )
	{ 
		return ( *_pfnGetShaderSamplerLoc )( this, shaderId, name ); 
	}
	int getShaderBufferLoc( uint32 shaderId, const char *name )
	{
		return ( *_pfnGetShaderBufferLoc )( this, shaderId, name );
	}
	void setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count = 1 ) 
	{
		( *_pfnSetShaderConst )( this, loc, type, values, count );
	}
	void setShaderSampler( int loc, uint32 texUnit ) 
	{
		( *_pfnSetShaderSampler )( this, loc, texUnit ); 
	}
	const char *getDefaultVSCode() 
	{ 
		return ( *_pfnGetDefaultVSCode )( this );
	}
	const char *getDefaultFSCode()  
	{ 
		return ( *_pfnGetDefaultFSCode ) ( this ); 
	}
	void runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim )
	{
		( *_pfnRunComputeShader ) ( this, shaderId, xDim, yDim, zDim );
	}

	// Renderbuffers
	uint32 createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
	                           bool depth, uint32 numColBufs, uint32 samples )
	{
		return ( *_pfnCreateRenderBuffer )( this, width, height, format, depth, numColBufs, samples );
	}
    void destroyRenderBuffer( uint32& rbObj )
	{ 
		( *_pfnDestroyRenderBuffer )( this, rbObj );
	}
	uint32 getRenderBufferTex( uint32 rbObj, uint32 bufIndex ) 
	{
		return ( *_pfnGetRenderBufferTex )( this, rbObj, bufIndex );
	}
	void setRenderBuffer( uint32 rbObj ) 
	{ 
		( *_pfnSetRenderBuffer )( this, rbObj ); 
	}
	bool getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
	                          int *compCount, void *dataBuffer, int bufferSize )
	{
		return ( *_pfnGetRenderBufferData )( this, rbObj, bufIndex, width, height, compCount, dataBuffer, bufferSize );
	}
	void getRenderBufferDimensions( uint32 rbObj, int *width, int *height )
	{
		( *_pfnGetRenderBufferDimensions )( this, rbObj, width, height );
	}

	// Queries
	uint32 createOcclusionQuery() 
	{
		return ( *_pfnCreateOcclusionQuery )( this );
	}
	void destroyQuery( uint32 queryObj ) 
	{
		( *_pfnDestroyQuery ) ( this, queryObj ); 
	}
	void beginQuery( uint32 queryObj ) 
	{ 
		( *_pfnBeginQuery )( this, queryObj ); 
	}
	void endQuery( uint32 queryObj ) 
	{ 
		( *_pfnEndQuery )( this, queryObj ); 
	}
	uint32 getQueryResult( uint32 queryObj )  
	{
		return ( *_pfnGetQueryResult )( this, queryObj );
	}

	// Render Device dependent GPU Timer
	GPUTimer *createGPUTimer() 
	{ 
		return ( *_pfnCreateGPUTimer )( this );
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
	{	( *_pfnSetStorageBuffer )( this, slot, bufObj ); }

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
		return ( *_pfnCommitStates )( this, filter );
	}
	void resetStates() 
	{
		( *_pfnResetStates )( this );
	}
	
	// Draw calls and clears
	void clear( uint32 flags, float *colorRGBA = 0x0, float depth = 1.0f )
	{
		( *_pfnClear )( this, flags, colorRGBA, depth ); 
	}
	void draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
	{
		( *_pfnDraw )( this, primType, firstVert, numVerts );
	}
	void drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
	                  uint32 firstVert, uint32 numVerts )
	{ 
		( *_pfnDrawIndexed )( this, primType, firstIndex, numIndices, firstVert, numVerts );
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

	// 8 ssbo

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
