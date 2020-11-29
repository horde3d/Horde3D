// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2020 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#include "egRendererBaseGL2.h"
#include "egModules.h"
#include "egCom.h"
#include "utOpenGL.h"

#include "utDebug.h"

#include <array>


namespace Horde3D {
namespace RDI_GL2 {

	using namespace h3dGL;

#ifdef H3D_VALIDATE_DRAWCALLS
#	define CHECK_GL_ERROR checkError();
#else
#	define CHECK_GL_ERROR
#endif

static const char *defaultShaderVS =
	"uniform mat4 viewProjMat;\n"
	"uniform mat4 worldMat;\n"
	"attribute vec3 vertPos;\n"
	"void main() {\n"
	"	gl_Position = viewProjMat * worldMat * vec4( vertPos, 1.0 );\n"
	"}\n";

static const char *defaultShaderFS =
	"uniform vec4 color;\n"
	"void main() {\n"
	"	gl_FragColor = color;\n"
	"}\n";

static const uint32 indexFormats[ 2 ] = { GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };

static const uint32 primitiveTypes[ 5 ] = { GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_LINES, GL_POINTS, GL_TRIANGLES }; // GL_PATCHES is not supported for gl 2

static const uint32 textureTypes[ 3 ] = { GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };

static const uint32 bufferMappingTypes[ 3 ] = { GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE };

// Texture formats mapping to supported non compressed GL texture formats
struct GLTextureFormatAndType
{
	uint32 glCreateFormat;
	uint32 glInputFormat;
	uint32 glSRGBFormat;
	uint32 glInputType;
};

// DEPTH is used as array count as it is always last in the list
// if no srgb format is available the same format is used
static const std::array< GLTextureFormatAndType, TextureFormats::DEPTH + 1 > textureGLFormats = { {
	{ 0, 0, 0, 0 },																							// TextureFormats::Unknown
	{ GL_R8, GL_RED, GL_R8, GL_UNSIGNED_BYTE },																// TextureFormats::R8
	{ GL_R16, GL_RED, GL_R16, GL_HALF_FLOAT },																// TextureFormats::R16F
	{ GL_R32F, GL_RED, GL_R32F, GL_FLOAT },																	// TextureFormats::R32F
	{ GL_RG8, GL_RG, GL_RG8, GL_UNSIGNED_BYTE },															// TextureFormats::RG8
	{ GL_RG16F, GL_RG, GL_RG16F, GL_HALF_FLOAT },															// TextureFormats::RG16F
	{ GL_RG32F, GL_RG, GL_RG32F, GL_FLOAT },																// TextureFormats::RG32F
	{ GL_RGBA8, GL_BGRA, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE },												// TextureFormats::BGRA8
	{ GL_RGBA16F, GL_RGBA, GL_RGBA16F, GL_HALF_FLOAT },														// TextureFormats::RGBA16F
	{ GL_RGBA32F, GL_RGBA, GL_RGBA32F, GL_FLOAT },															// TextureFormats::RGBA32F
	{ GL_RGBA32UI, GL_RGBA_INTEGER, GL_RGBA32UI, GL_UNSIGNED_INT },											// TextureFormats::RGBA32UI
	{ GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0 },						// TextureFormats::DXT1
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0 },						// TextureFormats::DXT3
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0 },						// TextureFormats::DXT5
	{ 0, 0, 0, 0 },																							// TextureFormats::ETC1
	{ 0, 0, 0, 0 },																							// TextureFormats::RGB8_ETC2
	{ 0, 0, 0, 0 },																							// TextureFormats::RGBA8_ETC2
	{ GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB, 0, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB, GL_FLOAT },	// TextureFormats::BC6_UF16
	{ GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB, 0, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB, GL_FLOAT },		// TextureFormats::BC6_SF16
	{ GL_COMPRESSED_RGBA_BPTC_UNORM_ARB, 0, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB, GL_UNSIGNED_BYTE },	// TextureFormats::BC7
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_4x4
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_5x4
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_5x5
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_6x5
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_6x6
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_8x5
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_8x6
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_8x8
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_10x5
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_10x6
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_10x8
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_10x10
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_12x10
	{ 0, 0, 0, 0 },																							// TextureFormats::ASTC_12x12
	{ GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT }								// TextureFormats::DEPTH
} };

// =================================================================================================
// GPUTimer
// =================================================================================================

GPUTimerGL2::GPUTimerGL2() : _numQueries( 0 ),  _queryFrame( 0 ), _activeQuery( false )
{
	_beginQuery.bind< GPUTimerGL2, &GPUTimerGL2::beginQuery >( this );
	_endQuery.bind< GPUTimerGL2, &GPUTimerGL2::endQuery >( this );
	_updateResults.bind< GPUTimerGL2, &GPUTimerGL2::updateResults >( this );
	_reset.bind< GPUTimerGL2, &GPUTimerGL2::reset >( this );

	reset();
}


GPUTimerGL2::~GPUTimerGL2()
{
	if( !_queryPool.empty() )
		glDeleteQueries( (uint32)_queryPool.size(), &_queryPool[0] );
}


void GPUTimerGL2::beginQuery( uint32 frameID )
{
	if( !glExt::ARB_timer_query ) return;
	ASSERT( !_activeQuery );
	
	if( _queryFrame != frameID )
	{
		if( !updateResults() ) return;

		_queryFrame = frameID;
		_numQueries = 0;
	}
	
	// Create new query pair if necessary
	uint32 queryObjs[2];
	if( _numQueries++ * 2 == _queryPool.size() )
	{
		glGenQueries( 2, queryObjs );
		_queryPool.push_back( queryObjs[0] );
		_queryPool.push_back( queryObjs[1] );
	}
	else
	{
		queryObjs[0] = _queryPool[(_numQueries - 1) * 2];
	}
	
	_activeQuery = true;
	 glQueryCounterARB( queryObjs[0], GL_TIMESTAMP );
}


void GPUTimerGL2::endQuery()
{
	if( _activeQuery )
	{	
		glQueryCounterARB( _queryPool[_numQueries * 2 - 1], GL_TIMESTAMP );
		_activeQuery = false;
	}
}


bool GPUTimerGL2::updateResults()
{
	if( !glExt::ARB_timer_query ) return false;
	
	if( _numQueries == 0 )
	{
		_time = 0;
		return true;
	}
	
	// Make sure that last query is available
	GLint available;
	glGetQueryObjectiv( _queryPool[_numQueries * 2 - 1], GL_QUERY_RESULT_AVAILABLE, &available );
	if( !available ) return false;
	
	//  Accumulate time
	GLuint64 timeStart = 0, timeEnd = 0, timeAccum = 0;
	for( uint32 i = 0; i < _numQueries; ++i )
	{
		glGetQueryObjectui64vARB( _queryPool[i * 2], GL_QUERY_RESULT, &timeStart );
		glGetQueryObjectui64vARB( _queryPool[i * 2 + 1], GL_QUERY_RESULT, &timeEnd );
		timeAccum += timeEnd - timeStart;
	}
	
	_time = (float)((double)timeAccum / 1000000.0);
	return true;
}


void GPUTimerGL2::reset()
{
	_time = glExt::ARB_timer_query ? 0.f : -1.f;
}


// =================================================================================================
// RenderDevice
// =================================================================================================

RenderDeviceGL2::RenderDeviceGL2()
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
	_curIndexBuf = 0;
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
	_geometryInfo.add( RDIGeometryInfoGL2() );
}


RenderDeviceGL2::~RenderDeviceGL2()
{
}


