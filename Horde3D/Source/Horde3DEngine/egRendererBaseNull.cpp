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

#include "egRendererBaseNull.h"
#include "egModules.h"
#include "egCom.h"

#include "utDebug.h"

#include <array>


namespace Horde3D {
namespace RDI_Null {

#define NULL_VERTEX_BUFFER 0
#define NULL_INDEX_BUFFER 1
    
static const char *defaultShaderVS = "Null\n";

static const char *defaultShaderFS = "Null\n";
    
// =================================================================================================
// GPUTimer
// =================================================================================================

GPUTimerNull::GPUTimerNull() : _queryFrame( 0 ), _activeQuery( false )
{
	_beginQuery.bind< GPUTimerNull, &GPUTimerNull::beginQuery >( this );
	_endQuery.bind< GPUTimerNull, &GPUTimerNull::endQuery >( this );
	_updateResults.bind< GPUTimerNull, &GPUTimerNull::updateResults >( this );
	_reset.bind< GPUTimerNull, &GPUTimerNull::reset >( this );

	reset();
}


GPUTimerNull::~GPUTimerNull()
{

}


void GPUTimerNull::beginQuery( uint32 frameID )
{
	ASSERT( !_activeQuery );
	
	if( _queryFrame != frameID )
	{
		if( !updateResults() ) return;

		_queryFrame = frameID;
	}
	
	_t0 = std::chrono::steady_clock::now();
    
	_activeQuery = true;
}


void GPUTimerNull::endQuery()
{
	if( _activeQuery )
	{	
        _t1 = std::chrono::steady_clock::now();
        _activeQuery = false;
	}
}


bool GPUTimerNull::updateResults()
{
	_time = std::chrono::duration_cast<std::chrono::duration< float > >( _t1 - _t0 ).count();
	return true;
}


void GPUTimerNull::reset()
{
	_time = 0.f;
}


// =================================================================================================
// RenderDevice
// =================================================================================================

RenderDeviceNull::RenderDeviceNull()
{
	initRDIFuncs(); // bind render device functions

	_numVertexLayouts = 0;
	
	_vpX = 0; _vpY = 0; _vpWidth = 320; _vpHeight = 240;
	_scX = 0; _scY = 0; _scWidth = 320; _scHeight = 240;
	_prevShaderId = _curShaderId = 0;
	_curRendBuf = 0; _outputBufferIndex = 0;
	_textureMem = 0; _bufferMem = 0;
	_curRasterState.hash = _newRasterState.hash = 0;
	_curBlendState.hash = _newBlendState.hash = 0;
	_curDepthStencilState.hash = _newDepthStencilState.hash = 0;
// 	_curVertLayout = _newVertLayout = 0;
// 	_curIndexBuf = _newIndexBuf = 0;
//	_curIndexBuf = 0;
	_curGeometryIndex = 1;
	_defaultFBO = 0;
	_defaultFBOMultisampled = false;
	_indexFormat = (uint32)IDXFMT_16;
	_activeVertexAttribsMask = 0;
	_pendingMask = 0;
	_tessPatchVerts = 0;

	_maxTexSlots = 16; // for OpenGL 2 there are always 16 texture slots
// 	_texSlots.reserve( _maxTexSlots ); // reserve memory

	_doubleBuffered = false;
	// add default geometry for resetting
//	_geometryInfo.add( RDIGeometryInfoGL2() );
}


RenderDeviceNull::~RenderDeviceNull()
{
}


void RenderDeviceNull::initRDIFuncs()
{
	_delegate_init.bind< RenderDeviceNull, &RenderDeviceNull::init >( this );
	_delegate_initStates.bind< RenderDeviceNull, &RenderDeviceNull::initStates >( this );
	_delegate_enableDebugOutput.bind< RenderDeviceNull, &RenderDeviceNull::enableDebugOutput >( this );
	_delegate_disableDebugOutput.bind< RenderDeviceNull, &RenderDeviceNull::disableDebugOutput >( this );
	_delegate_registerVertexLayout.bind< RenderDeviceNull, &RenderDeviceNull::registerVertexLayout >( this );
	_delegate_beginRendering.bind< RenderDeviceNull, &RenderDeviceNull::beginRendering >( this );

	_delegate_beginCreatingGeometry.bind< RenderDeviceNull, &RenderDeviceNull::beginCreatingGeometry >( this );
	_delegate_finishCreatingGeometry.bind< RenderDeviceNull, &RenderDeviceNull::finishCreatingGeometry >( this );
	_delegate_destroyGeometry.bind< RenderDeviceNull, &RenderDeviceNull::destroyGeometry >( this );
	_delegate_setGeomVertexParams.bind< RenderDeviceNull, &RenderDeviceNull::setGeomVertexParams >( this );
	_delegate_setGeomIndexParams.bind< RenderDeviceNull, &RenderDeviceNull::setGeomIndexParams >( this );
	_delegate_createVertexBuffer.bind< RenderDeviceNull, &RenderDeviceNull::createVertexBuffer >( this );
	_delegate_createIndexBuffer.bind< RenderDeviceNull, &RenderDeviceNull::createIndexBuffer >( this );
	_delegate_createTextureBuffer.bind< RenderDeviceNull, &RenderDeviceNull::createTextureBuffer >( this );
	_delegate_createShaderStorageBuffer.bind< RenderDeviceNull, &RenderDeviceNull::createShaderStorageBuffer >( this );
	_delegate_destroyBuffer.bind< RenderDeviceNull, &RenderDeviceNull::destroyBuffer >( this );
	_delegate_destroyTextureBuffer.bind< RenderDeviceNull, &RenderDeviceNull::destroyTextureBuffer >( this );
	_delegate_updateBufferData.bind< RenderDeviceNull, &RenderDeviceNull::updateBufferData >( this );
	_delegate_mapBuffer.bind< RenderDeviceNull, &RenderDeviceNull::mapBuffer >( this );
	_delegate_unmapBuffer.bind< RenderDeviceNull, &RenderDeviceNull::unmapBuffer >( this );

	_delegate_createTexture.bind< RenderDeviceNull, &RenderDeviceNull::createTexture >( this );
	_delegate_generateTextureMipmap.bind< RenderDeviceNull, &RenderDeviceNull::generateTextureMipmap >( this );
	_delegate_uploadTextureData.bind< RenderDeviceNull, &RenderDeviceNull::uploadTextureData >( this );
	_delegate_destroyTexture.bind< RenderDeviceNull, &RenderDeviceNull::destroyTexture >( this );
	_delegate_updateTextureData.bind< RenderDeviceNull, &RenderDeviceNull::updateTextureData >( this );
	_delegate_getTextureData.bind< RenderDeviceNull, &RenderDeviceNull::getTextureData >( this );
	_delegate_bindImageToTexture.bind< RenderDeviceNull, &RenderDeviceNull::bindImageToTexture >( this );

	_delegate_createShader.bind< RenderDeviceNull, &RenderDeviceNull::createShader >( this );
	_delegate_destroyShader.bind< RenderDeviceNull, &RenderDeviceNull::destroyShader >( this );
	_delegate_bindShader.bind< RenderDeviceNull, &RenderDeviceNull::bindShader >( this );
	_delegate_getShaderConstLoc.bind< RenderDeviceNull, &RenderDeviceNull::getShaderConstLoc >( this );
	_delegate_getShaderSamplerLoc.bind< RenderDeviceNull, &RenderDeviceNull::getShaderSamplerLoc >( this );
	_delegate_getShaderBufferLoc.bind< RenderDeviceNull, &RenderDeviceNull::getShaderBufferLoc >( this );
	_delegate_runComputeShader.bind< RenderDeviceNull, &RenderDeviceNull::runComputeShader >( this );
	_delegate_setShaderConst.bind< RenderDeviceNull, &RenderDeviceNull::setShaderConst >( this );
	_delegate_setShaderSampler.bind< RenderDeviceNull, &RenderDeviceNull::setShaderSampler >( this );
	_delegate_getDefaultVSCode.bind< RenderDeviceNull, &RenderDeviceNull::getDefaultVSCode >( this );
	_delegate_getDefaultFSCode.bind< RenderDeviceNull, &RenderDeviceNull::getDefaultFSCode >( this );

	_delegate_createRenderBuffer.bind< RenderDeviceNull, &RenderDeviceNull::createRenderBuffer >( this );
	_delegate_destroyRenderBuffer.bind< RenderDeviceNull, &RenderDeviceNull::destroyRenderBuffer >( this );
	_delegate_getRenderBufferTex.bind< RenderDeviceNull, &RenderDeviceNull::getRenderBufferTex >( this );
	_delegate_setRenderBuffer.bind< RenderDeviceNull, &RenderDeviceNull::setRenderBuffer >( this );
	_delegate_getRenderBufferData.bind< RenderDeviceNull, &RenderDeviceNull::getRenderBufferData >( this );
	_delegate_getRenderBufferDimensions.bind< RenderDeviceNull, &RenderDeviceNull::getRenderBufferDimensions >( this );

	_delegate_createOcclusionQuery.bind< RenderDeviceNull, &RenderDeviceNull::createOcclusionQuery >( this );
	_delegate_destroyQuery.bind< RenderDeviceNull, &RenderDeviceNull::destroyQuery >( this );
	_delegate_beginQuery.bind< RenderDeviceNull, &RenderDeviceNull::beginQuery >( this );
	_delegate_endQuery.bind< RenderDeviceNull, &RenderDeviceNull::endQuery >( this );
	_delegate_getQueryResult.bind< RenderDeviceNull, &RenderDeviceNull::getQueryResult >( this );

	_delegate_createGPUTimer.bind< RenderDeviceNull, &RenderDeviceNull::createGPUTimer >( this );
	_delegate_commitStates.bind< RenderDeviceNull, &RenderDeviceNull::commitStates >( this );
	_delegate_resetStates.bind< RenderDeviceNull, &RenderDeviceNull::resetStates >( this );
	_delegate_clear.bind< RenderDeviceNull, &RenderDeviceNull::clear >( this );

	_delegate_draw.bind< RenderDeviceNull, &RenderDeviceNull::draw >( this );
	_delegate_drawIndexed.bind< RenderDeviceNull, &RenderDeviceNull::drawIndexed >( this );
	_delegate_setStorageBuffer.bind< RenderDeviceNull, &RenderDeviceNull::setStorageBuffer >( this );
}


void RenderDeviceNull::initStates()
{
    _defaultFBOMultisampled = false;
    _doubleBuffered = true;
    _defaultFBO = 0;
}


bool RenderDeviceNull::init()
{
	Modules::log().writeInfo( "Initializing NULL backend" );
	
    // Save renderer name and version (used for binary shaders)
    _rendererName = "Null";
    _rendererVersion = "1.0";
    		
	// Get capabilities
	_caps.texFloat = 1;
	_caps.texNPOT = 1;
	_caps.rtMultisampling = 1;
	_caps.geometryShaders = true;
	_caps.tesselation = true;
	_caps.computeShaders = true;
	_caps.instancing = true;
	_caps.maxJointCount = 75;
	_caps.maxTexUnitCount = 16;
	_caps.texDXT = true;
	_caps.texETC2 = true;
	_caps.texBPTC = true;
	_caps.texASTC = true;
    _caps.binaryShaders = true;

	// Init states before creating test render buffer, to
	// ensure binding the current FBO again
	initStates();

	// Set depth format 
	_depthFormat = 32;
	
	resetStates();

	return true;
}


bool RenderDeviceNull::enableDebugOutput()
{
	return true;
}


bool RenderDeviceNull::disableDebugOutput()
{
	return true;
}

// =================================================================================================
// Vertex layouts
// =================================================================================================

uint32 RenderDeviceNull::registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs )
{
	if( _numVertexLayouts == MaxNumVertexLayouts )
		return 0;
	
	_vertexLayouts[_numVertexLayouts].numAttribs = numAttribs;

	for( uint32 i = 0; i < numAttribs; ++i )
		_vertexLayouts[_numVertexLayouts].attribs[i] = attribs[i];

	return ++_numVertexLayouts;
}


// =================================================================================================
// Buffers
// =================================================================================================

void RenderDeviceNull::beginRendering()
{	
	//	Get the currently bound frame buffer object. 
    _defaultFBO = 0;
	resetStates();
}

uint32 RenderDeviceNull::beginCreatingGeometry( uint32 vlObj )
{
	uint32 idx = _geometryInfo.add( RDIGeometryInfoNull() );
	RDIGeometryInfoNull &geo = _geometryInfo.getRef( idx );

	geo.layout = vlObj;

	return idx;
}

void RenderDeviceNull::finishCreatingGeometry( uint32 geoObj )
{
	H3D_UNUSED_VAR( geoObj );
}

void RenderDeviceNull::setGeomVertexParams( uint32 geoObj, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride )
{
	RDIGeometryInfoNull &geo = _geometryInfo.getRef( geoObj );
	RDIBufferNull &buf = _buffers.getRef( vbo );

	buf.geometryRefCount++;
// 	if ( geo.vertexBufInfo.size() - 1 >= vbSlot )
// 	{
// 		// parameters found, change them
// 		RDIVertBufSlotGL2 &attribInfo = geo.vertexBufInfo[ vbSlot ];
// 		attribInfo.vbObj = vbo;
// 		attribInfo.offset = offset;
// 		attribInfo.stride = stride;
// 	} 
// 	else
	{
		// create new vertex parameters
		RDIVertBufSlotNull attribInfo( vbo, offset, stride );
		geo.vertexBufInfo.push_back( attribInfo );
	}
}

void RenderDeviceNull::setGeomIndexParams( uint32 geoObj, uint32 indBuf, RDIIndexFormat format )
{
	RDIGeometryInfoNull &geo = _geometryInfo.getRef( geoObj );
	RDIBufferNull &buf = _buffers.getRef( indBuf );

	buf.geometryRefCount++;
	geo.indexBuf = indBuf;
	geo.indexBuf32Bit = format == IDXFMT_32 ? true : false;
}

void RenderDeviceNull::destroyGeometry( uint32& geoObj, bool destroyBindedBuffers )
{
	if ( geoObj == 0 )
		return;

	RDIGeometryInfoNull &geo = _geometryInfo.getRef( geoObj );
	
	if ( destroyBindedBuffers )
	{
		for ( unsigned int i = 0; i < geo.vertexBufInfo.size(); ++i )
		{
			decreaseBufferRefCount( geo.vertexBufInfo[ i ].vbObj );
			destroyBuffer( geo.vertexBufInfo[ i ].vbObj );
		}

		decreaseBufferRefCount( geo.indexBuf );
		destroyBuffer( geo.indexBuf );
	}
	else
	{
		// just decrease reference count
		for ( unsigned int i = 0; i < geo.vertexBufInfo.size(); ++i )
		{
			decreaseBufferRefCount( geo.vertexBufInfo[ i ].vbObj );
		}

		decreaseBufferRefCount( geo.indexBuf );
	}
	
	_geometryInfo.remove( geoObj );
	geoObj = 0;
}


void RenderDeviceNull::decreaseBufferRefCount( uint32 bufObj )
{
	if ( bufObj == 0 ) return;

	RDIBufferNull &buf = _buffers.getRef( bufObj );

	buf.geometryRefCount--;
}


uint32 RenderDeviceNull::createVertexBuffer( uint32 size, const void *data )
{
	return createBuffer( NULL_VERTEX_BUFFER, size, data );
}


uint32 RenderDeviceNull::createIndexBuffer( uint32 size, const void *data )
{
	return createBuffer( NULL_INDEX_BUFFER, size, data );;
}


uint32 RenderDeviceNull::createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data )
{
    H3D_UNUSED_VAR( format );
	H3D_UNUSED_VAR( bufSize );
    H3D_UNUSED_VAR( data );
    
	return 0;
}


uint32 RenderDeviceNull::createShaderStorageBuffer( uint32 size, const void *data )
{
	H3D_UNUSED_VAR( size );
	H3D_UNUSED_VAR( data );

	Modules::log().writeError( "Shader storage buffers are not supported on OpenGL 2 devices." );

	return 0;
}


uint32 RenderDeviceNull::createBuffer( uint32 bufType, uint32 size, const void *data )
{
	RDIBufferNull buf;

	buf.type = bufType;
	buf.size = size;
	
	_bufferMem += size;
	return _buffers.add( buf );
}


void RenderDeviceNull::destroyBuffer( uint32& bufObj )
{
	if( bufObj == 0 )
		return;
	
	RDIBufferNull &buf = _buffers.getRef( bufObj );

	if ( buf.geometryRefCount < 1 )
	{
		_bufferMem -= buf.size;
		_buffers.remove( bufObj );
		bufObj = 0;
	}
}


void RenderDeviceNull::destroyTextureBuffer( uint32& bufObj )
{
	H3D_UNUSED_VAR( bufObj );
}


void RenderDeviceNull::updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data )
{
	
}


void * RenderDeviceNull::mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType )
{
	return nullptr;
}


void RenderDeviceNull::unmapBuffer( uint32 geoObj, uint32 bufObj )
{
    H3D_UNUSED_VAR( geoObj );
    H3D_UNUSED_VAR( bufObj );
}