void RenderDeviceGL2::initRDIFuncs()
{
	_delegate_init.bind< RenderDeviceGL2, &RenderDeviceGL2::init >( this );
	_delegate_initStates.bind< RenderDeviceGL2, &RenderDeviceGL2::initStates >( this );
	_delegate_enableDebugOutput.bind< RenderDeviceGL2, &RenderDeviceGL2::enableDebugOutput >( this );
	_delegate_disableDebugOutput.bind< RenderDeviceGL2, &RenderDeviceGL2::disableDebugOutput >( this );
	_delegate_registerVertexLayout.bind< RenderDeviceGL2, &RenderDeviceGL2::registerVertexLayout >( this );
	_delegate_beginRendering.bind< RenderDeviceGL2, &RenderDeviceGL2::beginRendering >( this );

	_delegate_beginCreatingGeometry.bind< RenderDeviceGL2, &RenderDeviceGL2::beginCreatingGeometry >( this );
	_delegate_finishCreatingGeometry.bind< RenderDeviceGL2, &RenderDeviceGL2::finishCreatingGeometry >( this );
	_delegate_destroyGeometry.bind< RenderDeviceGL2, &RenderDeviceGL2::destroyGeometry >( this );
	_delegate_setGeomVertexParams.bind< RenderDeviceGL2, &RenderDeviceGL2::setGeomVertexParams >( this );
	_delegate_setGeomIndexParams.bind< RenderDeviceGL2, &RenderDeviceGL2::setGeomIndexParams >( this );
	_delegate_createVertexBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::createVertexBuffer >( this );
	_delegate_createIndexBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::createIndexBuffer >( this );
	_delegate_createTextureBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::createTextureBuffer >( this );
	_delegate_createShaderStorageBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::createShaderStorageBuffer >( this );
	_delegate_destroyBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::destroyBuffer >( this );
	_delegate_destroyTextureBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::destroyTextureBuffer >( this );
	_delegate_updateBufferData.bind< RenderDeviceGL2, &RenderDeviceGL2::updateBufferData >( this );
	_delegate_mapBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::mapBuffer >( this );
	_delegate_unmapBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::unmapBuffer >( this );

	_delegate_createTexture.bind< RenderDeviceGL2, &RenderDeviceGL2::createTexture >( this );
	_delegate_generateTextureMipmap.bind< RenderDeviceGL2, &RenderDeviceGL2::generateTextureMipmap >( this );
	_delegate_uploadTextureData.bind< RenderDeviceGL2, &RenderDeviceGL2::uploadTextureData >( this );
	_delegate_destroyTexture.bind< RenderDeviceGL2, &RenderDeviceGL2::destroyTexture >( this );
	_delegate_updateTextureData.bind< RenderDeviceGL2, &RenderDeviceGL2::updateTextureData >( this );
	_delegate_getTextureData.bind< RenderDeviceGL2, &RenderDeviceGL2::getTextureData >( this );
	_delegate_bindImageToTexture.bind< RenderDeviceGL2, &RenderDeviceGL2::bindImageToTexture >( this );

	_delegate_createShader.bind< RenderDeviceGL2, &RenderDeviceGL2::createShader >( this );
	_delegate_destroyShader.bind< RenderDeviceGL2, &RenderDeviceGL2::destroyShader >( this );
	_delegate_bindShader.bind< RenderDeviceGL2, &RenderDeviceGL2::bindShader >( this );
	_delegate_getShaderConstLoc.bind< RenderDeviceGL2, &RenderDeviceGL2::getShaderConstLoc >( this );
	_delegate_getShaderSamplerLoc.bind< RenderDeviceGL2, &RenderDeviceGL2::getShaderSamplerLoc >( this );
	_delegate_getShaderBufferLoc.bind< RenderDeviceGL2, &RenderDeviceGL2::getShaderBufferLoc >( this );
	_delegate_runComputeShader.bind< RenderDeviceGL2, &RenderDeviceGL2::runComputeShader >( this );
	_delegate_setShaderConst.bind< RenderDeviceGL2, &RenderDeviceGL2::setShaderConst >( this );
	_delegate_setShaderSampler.bind< RenderDeviceGL2, &RenderDeviceGL2::setShaderSampler >( this );
	_delegate_getDefaultVSCode.bind< RenderDeviceGL2, &RenderDeviceGL2::getDefaultVSCode >( this );
	_delegate_getDefaultFSCode.bind< RenderDeviceGL2, &RenderDeviceGL2::getDefaultFSCode >( this );

	_delegate_createRenderBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::createRenderBuffer >( this );
	_delegate_destroyRenderBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::destroyRenderBuffer >( this );
	_delegate_getRenderBufferTex.bind< RenderDeviceGL2, &RenderDeviceGL2::getRenderBufferTex >( this );
	_delegate_setRenderBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::setRenderBuffer >( this );
	_delegate_getRenderBufferData.bind< RenderDeviceGL2, &RenderDeviceGL2::getRenderBufferData >( this );
	_delegate_getRenderBufferDimensions.bind< RenderDeviceGL2, &RenderDeviceGL2::getRenderBufferDimensions >( this );

	_delegate_createOcclusionQuery.bind< RenderDeviceGL2, &RenderDeviceGL2::createOcclusionQuery >( this );
	_delegate_destroyQuery.bind< RenderDeviceGL2, &RenderDeviceGL2::destroyQuery >( this );
	_delegate_beginQuery.bind< RenderDeviceGL2, &RenderDeviceGL2::beginQuery >( this );
	_delegate_endQuery.bind< RenderDeviceGL2, &RenderDeviceGL2::endQuery >( this );
	_delegate_getQueryResult.bind< RenderDeviceGL2, &RenderDeviceGL2::getQueryResult >( this );

	_delegate_createGPUTimer.bind< RenderDeviceGL2, &RenderDeviceGL2::createGPUTimer >( this );
	_delegate_commitStates.bind< RenderDeviceGL2, &RenderDeviceGL2::commitStates >( this );
	_delegate_resetStates.bind< RenderDeviceGL2, &RenderDeviceGL2::resetStates >( this );
	_delegate_clear.bind< RenderDeviceGL2, &RenderDeviceGL2::clear >( this );

	_delegate_draw.bind< RenderDeviceGL2, &RenderDeviceGL2::draw >( this );
	_delegate_drawIndexed.bind< RenderDeviceGL2, &RenderDeviceGL2::drawIndexed >( this );
	_delegate_setStorageBuffer.bind< RenderDeviceGL2, &RenderDeviceGL2::setStorageBuffer >( this );
}


void RenderDeviceGL2::initStates()
{
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	GLint value;
	glGetIntegerv( GL_SAMPLE_BUFFERS, &value );
	_defaultFBOMultisampled = value > 0;
	GLboolean doubleBuffered;
	glGetBooleanv( GL_DOUBLEBUFFER, &doubleBuffered );
	_doubleBuffered = doubleBuffered != 0;
	// Get the currently bound frame buffer object to avoid reset to invalid FBO
	glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &_defaultFBO );
}