// =================================================================================================
// Textures
// =================================================================================================

uint32 RenderDeviceNull::createTexture( TextureTypes::List type, int width, int height, int depth,
                                       TextureFormats::List format,
                                       int maxMipLevel, bool genMips, bool compress, bool sRGB )
{
	ASSERT( depth > 0 );

	if ( !_caps.texETC2 && ( format == TextureFormats::ETC1 || format == TextureFormats::RGB8_ETC2 || format == TextureFormats::RGBA8_ETC2 ) )
	{
		Modules::log().writeWarning( "Unsupported texture formats: ETC1, ETC2" );
		return 0;
	}
	if ( !_caps.texBPTC && ( format == TextureFormats::BC6_SF16 || format == TextureFormats::BC6_UF16 || format == TextureFormats::BC7 ) )
	{
		Modules::log().writeWarning( "Unsupported texture formats: BC6, BC7" );
		return 0;
	}
	if ( !_caps.texASTC && ( format >= TextureFormats::ASTC_4x4 && format <= TextureFormats::ASTC_12x12 ) )
	{
		Modules::log().writeWarning( "Unsupported texture formats: ASTC" );
		return 0;
	}
	if( !_caps.texNPOT )
	{
		// Check if texture is NPOT
		if( (width & (width-1)) != 0 || (height & (height-1)) != 0 )
			Modules::log().writeWarning( "Texture has non-power-of-two dimensions although NPOT is not supported by GPU" );
	}
	
	RDITextureNull tex;
	tex.type = 1;
	tex.format = format;
	tex.width = width;
	tex.height = height;
	tex.depth = depth;
	tex.sRGB = sRGB && Modules::config().sRGBLinearization;
	tex.genMips = genMips;
	tex.hasMips = maxMipLevel > 0;

	tex.glFmt = 1;
	
	tex.samplerState = 0;
	applySamplerState( tex );
	
	// Calculate memory requirements
	tex.memSize = calcTextureSize( format, width, height, depth, maxMipLevel );
	if( type == TextureTypes::TexCube ) tex.memSize *= 6;
	_textureMem += tex.memSize;
	
	return _textures.add( tex );
}