bool RenderDeviceGL2::init()
{
	bool failed = false;

	char *vendor = (char *)glGetString( GL_VENDOR );
	char *renderer = (char *)glGetString( GL_RENDERER );
	char *version = (char *)glGetString( GL_VERSION );

	if( !version || !renderer || !vendor )
	{
		Modules::log().writeError("OpenGL not initialized. Make sure you have a valid OpenGL context");
		return false;
	}

	Modules::log().writeInfo( "Initializing GL2 backend using OpenGL driver '%s' by '%s' on '%s'",
	                          version, vendor, renderer );
	
	// Init extensions
	if( !initOpenGLExtensions(true) )
	{	
		Modules::log().writeError( "Could not find all required OpenGL function entry points" );
		failed = true;
	}

	// Check that OpenGL 2.0 is available
	if( glExt::majorVersion * 10 + glExt::minorVersion < 20 )
	{
		Modules::log().writeError( "OpenGL 2.0 not available" );
		failed = true;
	}
	
	// Check that required extensions are supported
	if( !glExt::EXT_framebuffer_object )
	{
		Modules::log().writeError( "Extension EXT_framebuffer_object not supported" );
		failed = true;
	}
	if( !glExt::EXT_texture_filter_anisotropic )
	{
		Modules::log().writeError( "Extension EXT_texture_filter_anisotropic not supported" );
		failed = true;
	}
	if( !glExt::EXT_texture_compression_s3tc )
	{
		Modules::log().writeError( "Extension EXT_texture_compression_s3tc not supported" );
		failed = true;
	}
	if( !glExt::EXT_texture_sRGB )
	{
		Modules::log().writeError( "Extension EXT_texture_sRGB not supported" );
		failed = true;
	}
	if ( !glExt::ARB_texture_rg )
	{
		Modules::log().writeError( "Extension ARB_texture_rg not supported" );
		failed = true;
	}
	
	if( failed )
	{
		Modules::log().writeError( "Failed to init renderer backend, debug info following" );
		char *exts = (char *)glGetString( GL_EXTENSIONS );
		Modules::log().writeInfo( "Supported extensions: '%s'", exts );

		return false;
	}
	
	// Get capabilities
	_caps.texFloat = glExt::ARB_texture_float ? 1 : 0;
	_caps.texNPOT = glExt::ARB_texture_non_power_of_two ? 1 : 0;
	_caps.rtMultisampling = glExt::EXT_framebuffer_multisample ? 1 : 0;
	_caps.geometryShaders = false;
	_caps.tesselation = false;
	_caps.computeShaders = false;
	_caps.instancing = false;
	_caps.maxJointCount = 75;
	_caps.maxTexUnitCount = 16;
	_caps.texDXT = true;
	_caps.texETC2 = false;
	_caps.texBPTC = glExt::ARB_texture_compression_bptc;
	_caps.texASTC = false;

	// Init states before creating test render buffer, to
	// ensure binding the current FBO again
	initStates();

	// Find supported depth format (some old ATI cards only support 16 bit depth for FBOs)
	_depthFormat = GL_DEPTH_COMPONENT24;
	uint32 testBuf = createRenderBuffer( 32, 32, TextureFormats::BGRA8, true, 1, 0, false ); 
	if( testBuf == 0 )
	{	
		_depthFormat = GL_DEPTH_COMPONENT16;
		Modules::log().writeWarning( "Render target depth precision limited to 16 bit" );
	}
	else
		destroyRenderBuffer( testBuf );
	
	resetStates();

	return true;
}


bool RenderDeviceGL2::enableDebugOutput()
{
	return true;
}


bool RenderDeviceGL2::disableDebugOutput()
{
	return true;
}

// =================================================================================================
// Vertex layouts
// =================================================================================================

uint32 RenderDeviceGL2::registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs )
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

void RenderDeviceGL2::beginRendering()
{	
	//	Get the currently bound frame buffer object. 
	glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &_defaultFBO );
	resetStates();
}

uint32 RenderDeviceGL2::beginCreatingGeometry( uint32 vlObj )
{
	uint32 idx = _geometryInfo.add( RDIGeometryInfoGL2() );
	RDIGeometryInfoGL2 &geo = _geometryInfo.getRef( idx );

	geo.layout = vlObj;

	return idx;
}

void RenderDeviceGL2::finishCreatingGeometry( uint32 geoObj )
{
	H3D_UNUSED_VAR( geoObj );
}

void RenderDeviceGL2::setGeomVertexParams( uint32 geoObj, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride )
{
	RDIGeometryInfoGL2 &geo = _geometryInfo.getRef( geoObj );
	RDIBufferGL2 &buf = _buffers.getRef( vbo );

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
		RDIVertBufSlotGL2 attribInfo( vbo, offset, stride );
		geo.vertexBufInfo.push_back( attribInfo );
	}
}

void RenderDeviceGL2::setGeomIndexParams( uint32 geoObj, uint32 indBuf, RDIIndexFormat format )
{
	RDIGeometryInfoGL2 &geo = _geometryInfo.getRef( geoObj );
	RDIBufferGL2 &buf = _buffers.getRef( indBuf );

	buf.geometryRefCount++;
	geo.indexBufIdx = indBuf;
	geo.indexBuf32Bit = format == IDXFMT_32 ? true : false;
}

void RenderDeviceGL2::destroyGeometry( uint32& geoObj, bool destroyBindedBuffers )
{
	if ( geoObj == 0 )
		return;

	RDIGeometryInfoGL2 &geo = _geometryInfo.getRef( geoObj );
	
	if ( destroyBindedBuffers )
	{
		for ( unsigned int i = 0; i < geo.vertexBufInfo.size(); ++i )
		{
			decreaseBufferRefCount( geo.vertexBufInfo[ i ].vbObj );
			destroyBuffer( geo.vertexBufInfo[ i ].vbObj );
		}

		decreaseBufferRefCount( geo.indexBufIdx );
		destroyBuffer( geo.indexBufIdx );
	}
	else
	{
		// just decrease reference count
		for ( unsigned int i = 0; i < geo.vertexBufInfo.size(); ++i )
		{
			decreaseBufferRefCount( geo.vertexBufInfo[ i ].vbObj );
		}

		decreaseBufferRefCount( geo.indexBufIdx );
	}
	
	_geometryInfo.remove( geoObj );
	geoObj = 0;
}


void RenderDeviceGL2::decreaseBufferRefCount( uint32 bufObj )
{
	if ( bufObj == 0 ) return;

	RDIBufferGL2 &buf = _buffers.getRef( bufObj );

	buf.geometryRefCount--;
}


uint32 RenderDeviceGL2::createVertexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ARRAY_BUFFER, size, data );
}


uint32 RenderDeviceGL2::createIndexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ELEMENT_ARRAY_BUFFER, size, data );;
}


uint32 RenderDeviceGL2::createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data )
{
	RDITextureBufferGL2 buf;

	buf.bufObj = createBuffer( GL_TEXTURE_BUFFER_ARB, bufSize, data );

	glGenTextures( 1, &buf.glTexID );
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( GL_TEXTURE_BUFFER_ARB, buf.glTexID );

	switch ( format )
	{
		case TextureFormats::BGRA8:
			buf.glFmt = GL_RGBA8;
			break;
		case TextureFormats::RGBA16F:
			buf.glFmt = GL_RGBA16F_ARB;
			break;
		case TextureFormats::RGBA32F:
			buf.glFmt = GL_RGBA32F_ARB;
			break;
		case TextureFormats::R32F:
			buf.glFmt = GL_R32F_ARB;
			break;
		case TextureFormats::RG32F:
			buf.glFmt = GL_RG32F_ARB;
			break;
		default:
			ASSERT( 0 );
			break;
	};

	// bind texture to buffer
	glTexBufferARB( GL_TEXTURE_BUFFER_ARB, buf.glFmt, _buffers.getRef( buf.bufObj ).glObj );

	glBindTexture( GL_TEXTURE_BUFFER_ARB, 0 );
	if ( _texSlots[ 15 ].texObj )
		glBindTexture( _textures.getRef( _texSlots[ 15 ].texObj ).type, _textures.getRef( _texSlots[ 15 ].texObj ).glObj );

	return _textureBuffs.add( buf );
}


uint32 RenderDeviceGL2::createShaderStorageBuffer( uint32 size, const void *data )
{
	H3D_UNUSED_VAR( size );
	H3D_UNUSED_VAR( data );

	Modules::log().writeError( "Shader storage buffers are not supported on OpenGL 2 devices." );

	return 0;
}


uint32 RenderDeviceGL2::createBuffer( uint32 bufType, uint32 size, const void *data )
{
	RDIBufferGL2 buf;

	buf.type = bufType;
	buf.size = size;
	glGenBuffers( 1, &buf.glObj );
	glBindBuffer( buf.type, buf.glObj );
	glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );
	glBindBuffer( buf.type, 0 );
	
	_bufferMem += size;
	return _buffers.add( buf );
}