void RenderDeviceNull::generateTextureMipmap( uint32 texObj )
{
	H3D_UNUSED_VAR( texObj );
}


void RenderDeviceNull::uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	H3D_UNUSED_VAR( texObj );
    H3D_UNUSED_VAR( slice );
    H3D_UNUSED_VAR( mipLevel );
    H3D_UNUSED_VAR( pixels );
}


void RenderDeviceNull::destroyTexture( uint32& texObj )
{
	if( texObj == 0 )
		return;
	
	const RDITextureNull &tex = _textures.getRef( texObj );

	_textureMem -= tex.memSize;
	_textures.remove( texObj );
	texObj = 0;
}


bool RenderDeviceNull::isCompressedTextureFormat( TextureFormats::List fmt )
{
	return	( fmt == TextureFormats::DXT1 ) || ( fmt == TextureFormats::DXT3 ) ||
			( fmt == TextureFormats::DXT5 ) || ( fmt == TextureFormats::ETC1 ) ||
			( fmt == TextureFormats::RGB8_ETC2 ) || ( fmt == TextureFormats::RGBA8_ETC2 ) ||
			( fmt == TextureFormats::BC6_SF16 ) || ( fmt == TextureFormats::BC6_UF16 ) ||
			( fmt == TextureFormats::BC7 ) || ( fmt == TextureFormats::ASTC_4x4 ) ||
			( fmt == TextureFormats::ASTC_5x4 ) || ( fmt == TextureFormats::ASTC_5x5 ) ||
			( fmt == TextureFormats::ASTC_6x5 ) || ( fmt == TextureFormats::ASTC_6x6 ) ||
			( fmt == TextureFormats::ASTC_8x5 ) || ( fmt == TextureFormats::ASTC_8x6 ) ||
			( fmt == TextureFormats::ASTC_8x8 ) || ( fmt == TextureFormats::ASTC_10x5 ) ||
			( fmt == TextureFormats::ASTC_10x6 ) || ( fmt == TextureFormats::ASTC_10x8 ) ||
			( fmt == TextureFormats::ASTC_10x10 ) || ( fmt == TextureFormats::ASTC_12x10 ) ||
			( fmt == TextureFormats::ASTC_12x12 ) ;
}


void RenderDeviceNull::updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	uploadTextureData( texObj, slice, mipLevel, pixels );
}


bool RenderDeviceNull::getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer )
{
	const RDITextureNull &tex = _textures.getRef( texObj );

    memset( buffer, 0, tex.memSize );
	
	return true;
}

void RenderDeviceNull::bindImageToTexture(uint32 texObj, void *eglImage)
{
    H3D_UNUSED_VAR( texObj );
    H3D_UNUSED_VAR( eglImage );
}

// =================================================================================================
// Shaders
// =================================================================================================

uint32 RenderDeviceNull::createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc, 
                                              const char *tessControlShaderSrc, const char *tessEvalShaderSrc, const char *computeShaderSrc )
{
    H3D_UNUSED_VAR( vertexShaderSrc );
    H3D_UNUSED_VAR( fragmentShaderSrc );
    H3D_UNUSED_VAR( geometryShaderSrc );
    H3D_UNUSED_VAR( tessControlShaderSrc );
    H3D_UNUSED_VAR( tessEvalShaderSrc );
    H3D_UNUSED_VAR( computeShaderSrc );
    
	return 1;
}


bool RenderDeviceNull::linkShaderProgram( uint32 programObj )
{
    H3D_UNUSED_VAR( programObj );
	return true;
}