void RenderDeviceGL2::destroyBuffer( uint32& bufObj )
{
	if( bufObj == 0 )
		return;
	
	RDIBufferGL2 &buf = _buffers.getRef( bufObj );

	if ( buf.geometryRefCount < 1 )
	{
		glDeleteBuffers( 1, &buf.glObj );

		_bufferMem -= buf.size;
		_buffers.remove( bufObj );
		bufObj = 0;
	}
}


void RenderDeviceGL2::destroyTextureBuffer( uint32& bufObj )
{
	
}


void RenderDeviceGL2::updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data )
{
	const RDIBufferGL2 &buf = _buffers.getRef( bufObj );
	ASSERT( offset + size <= buf.size );
	
	glBindBuffer( buf.type, buf.glObj );
	
	if( offset == 0 && size == buf.size )
	{
		// Replacing the whole buffer can help the driver to avoid pipeline stalls
		glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );
		return;
	}

	glBufferSubData( buf.type, offset, size, data );
}


void * RenderDeviceGL2::mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType )
{
	const RDIBufferGL2 &buf = _buffers.getRef( bufObj );
	ASSERT( offset + size <= buf.size );

	glBindBuffer( buf.type, buf.glObj );

	return glMapBuffer( buf.type, bufferMappingTypes[ mapType ] );
}


void RenderDeviceGL2::unmapBuffer( uint32 geoObj, uint32 bufObj )
{
	const RDIBufferGL2 &buf = _buffers.getRef( bufObj );

	// multiple buffers can be mapped at the same time, so bind the one that needs to be unmapped
	glBindBuffer( buf.type, buf.glObj );

	glUnmapBuffer( buf.type );
}


// =================================================================================================
// Textures
// =================================================================================================

uint32 RenderDeviceGL2::createTexture( TextureTypes::List type, int width, int height, int depth,
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
	
	RDITextureGL2 tex;
	tex.type = textureTypes[ type ];
	tex.format = format;
	tex.width = width;
	tex.height = height;
	tex.depth = depth;
	tex.sRGB = sRGB && Modules::config().sRGBLinearization;
	tex.genMips = genMips;
	tex.hasMips = maxMipLevel > 0;

	if ( format > ( int ) textureGLFormats.size() ) { ASSERT( 0 ); return 0; }

	tex.glFmt = format != TextureFormats::DEPTH
				? ( tex.sRGB ? textureGLFormats[ format ].glSRGBFormat : textureGLFormats[ format ].glCreateFormat )
				: _depthFormat;

	glGenTextures( 1, &tex.glObj );
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );

	glTexParameteri( tex.type, GL_TEXTURE_MAX_LEVEL, maxMipLevel );

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv( tex.type, GL_TEXTURE_BORDER_COLOR, borderColor );
	
	tex.samplerState = 0;
	applySamplerState( tex );
	
	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );

	// Calculate memory requirements
	tex.memSize = calcTextureSize( format, width, height, depth, maxMipLevel );
	if( type == TextureTypes::TexCube ) tex.memSize *= 6;
	_textureMem += tex.memSize;
	
	return _textures.add( tex );
}


void RenderDeviceGL2::generateTextureMipmap( uint32 texObj )
{
	const RDITextureGL2 &tex = _textures.getRef( texObj );

	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	glGenerateMipmapEXT( tex.type );
	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
}


void RenderDeviceGL2::uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	ASSERT( pixels );
	const RDITextureGL2 &tex = _textures.getRef( texObj );
	TextureFormats::List format = tex.format;

	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	
	bool compressed = isCompressedTextureFormat( format );

	int inputType = textureGLFormats[ format ].glInputType;
	int inputFormat = textureGLFormats[ format ].glInputFormat;
	
	// Calculate size of next mipmap using "floor" convention
	int width = std::max( tex.width >> mipLevel, 1 ), height = std::max( tex.height >> mipLevel, 1 );
	
	if( tex.type == textureTypes[ TextureTypes::Tex2D ] || tex.type == textureTypes[ TextureTypes::TexCube ] )
	{
		int target = ( tex.type == textureTypes[ TextureTypes::Tex2D ] ) ?
			GL_TEXTURE_2D : ( GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice );
		
		if( compressed )
			glCompressedTexImage2D( target, mipLevel, tex.glFmt, width, height, 0,
			                        calcTextureSize( format, width, height, 1 ), pixels );
		else
			glTexImage2D( target, mipLevel, tex.glFmt, width, height, 0, inputFormat, inputType, pixels );
	}
	else if ( tex.type == textureTypes[ TextureTypes::Tex3D ] )
	{
		int depth = std::max( tex.depth >> mipLevel, 1 );
		
		if( compressed )
			glCompressedTexImage3D( GL_TEXTURE_3D, mipLevel, tex.glFmt, width, height, depth, 0,
			                        calcTextureSize( format, width, height, depth ), pixels );	
		else
			glTexImage3D( GL_TEXTURE_3D, mipLevel, tex.glFmt, width, height, depth, 0,
			              inputFormat, inputType, pixels );
	}

	if( tex.genMips && (tex.type != GL_TEXTURE_CUBE_MAP || slice == 5) )
	{
		// Note: for cube maps mips are only generated when the side with the highest index is uploaded
		glEnable( tex.type );  // Workaround for ATI driver bug
		glGenerateMipmapEXT( tex.type );
		glDisable( tex.type );
	}

	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
}


void RenderDeviceGL2::destroyTexture( uint32& texObj )
{
	if( texObj == 0 )
		return;
	
	const RDITextureGL2 &tex = _textures.getRef( texObj );
	if( tex.glObj ) glDeleteTextures( 1, &tex.glObj );

	_textureMem -= tex.memSize;
	_textures.remove( texObj );
	texObj = 0;
}


bool RenderDeviceGL2::isCompressedTextureFormat( TextureFormats::List fmt )
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


void RenderDeviceGL2::updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	uploadTextureData( texObj, slice, mipLevel, pixels );
}


bool RenderDeviceGL2::getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer )
{
	const RDITextureGL2 &tex = _textures.getRef( texObj );
	
	int target = tex.type == textureTypes[ TextureTypes::TexCube ] ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
	if( target == GL_TEXTURE_CUBE_MAP ) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;
	
	int fmt, type = 0;
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	
	bool compressed = isCompressedTextureFormat( tex.format );

	if ( tex.format > ( int ) textureGLFormats.size() ) return false;

	fmt = textureGLFormats[ tex.format ].glInputFormat;
	type = textureGLFormats[ tex.format ].glInputType;

	if( compressed )
		glGetCompressedTexImage( target, mipLevel, buffer );
	else
		glGetTexImage( target, mipLevel, fmt, type, buffer );

	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );

	return true;
}

void RenderDeviceGL2::bindImageToTexture(uint32 texObj, void *eglImage)
{
	if( !glExt::OES_EGL_image )
		Modules::log().writeError("OES_egl_image not supported");
	else
	{
		const RDITextureGL2 &tex = _textures.getRef( texObj );
		glActiveTexture( GL_TEXTURE15 );
		glBindTexture( tex.type, tex.glObj );
		glEGLImageTargetTexture2DOES( tex.type, eglImage );
		glBindTexture( tex.type, 0 );
		if( _texSlots[15].texObj )
			glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
	}
}

// =================================================================================================
// Shaders
// =================================================================================================

uint32 RenderDeviceGL2::createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc )
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog = 0x0;
	int status;

	_shaderLog = "";

	// Vertex shader
	uint32 vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, &vertexShaderSrc, 0x0 );
	glCompileShader( vs );
	glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
	if( !status )
	{	
		// Get info
		glGetShaderiv( vs, GL_INFO_LOG_LENGTH, &infologLength );
		if( infologLength > 1 )
		{
			infoLog = new char[infologLength];
			glGetShaderInfoLog( vs, infologLength, &charsWritten, infoLog );
			_shaderLog = _shaderLog + "[Vertex Shader]\n" + infoLog;
			delete[] infoLog; infoLog = 0x0;
		}

		glDeleteShader( vs );
		return 0;
	}

	// Fragment shader
	uint32 fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &fragmentShaderSrc, 0x0 );
	glCompileShader( fs );
	glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
	if( !status )
	{	
		glGetShaderiv( fs, GL_INFO_LOG_LENGTH, &infologLength );
		if( infologLength > 1 )
		{
			infoLog = new char[infologLength];
			glGetShaderInfoLog( fs, infologLength, &charsWritten, infoLog );
			_shaderLog = _shaderLog + "[Fragment Shader]\n" + infoLog;
			delete[] infoLog; infoLog = 0x0;
		}

		glDeleteShader( vs );
		glDeleteShader( fs );
		return 0;
	}

	// Shader program
	uint32 program = glCreateProgram();
	glAttachShader( program, vs );
	glAttachShader( program, fs );
	glDeleteShader( vs );
	glDeleteShader( fs );

	return program;
}


bool RenderDeviceGL2::linkShaderProgram( uint32 programObj )
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog = 0x0;
	int status;

	_shaderLog = "";
	
	glLinkProgram( programObj );
	glGetProgramiv( programObj, GL_INFO_LOG_LENGTH, &infologLength );
	if( infologLength > 1 )
	{
		infoLog = new char[infologLength];
		glGetProgramInfoLog( programObj, infologLength, &charsWritten, infoLog );
		_shaderLog = _shaderLog + "[Linking]\n" + infoLog;
		delete[] infoLog; infoLog = 0x0;
	}
	
	glGetProgramiv( programObj, GL_LINK_STATUS, &status );
	if( !status ) return false;

	return true;
}


uint32 RenderDeviceGL2::createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
                                      const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc )
{
	H3D_UNUSED_VAR( geometryShaderSrc );
	H3D_UNUSED_VAR( tessControlShaderSrc );
	H3D_UNUSED_VAR( tessEvaluationShaderSrc );
	H3D_UNUSED_VAR( computeShaderSrc );

	// Compile and link shader
	uint32 programObj = createShaderProgram( vertexShaderSrc, fragmentShaderSrc );
	if( programObj == 0 ) return 0;
	if( !linkShaderProgram( programObj ) ) return 0;
	
	uint32 shaderId = _shaders.add( RDIShaderGL2() );
	RDIShaderGL2 &shader = _shaders.getRef( shaderId );
	shader.oglProgramObj = programObj;
	
	int attribCount;
	glGetProgramiv( programObj, GL_ACTIVE_ATTRIBUTES, &attribCount );
	
	for( uint32 i = 0; i < _numVertexLayouts; ++i )
	{
		RDIVertexLayout &vl = _vertexLayouts[i];
		bool allAttribsFound = true;
		
		for( uint32 j = 0; j < 16; ++j )
			shader.inputLayouts[i].attribIndices[j] = -1;
		
		for( int j = 0; j < attribCount; ++j )
		{
			char name[32];
			uint32 size, type;
			glGetActiveAttrib( programObj, j, 32, 0x0, (int *)&size, &type, name );

			bool attribFound = false;
			for( uint32 k = 0; k < vl.numAttribs; ++k )
			{
				if( vl.attribs[k].semanticName.compare(name) == 0 )
				{
					shader.inputLayouts[i].attribIndices[k] = glGetAttribLocation( programObj, name );
					attribFound = true;
				}
			}

			if( !attribFound )
			{
				allAttribsFound = false;
				break;
			}
		}

		shader.inputLayouts[i].valid = allAttribsFound;
	}

	return shaderId;
}


void RenderDeviceGL2::destroyShader( uint32& shaderId )
{
	if( shaderId == 0 )
		return;

	RDIShaderGL2 &shader = _shaders.getRef( shaderId );
	glDeleteProgram( shader.oglProgramObj );
	_shaders.remove( shaderId );
	shaderId = 0;
}


void RenderDeviceGL2::bindShader( uint32 shaderId )
{
	if( shaderId != 0 )
	{
		RDIShaderGL2 &shader = _shaders.getRef( shaderId );
		glUseProgram( shader.oglProgramObj );
	}
	else
	{
		glUseProgram( 0 );
	}
	
	_curShaderId = shaderId;
	_pendingMask |= PM_GEOMETRY;
} 


int RenderDeviceGL2::getShaderConstLoc( uint32 shaderId, const char *name )
{
	RDIShaderGL2 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


int RenderDeviceGL2::getShaderSamplerLoc( uint32 shaderId, const char *name )
{
	RDIShaderGL2 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


int RenderDeviceGL2::getShaderBufferLoc( uint32 shaderId, const char *name )
{
	H3D_UNUSED_VAR( shaderId );
	H3D_UNUSED_VAR( name );

	// Not supported on OpenGL 2
	return -1;
}

void RenderDeviceGL2::setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count )
{
	switch( type )
	{
	case CONST_FLOAT:
		glUniform1fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT2:
		glUniform2fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT3:
		glUniform3fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT4:
		glUniform4fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT44:
		glUniformMatrix4fv( loc, count, false, (float *)values );
		break;
	case CONST_FLOAT33:
		glUniformMatrix3fv( loc, count, false, (float *)values );
		break;
	case CONST_INT:
		glUniform1iv( loc, count, (GLint *)values );
		break;
	case CONST_INT2:
		glUniform2iv( loc, count, (GLint *)values );
		break;
	case CONST_INT3:
		glUniform3iv( loc, count, (GLint *)values );
		break;
	case CONST_INT4:
		glUniform4iv( loc, count, (GLint *)values );
		break;
	}
}


void RenderDeviceGL2::setShaderSampler( int loc, uint32 texUnit )
{
	glUniform1i( loc, (int)texUnit );
}


const char *RenderDeviceGL2::getDefaultVSCode()
{
	return defaultShaderVS;
}


const char *RenderDeviceGL2::getDefaultFSCode()
{
	return defaultShaderFS;
}


void RenderDeviceGL2::runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim )
{
	H3D_UNUSED_VAR( shaderId );
	H3D_UNUSED_VAR( xDim );
	H3D_UNUSED_VAR( yDim );
	H3D_UNUSED_VAR( zDim );

	Modules::log().writeError( "Compute shaders are not supported on OpenGL 2 render device." );
}

// =================================================================================================
// Renderbuffers
// =================================================================================================