uint32 RenderDeviceNull::createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
                                      const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc )
{
	H3D_UNUSED_VAR( geometryShaderSrc );
	H3D_UNUSED_VAR( tessControlShaderSrc );
	H3D_UNUSED_VAR( tessEvaluationShaderSrc );
	H3D_UNUSED_VAR( computeShaderSrc );

	// Compile and link shader
	uint32 programObj = createShaderProgram( vertexShaderSrc, fragmentShaderSrc, geometryShaderSrc, 
                                             tessControlShaderSrc, tessEvaluationShaderSrc, computeShaderSrc );
	if( programObj == 0 ) return 0;
	if( !linkShaderProgram( programObj ) ) return 0;
	
	uint32 shaderId = _shaders.add( RDIShaderNull() );
	RDIShaderNull &shader = _shaders.getRef( shaderId );
	shader.oglProgramObj = programObj;
	
	return shaderId;
}


void RenderDeviceNull::destroyShader( uint32& shaderId )
{
	if( shaderId == 0 )
		return;

	RDIShaderNull &shader = _shaders.getRef( shaderId );
	_shaders.remove( shaderId );
	shaderId = 0;
}


void RenderDeviceNull::bindShader( uint32 shaderId )
{
	_curShaderId = shaderId;
	_pendingMask |= PM_GEOMETRY;
} 


int RenderDeviceNull::getShaderConstLoc( uint32 shaderId, const char *name )
{
    return 0;
}


int RenderDeviceNull::getShaderSamplerLoc( uint32 shaderId, const char *name )
{
	return 0;
}


int RenderDeviceNull::getShaderBufferLoc( uint32 shaderId, const char *name )
{
	H3D_UNUSED_VAR( shaderId );
	H3D_UNUSED_VAR( name );

	return -1;
}

void RenderDeviceNull::setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count )
{
	H3D_UNUSED_VAR( loc );
    H3D_UNUSED_VAR( type );
    H3D_UNUSED_VAR( values );
    H3D_UNUSED_VAR( count );
}


void RenderDeviceNull::setShaderSampler( int loc, uint32 texUnit )
{
    H3D_UNUSED_VAR( loc );
    H3D_UNUSED_VAR( texUnit );
}


const char *RenderDeviceNull::getDefaultVSCode()
{
	return defaultShaderVS;
}


const char *RenderDeviceNull::getDefaultFSCode()
{
	return defaultShaderFS;
}


void RenderDeviceNull::runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim )
{
	Modules::log().writeInfo( "Launching compute shader: shaderID = %u xDim = %u yDim = %u zDim = %u", 
                              shaderId, xDim, yDim, zDim );
}

// =================================================================================================
// Renderbuffers
// =================================================================================================

uint32 RenderDeviceNull::createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
                                            bool depth, uint32 numColBufs, uint32 samples, uint32 maxMipLevel )
{
	if( numColBufs > RDIRenderBufferNull::MaxColorAttachmentCount ) return 0;

	uint32 maxSamples = 0;
	if( _caps.rtMultisampling )
	{
		maxSamples = 16;
	}
	if( samples > maxSamples )
	{
		samples = maxSamples;
		Modules::log().writeWarning( "GPU does not support desired multisampling quality for render target" );
	}

	RDIRenderBufferNull rb;
	rb.width = width;
	rb.height = height;
	rb.samples = samples;

	// Create framebuffers
    rb.fbo = 1;
	
	if( numColBufs > 0 )
	{
		// Attach color buffers
		for( uint32 j = 0; j < numColBufs; ++j )
		{
			// Create a color texture
			uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, format, maxMipLevel, maxMipLevel > 0, false, false );
			ASSERT( texObj != 0 );
			uploadTextureData( texObj, 0, 0, 0x0 );
			rb.colTexs[j] = texObj;

		}
	}

	// Attach depth buffer
	if( depth )
	{
		// Create a depth texture
		uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, TextureFormats::DEPTH, 0, false, false, false );
		ASSERT( texObj != 0 );
		uploadTextureData( texObj, 0, 0, 0x0 );
		rb.depthTex = texObj;
	}

	uint32 rbObj = _rendBufs.add( rb );
			
	return rbObj;
}


void RenderDeviceNull::destroyRenderBuffer( uint32& rbObj )
{
	RDIRenderBufferNull &rb = _rendBufs.getRef( rbObj );

	rb.depthTex = rb.depthBuf = 0;
		
	for( uint32 i = 0; i < RDIRenderBufferNull::MaxColorAttachmentCount; ++i )
	{
		if( rb.colTexs[i] != 0 ) destroyTexture( rb.colTexs[i] );
		rb.colTexs[i] = rb.colBufs[i] = 0;
	}

	rb.fbo = rb.fboMS = 0;

	_rendBufs.remove( rbObj );
	rbObj = 0;
}


uint32 RenderDeviceNull::getRenderBufferTex( uint32 rbObj, uint32 bufIndex )
{
	RDIRenderBufferNull &rb = _rendBufs.getRef( rbObj );
	
	if( bufIndex < RDIRenderBufferNull::MaxColorAttachmentCount ) return rb.colTexs[bufIndex];
	else if( bufIndex == 32 ) return rb.depthTex;
	else return 0;
}

void RenderDeviceNull::getRenderBufferDimensions( uint32 rbObj, int *width, int *height )
{
	RDIRenderBufferNull &rb = _rendBufs.getRef( rbObj );
	
	*width = rb.width;
	*height = rb.height;
}

void RenderDeviceNull::resolveRenderBuffer( uint32 rbObj )
{
	RDIRenderBufferNull &rb = _rendBufs.getRef( rbObj );
	
	if( rb.fboMS == 0 ) return;
}


void RenderDeviceNull::setRenderBuffer( uint32 rbObj )
{
	// Resolve render buffer if necessary
	if( _curRendBuf != 0 ) resolveRenderBuffer( _curRendBuf );
	
	// Set new render buffer
	_curRendBuf = rbObj;
	
	if( rbObj == 0 )
	{
		_fbWidth = _vpWidth + _vpX;
		_fbHeight = _vpHeight + _vpY;
	}
	else
	{
		// Unbind all textures to make sure that no FBO attachment is bound any more
		for( uint32 i = 0; i < 16; ++i ) setTexture( i, 0, 0, 0 );
		commitStates( PM_TEXTURES );
	}
}


bool RenderDeviceNull::getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
                                        int *compCount, void *dataBuffer, int bufferSize )
{
	int x, y, w, h;

	beginRendering();
	
	if( rbObj == 0 )
	{
		if( bufIndex != 32 && bufIndex != 0 ) return false;
		if( width != 0x0 ) *width = _vpWidth;
		if( height != 0x0 ) *height = _vpHeight;
		
		x = _vpX; y = _vpY; w = _vpWidth; h = _vpHeight;
	}
	else
	{
		resolveRenderBuffer( rbObj );
	}
	
	int comps = (bufIndex == 32 ? 1 : 4);
	if( compCount != 0x0 ) *compCount = comps;
	
    memset( dataBuffer, 0, bufferSize );
    
	return true;
}


// =================================================================================================
// Queries
// =================================================================================================

uint32 RenderDeviceNull::createOcclusionQuery()
{
    return 0;
}


void RenderDeviceNull::destroyQuery( uint32 queryObj )
{
    H3D_UNUSED_VAR( queryObj );
}


void RenderDeviceNull::beginQuery( uint32 queryObj )
{
    H3D_UNUSED_VAR( queryObj );
}


void RenderDeviceNull::endQuery( uint32 /*queryObj*/ )
{

}