uint32 RenderDeviceGL2::createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
                                            bool depth, uint32 numColBufs, uint32 samples, uint32 maxMipLevel )
{
	if( (format == TextureFormats::RGBA16F || format == TextureFormats::RGBA32F) && !_caps.texFloat )
	{
		return 0;
	}

	if( numColBufs > RDIRenderBufferGL2::MaxColorAttachmentCount ) return 0;

	uint32 maxSamples = 0;
	if( _caps.rtMultisampling )
	{
		GLint value;
		glGetIntegerv( GL_MAX_SAMPLES_EXT, &value );
		maxSamples = (uint32)value;
	}
	if( samples > maxSamples )
	{
		samples = maxSamples;
		Modules::log().writeWarning( "GPU does not support desired multisampling quality for render target" );
	}

	RDIRenderBufferGL2 rb;
	rb.width = width;
	rb.height = height;
	rb.samples = samples;

	// Create framebuffers
	glGenFramebuffersEXT( 1, &rb.fbo );
	if( samples > 0 ) glGenFramebuffersEXT( 1, &rb.fboMS );

	if( numColBufs > 0 )
	{
		// Attach color buffers
		for( uint32 j = 0; j < numColBufs; ++j )
		{
			// Create a color texture
			uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, format, maxMipLevel, maxMipLevel > 0, false, false );
			ASSERT( texObj != 0 );
			rb.colTexs[j] = texObj;
			RDITextureGL2 &tex = _textures.getRef( texObj );
			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fbo );
			// Attach the texture
			glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + j, GL_TEXTURE_2D, tex.glObj, 0 );

			if( samples > 0 )
			{
				glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fboMS );
				// Create a multisampled renderbuffer
				glGenRenderbuffersEXT( 1, &rb.colBufs[j] );
				glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, rb.colBufs[j] );
				glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER_EXT, rb.samples, tex.glFmt, rb.width, rb.height );
				// Attach the renderbuffer
				glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + j,
				                              GL_RENDERBUFFER_EXT, rb.colBufs[j] );
			}
		}

		uint32 buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT,
		                     GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fbo );
		glDrawBuffers( numColBufs, buffers );
		
		if( samples > 0 )
		{
			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fboMS );
			glDrawBuffers( numColBufs, buffers );
		}
	}
	else
	{	
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fbo );
		glDrawBuffer( GL_NONE );
		glReadBuffer( GL_NONE );
		
		if( samples > 0 )
		{
			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fboMS );
			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );
		}
	}

	// Attach depth buffer
	if( depth )
	{
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fbo );
		// Create a depth texture
		uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, TextureFormats::DEPTH, 0, false, false, false );
		ASSERT( texObj != 0 );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
		rb.depthTex = texObj;
		RDITextureGL2 &tex = _textures.getRef( texObj );
		// Attach the texture
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex.glObj, 0 );

		if( samples > 0 )
		{
			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fboMS );
			// Create a multisampled renderbuffer
			glGenRenderbuffersEXT( 1, &rb.depthBuf );
			glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, rb.depthBuf );
			glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER_EXT, rb.samples, _depthFormat, rb.width, rb.height );
			// Attach the renderbuffer
			glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			                              GL_RENDERBUFFER_EXT, rb.depthBuf );
		}
	}

	uint32 rbObj = _rendBufs.add( rb );
	
	// Check if FBO is complete
	bool valid = true;
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fbo );
	uint32 status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _defaultFBO );
	if( status != GL_FRAMEBUFFER_COMPLETE_EXT ) valid = false;
	
	if( samples > 0 )
	{
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fboMS );
		status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _defaultFBO );
		if( status != GL_FRAMEBUFFER_COMPLETE_EXT ) valid = false;
	}

	if( !valid )
	{
		destroyRenderBuffer( rbObj );
		return 0;
	}
	
	return rbObj;
}


void RenderDeviceGL2::destroyRenderBuffer( uint32& rbObj )
{
	RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );
	
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _defaultFBO );
	
	if( rb.depthTex != 0 ) destroyTexture( rb.depthTex );
	if( rb.depthBuf != 0 ) glDeleteRenderbuffersEXT( 1, &rb.depthBuf );
	rb.depthTex = rb.depthBuf = 0;
		
	for( uint32 i = 0; i < RDIRenderBufferGL2::MaxColorAttachmentCount; ++i )
	{
		if( rb.colTexs[i] != 0 ) destroyTexture( rb.colTexs[i] );
		if( rb.colBufs[i] != 0 ) glDeleteRenderbuffersEXT( 1, &rb.colBufs[i] );
		rb.colTexs[i] = rb.colBufs[i] = 0;
	}

	if( rb.fbo != 0 ) glDeleteFramebuffersEXT( 1, &rb.fbo );
	if( rb.fboMS != 0 ) glDeleteFramebuffersEXT( 1, &rb.fboMS );
	rb.fbo = rb.fboMS = 0;

	_rendBufs.remove( rbObj );
	rbObj = 0;
}


uint32 RenderDeviceGL2::getRenderBufferTex( uint32 rbObj, uint32 bufIndex )
{
	RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );
	
	if( bufIndex < RDIRenderBufferGL2::MaxColorAttachmentCount ) return rb.colTexs[bufIndex];
	else if( bufIndex == 32 ) return rb.depthTex;
	else return 0;
}

void RenderDeviceGL2::getRenderBufferDimensions( uint32 rbObj, int *width, int *height )
{
	RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );
	
	*width = rb.width;
	*height = rb.height;
}

void RenderDeviceGL2::resolveRenderBuffer( uint32 rbObj )
{
	RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );
	
	if( rb.fboMS == 0 ) return;
	
	glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, rb.fboMS );
	glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, rb.fbo );

	bool depthResolved = false;
	for( uint32 i = 0; i < RDIRenderBufferGL2::MaxColorAttachmentCount; ++i )
	{
		if( rb.colBufs[i] != 0 )
		{
			glReadBuffer( GL_COLOR_ATTACHMENT0_EXT + i );
			glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT + i );
			
			int mask = GL_COLOR_BUFFER_BIT;
			if( !depthResolved && rb.depthBuf != 0 )
			{
				mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
				depthResolved = true;
			}
			glBlitFramebufferEXT( 0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height, mask, GL_NEAREST );
		}
	}

	if( !depthResolved && rb.depthBuf != 0 )
	{
		glReadBuffer( GL_NONE );
		glDrawBuffer( GL_NONE );
		glBlitFramebufferEXT( 0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height,
							  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST );
	}

	glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, _defaultFBO );
	glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, _defaultFBO );
}


void RenderDeviceGL2::setRenderBuffer( uint32 rbObj )
{
	// Resolve render buffer if necessary
	if( _curRendBuf != 0 ) resolveRenderBuffer( _curRendBuf );
	
	// Set new render buffer
	_curRendBuf = rbObj;
	
	if( rbObj == 0 )
	{
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _defaultFBO );
		if( _defaultFBO == 0 )
		{
			if( _doubleBuffered )
				glDrawBuffer( _outputBufferIndex == 1 ? GL_BACK_RIGHT : GL_BACK_LEFT );
			else
				glDrawBuffer( _outputBufferIndex == 1 ? GL_FRONT_RIGHT : GL_FRONT_LEFT );
		}
		_fbWidth = _vpWidth + _vpX;
		_fbHeight = _vpHeight + _vpY;
		if( _defaultFBOMultisampled ) glEnable( GL_MULTISAMPLE );
		else glDisable( GL_MULTISAMPLE );
	}
	else
	{
		// Unbind all textures to make sure that no FBO attachment is bound any more
		for( uint32 i = 0; i < 16; ++i ) setTexture( i, 0, 0, 0 );
		commitStates( PM_TEXTURES );
		
		RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );

		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fboMS != 0 ? rb.fboMS : rb.fbo );
		ASSERT( glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) == GL_FRAMEBUFFER_COMPLETE_EXT );
		_fbWidth = rb.width;
		_fbHeight = rb.height;

		if( rb.fboMS != 0 ) glEnable( GL_MULTISAMPLE );
		else glDisable( GL_MULTISAMPLE );
	}
}