uint32 RenderDeviceNull::getQueryResult( uint32 queryObj )
{
	H3D_UNUSED_VAR( queryObj );
	return 0;
}


// =================================================================================================
// Internal state management
// =================================================================================================

void RenderDeviceNull::checkError()
{

}


void RenderDeviceNull::setStorageBuffer( uint8 slot, uint32 bufObj )
{
	H3D_UNUSED_VAR( slot );
	H3D_UNUSED_VAR( bufObj );
}


bool RenderDeviceNull::applyVertexLayout( RDIGeometryInfoNull &geo )
{
	uint32 newVertexAttribMask = 0;
	
	if( geo.layout != 0 )
	{
		if( _curShaderId == 0 ) return false;
		
		RDIVertexLayout &vl = _vertexLayouts[ geo.layout - 1];
		RDIShaderNull &shader = _shaders.getRef( _curShaderId );
		RDIInputLayoutNull &inputLayout = shader.inputLayouts[ geo.layout - 1];
		
		if( !inputLayout.valid )
			return false;

	}
	
	_activeVertexAttribsMask = newVertexAttribMask;

	return true;
}


void RenderDeviceNull::applySamplerState( RDITextureNull &tex )
{

}


void RenderDeviceNull::applyRenderStates()
{
	// Rasterizer state
	if( _newRasterState.hash != _curRasterState.hash )
	{	
		_curRasterState.hash = _newRasterState.hash;
	}

	// Blend state
	if( _newBlendState.hash != _curBlendState.hash )
	{	
		_curBlendState.hash = _newBlendState.hash;
	}

	// Depth-stencil state
	if( _newDepthStencilState.hash != _curDepthStencilState.hash )
	{
		_curDepthStencilState.hash = _newDepthStencilState.hash;
	}
}


bool RenderDeviceNull::commitStates( uint32 filter )
{
	if( _pendingMask & filter )
	{
		uint32 mask = _pendingMask & filter;
	
		// Set viewport
		if( mask & PM_VIEWPORT )
		{
			_pendingMask &= ~PM_VIEWPORT;
		}

		if( mask & PM_RENDERSTATES )
		{
			applyRenderStates();
			_pendingMask &= ~PM_RENDERSTATES;
		}

		// Set scissor rect
		if( mask & PM_SCISSOR )
		{
			_pendingMask &= ~PM_SCISSOR;
		}
		
		// Bind textures and set sampler state
		if( mask & PM_TEXTURES )
		{
			_pendingMask &= ~PM_TEXTURES;
		}

		// Bind vertex buffers
		if( mask & PM_GEOMETRY )
		{
            _indexFormat = 0;
            _prevShaderId = _curShaderId;
            _pendingMask &= ~PM_GEOMETRY;	
		}

	}

	return true;
}


void RenderDeviceNull::resetStates()
{
// 	_curVertLayout = 1; _newVertLayout = 0;
	_curGeometryIndex = 1;
	_curRasterState.hash = 0xFFFFFFFF; _newRasterState.hash = 0;
	_curBlendState.hash = 0xFFFFFFFF; _newBlendState.hash = 0;
	_curDepthStencilState.hash = 0xFFFFFFFF; _newDepthStencilState.hash = 0;

//	_texSlots.clear();
	for( uint32 i = 0; i < 16; ++i )
		setTexture( i, 0, 0, 0 );

	setColorWriteMask( true );
	_pendingMask = 0xFFFFFFFF;
}


// =================================================================================================
// Draw calls and clears
// =================================================================================================

void RenderDeviceNull::clear( uint32 flags, float *colorRGBA, float depth )
{
    Modules::log().writeInfo( "%s: clearing with flags %u, colorRGBA is %p, depth is %f",
                              __func__, flags, colorRGBA, depth );
}


void RenderDeviceNull::draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
{
    Modules::log().writeInfo( "%s: drawing with primType %d, firstVert is %u, numVerts is %u",
                              __func__, primType, firstVert, numVerts );
    
    commitStates();
}


void RenderDeviceNull::drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
                                uint32 firstVert, uint32 numVerts )
{
    Modules::log().writeInfo( "%s: drawing with primType %d, firstIndex is %u,"
                              "numIndices is %u, firstVert is %u, numVerts is %u",
                              __func__, primType, firstIndex, numIndices, firstVert, numVerts );
    
    commitStates();
}

}  // namespace RDI_Null
}  // namespace Horde3D