bool RenderDeviceGL2::getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
                                        int *compCount, void *dataBuffer, int bufferSize )
{
	int x, y, w, h;
	int format = GL_RGBA;
	int type = GL_FLOAT;
	beginRendering();
	glPixelStorei( GL_PACK_ALIGNMENT, 4 );
	
	if( rbObj == 0 )
	{
		if( bufIndex != 32 && bufIndex != 0 ) return false;
		if( width != 0x0 ) *width = _vpWidth;
		if( height != 0x0 ) *height = _vpHeight;
		
		x = _vpX; y = _vpY; w = _vpWidth; h = _vpHeight;

		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _defaultFBO );
		if( bufIndex != 32 )
		{
			if( _doubleBuffered )
				glReadBuffer( _outputBufferIndex == 1 ? GL_BACK_RIGHT : GL_BACK_LEFT );
			else
				glReadBuffer( _outputBufferIndex == 1 ? GL_FRONT_RIGHT : GL_FRONT_LEFT );
		}
		//format = GL_BGRA;
		//type = GL_UNSIGNED_BYTE;
	}
	else
	{
		resolveRenderBuffer( rbObj );
		RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );
		
		if( bufIndex == 32 && rb.depthTex == 0 ) return false;
		if( bufIndex != 32 )
		{
			if( (unsigned)bufIndex >= RDIRenderBufferGL2::MaxColorAttachmentCount || rb.colTexs[bufIndex] == 0 )
				return false;
		}
		if( width != 0x0 ) *width = rb.width;
		if( height != 0x0 ) *height = rb.height;

		x = 0; y = 0; w = rb.width; h = rb.height;
		
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, rb.fbo );
		if( bufIndex != 32 ) glReadBuffer( GL_COLOR_ATTACHMENT0_EXT + bufIndex );
	}

	if( bufIndex == 32 )
	{	
		format = GL_DEPTH_COMPONENT;
		type = GL_FLOAT;
	}
	
	int comps = (bufIndex == 32 ? 1 : 4);
	if( compCount != 0x0 ) *compCount = comps;
	
	bool retVal = false;
	if( dataBuffer != 0x0 &&
	    bufferSize >= w * h * comps * (type == GL_FLOAT ? 4 : 1) ) 
	{
		glFinish();
		glReadPixels( x, y, w, h, format, type, dataBuffer );
		retVal = true;
	}
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _defaultFBO );

	return retVal;
}


// =================================================================================================
// Queries
// =================================================================================================

uint32 RenderDeviceGL2::createOcclusionQuery()
{
	uint32 queryObj;
	glGenQueries( 1, &queryObj );
	return queryObj;
}


void RenderDeviceGL2::destroyQuery( uint32 queryObj )
{
	if( queryObj == 0 ) return;
	
	glDeleteQueries( 1, &queryObj );
}


void RenderDeviceGL2::beginQuery( uint32 queryObj )
{
	glBeginQuery( GL_SAMPLES_PASSED, queryObj );
}


void RenderDeviceGL2::endQuery( uint32 /*queryObj*/ )
{
	glEndQuery( GL_SAMPLES_PASSED );
}


uint32 RenderDeviceGL2::getQueryResult( uint32 queryObj )
{
	uint32 samples = 0;
	glGetQueryObjectuiv( queryObj, GL_QUERY_RESULT, &samples );
	return samples;
}


// =================================================================================================
// Internal state management
// =================================================================================================

void RenderDeviceGL2::checkError()
{
#if !defined( NDEBUG )
	uint32 error = glGetError();
	ASSERT( error != GL_INVALID_ENUM );
	ASSERT( error != GL_INVALID_VALUE );
	ASSERT( error != GL_INVALID_OPERATION );
	ASSERT( error != GL_OUT_OF_MEMORY );
	ASSERT( error != GL_STACK_OVERFLOW && error != GL_STACK_UNDERFLOW );
#endif
}


void RenderDeviceGL2::setStorageBuffer( uint8 slot, uint32 bufObj )
{
	H3D_UNUSED_VAR( slot );
	H3D_UNUSED_VAR( bufObj );
}


bool RenderDeviceGL2::applyVertexLayout( const RDIGeometryInfoGL2 &geo )
{
	uint32 newVertexAttribMask = 0;
	
	if( geo.layout != 0 )
	{
		if( _curShaderId == 0 ) return false;
		
		RDIVertexLayout &vl = _vertexLayouts[ geo.layout - 1];
		RDIShaderGL2 &shader = _shaders.getRef( _curShaderId );
		RDIInputLayoutGL2 &inputLayout = shader.inputLayouts[ geo.layout - 1];
		
		if( !inputLayout.valid )
			return false;

		// Set vertex attrib pointers
		for( uint32 i = 0; i < vl.numAttribs; ++i )
		{
			int8 attribIndex = inputLayout.attribIndices[i];
			if( attribIndex >= 0 )
			{
				VertexLayoutAttrib &attrib = vl.attribs[i];
				const RDIVertBufSlotGL2 &vbSlot = geo.vertexBufInfo[ attrib.vbSlot ];
				
				ASSERT( _buffers.getRef( geo.vertexBufInfo[ attrib.vbSlot ].vbObj ).glObj != 0 &&
						_buffers.getRef( geo.vertexBufInfo[ attrib.vbSlot ].vbObj ).type == GL_ARRAY_BUFFER );
				
				glBindBuffer( GL_ARRAY_BUFFER, _buffers.getRef( geo.vertexBufInfo[ attrib.vbSlot ].vbObj ).glObj );
				glVertexAttribPointer( attribIndex, attrib.size, GL_FLOAT, GL_FALSE,
									   vbSlot.stride, (char *)0 + vbSlot.offset + attrib.offset );

				newVertexAttribMask |= 1 << attribIndex;
			}
		}
	}
	
	for( uint32 i = 0; i < 16; ++i )
	{
		uint32 curBit = 1 << i;
		if( (newVertexAttribMask & curBit) != (_activeVertexAttribsMask & curBit) )
		{
			if( newVertexAttribMask & curBit ) glEnableVertexAttribArray( i );
			else glDisableVertexAttribArray( i );
		}
	}
	_activeVertexAttribsMask = newVertexAttribMask;

	return true;
}


void RenderDeviceGL2::applySamplerState( RDITextureGL2 &tex )
{
	uint32 state = tex.samplerState;
	uint32 target = tex.type;
	
	const uint32 magFilters[] = { GL_LINEAR, GL_LINEAR, GL_NEAREST };
	const uint32 minFiltersMips[] = { GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST };
	const uint32 maxAniso[] = { 1, 2, 4, 0, 8, 0, 0, 0, 16 };
	const uint32 wrapModes[] = { GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_BORDER };

	if( tex.hasMips )
		glTexParameteri( target, GL_TEXTURE_MIN_FILTER, minFiltersMips[(state & SS_FILTER_MASK) >> SS_FILTER_START] );
	else
		glTexParameteri( target, GL_TEXTURE_MIN_FILTER, magFilters[(state & SS_FILTER_MASK) >> SS_FILTER_START] );

	glTexParameteri( target, GL_TEXTURE_MAG_FILTER, magFilters[(state & SS_FILTER_MASK) >> SS_FILTER_START] );
	glTexParameteri( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[(state & SS_ANISO_MASK) >> SS_ANISO_START] );
	glTexParameteri( target, GL_TEXTURE_WRAP_S, wrapModes[(state & SS_ADDRU_MASK) >> SS_ADDRU_START] );
	glTexParameteri( target, GL_TEXTURE_WRAP_T, wrapModes[(state & SS_ADDRV_MASK) >> SS_ADDRV_START] );
	glTexParameteri( target, GL_TEXTURE_WRAP_R, wrapModes[(state & SS_ADDRW_MASK) >> SS_ADDRW_START] );
	
	if( !(state & SS_COMP_LEQUAL) )
	{
		glTexParameteri( target, GL_TEXTURE_COMPARE_MODE, GL_NONE );
	}
	else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	}
}


void RenderDeviceGL2::applyRenderStates()
{
	// Rasterizer state
	if( _newRasterState.hash != _curRasterState.hash )
	{
		if( _newRasterState.fillMode == RS_FILL_SOLID ) glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		else glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		if( _newRasterState.cullMode == RS_CULL_BACK )
		{
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );
		}
		else if( _newRasterState.cullMode == RS_CULL_FRONT )
		{
			glEnable( GL_CULL_FACE );
			glCullFace( GL_FRONT );
		}
		else
		{
			glDisable( GL_CULL_FACE );
		}

		if( !_newRasterState.scissorEnable ) glDisable( GL_SCISSOR_TEST );
		else glEnable( GL_SCISSOR_TEST );

		if( _newRasterState.renderTargetWriteMask ) glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		else glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		
		_curRasterState.hash = _newRasterState.hash;
	}

	// Blend state
	if( _newBlendState.hash != _curBlendState.hash )
	{
		if( !_newBlendState.alphaToCoverageEnable ) glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );
		else glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );

		if( !_newBlendState.blendEnable )
		{
			glDisable( GL_BLEND );
		}
		else
		{
			uint32 oglBlendFuncs[ 16 ] = { GL_ZERO, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_DST_COLOR, GL_SRC_COLOR,
										   GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO };

			glEnable( GL_BLEND );
			glBlendFunc( oglBlendFuncs[_newBlendState.srcBlendFunc], oglBlendFuncs[_newBlendState.destBlendFunc] );
		}
		
		_curBlendState.hash = _newBlendState.hash;
	}

	// Depth-stencil state
	if( _newDepthStencilState.hash != _curDepthStencilState.hash )
	{
		if( _newDepthStencilState.depthWriteMask ) glDepthMask( GL_TRUE );
		else glDepthMask( GL_FALSE);

		if( _newDepthStencilState.depthEnable )
		{
			uint32 oglDepthFuncs[8] = { GL_LEQUAL, GL_LESS, GL_EQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS, GL_ALWAYS, GL_ALWAYS };
			
			glEnable( GL_DEPTH_TEST );
			glDepthFunc( oglDepthFuncs[_newDepthStencilState.depthFunc] );
		}
		else
		{
			glDisable( GL_DEPTH_TEST );
		}
		
		_curDepthStencilState.hash = _newDepthStencilState.hash;
	}
}


bool RenderDeviceGL2::commitStates( uint32 filter )
{
	if( _pendingMask & filter )
	{
		uint32 mask = _pendingMask & filter;
	
		// Set viewport
		if( mask & PM_VIEWPORT )
		{
			glViewport( _vpX, _vpY, _vpWidth, _vpHeight );
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
			glScissor( _scX, _scY, _scWidth, _scHeight );
			_pendingMask &= ~PM_SCISSOR;
		}
		
		// Bind index buffer
// 		if( mask & PM_INDEXBUF )
// 		{
// 			if( _newIndexBuf != _curIndexBuf )
// 			{
// 				if( _newIndexBuf != 0 )
// 					glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _buffers.getRef( _newIndexBuf ).glObj );
// 				else
// 					glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
// 				
// 				_curIndexBuf = _newIndexBuf;
// 				_pendingMask &= ~PM_INDEXBUF;
// 			}
// 		}
// 
		// Bind textures and set sampler state
		if( mask & PM_TEXTURES )
		{
			for( uint32 i = 0; i < 16; ++i )
			{
				glActiveTexture( GL_TEXTURE0 + i );

				if( _texSlots[i].texObj != 0 )
				{
					RDITextureGL2 &tex = _textures.getRef( _texSlots[i].texObj );
					glBindTexture( tex.type, tex.glObj );

					// Apply sampler state
					if( tex.samplerState != _texSlots[i].samplerState )
					{
						tex.samplerState = _texSlots[i].samplerState;
						applySamplerState( tex );
					}
				}
				else
				{
					glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
					glBindTexture( GL_TEXTURE_3D, 0 );
					glBindTexture( GL_TEXTURE_2D, 0 );
				}
			}
			
			_pendingMask &= ~PM_TEXTURES;
		}

		// Bind vertex buffers
		if( mask & PM_GEOMETRY )
		{
			//if( _newVertLayout != _curVertLayout || _curShader != _prevShader )
			{
				RDIGeometryInfoGL2 &geo = _geometryInfo.getRef( _curGeometryIndex );
				
				// index buffer mapping
				if ( geo.indexBufIdx != _curIndexBuf )
				{
					if ( geo.indexBufIdx != 0 )
					{
						RDIBufferGL2 &buf = _buffers.getRef( geo.indexBufIdx );
						ASSERT( buf.glObj != 0 )

						glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buf.glObj );
					}
					else
						glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
				}

				if( !applyVertexLayout( geo ) )
					return false;

// 				_curVertLayout = _newVertLayout;
				_indexFormat = geo.indexBuf32Bit;
				_curIndexBuf = geo.indexBufIdx;
				_prevShaderId = _curShaderId;
				_pendingMask &= ~PM_GEOMETRY;
			}
		}

		CHECK_GL_ERROR
	}

	return true;
}


void RenderDeviceGL2::resetStates()
{
 	_curIndexBuf = 1; 
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
	commitStates();

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
//	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _defaultFBO );
}


// =================================================================================================
// Draw calls and clears
// =================================================================================================

void RenderDeviceGL2::clear( uint32 flags, float *colorRGBA, float depth )
{
	uint32 prevBuffers[4] = { 0 };

	if( _curRendBuf != 0x0 )
	{
		RDIRenderBufferGL2 &rb = _rendBufs.getRef( _curRendBuf );
		
		if( (flags & CLR_DEPTH) && rb.depthTex == 0 ) flags &= ~CLR_DEPTH;
		
		// Store state of glDrawBuffers
		for( uint32 i = 0; i < 4; ++i )
			glGetIntegerv( GL_DRAW_BUFFER0 + i, (int *)&prevBuffers[i] );
		
		uint32 buffers[4], cnt = 0;
		
		if( (flags & CLR_COLOR_RT0) && rb.colTexs[0] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT0_EXT;
		if( (flags & CLR_COLOR_RT1) && rb.colTexs[1] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT1_EXT;
		if( (flags & CLR_COLOR_RT2) && rb.colTexs[2] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT2_EXT;
		if( (flags & CLR_COLOR_RT3) && rb.colTexs[3] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT3_EXT;

		if( cnt == 0 )
			flags &= ~(CLR_COLOR_RT0 | CLR_COLOR_RT1 | CLR_COLOR_RT2 | CLR_COLOR_RT3);
		else
			glDrawBuffers( cnt, buffers );
	}
	
	uint32 oglClearMask = 0;
	
	if( flags & CLR_DEPTH )
	{
		oglClearMask |= GL_DEPTH_BUFFER_BIT;
		glClearDepth( depth );
	}
	if( flags & (CLR_COLOR_RT0 | CLR_COLOR_RT1 | CLR_COLOR_RT2 | CLR_COLOR_RT3) )
	{
		oglClearMask |= GL_COLOR_BUFFER_BIT;
		if( colorRGBA ) glClearColor( colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3] );
		else glClearColor( 0, 0, 0, 0 );
	}
	
	if( oglClearMask )
	{	
		commitStates( PM_VIEWPORT | PM_SCISSOR | PM_RENDERSTATES );
		glClear( oglClearMask );
	}

	// Restore state of glDrawBuffers
	if( _curRendBuf != 0x0 )
		glDrawBuffers( 4, prevBuffers );

	CHECK_GL_ERROR
}


void RenderDeviceGL2::draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
{
	if( commitStates() )
	{
		glDrawArrays( RDI_GL2::primitiveTypes[ (uint32)primType ], firstVert, numVerts );
	}

	CHECK_GL_ERROR
}


void RenderDeviceGL2::drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
                                uint32 firstVert, uint32 numVerts )
{
	if( commitStates() )
	{
		firstIndex *= (_indexFormat == IDXFMT_16) ? sizeof( short ) : sizeof( int );
		
		glDrawRangeElements( RDI_GL2::primitiveTypes[ ( uint32 ) primType ], firstVert, firstVert + numVerts,
		                     numIndices, RDI_GL2::indexFormats[ _indexFormat ], (char *)0 + firstIndex );
	}

	CHECK_GL_ERROR
}

}  // namespace RDI_GL2
}  // namespace Horde3D
