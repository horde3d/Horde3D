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

#include "egRendererBaseGLES3.h"
#include "egModules.h"
#include "egCom.h"
#include "utOpenGLES3.h"

#include "utDebug.h"
#include <array>


namespace Horde3D {
namespace RDI_GLES3 {

	using namespace h3dGLES;

#ifdef H3D_VALIDATE_DRAWCALLS
#	define CHECK_GL_ERROR checkError();
#else
#	define CHECK_GL_ERROR
#endif

static const char *defaultShaderVS =
	"#version 300 es\n"
	"uniform mat4 viewProjMat;\n"
	"uniform mat4 worldMat;\n"
	"layout ( location = 0 ) in vec3 vertPos;\n"
	"void main() {\n"
	"	gl_Position = viewProjMat * worldMat * vec4( vertPos, 1.0 );\n"
	"}\n";

static const char *defaultShaderFS =
	"#version 300 es\n"
	"precision highp float;\n"
	"out vec4 fragColor;\n"
	"uniform vec4 color;\n"
	"void main() {\n"
	"	fragColor = color;\n"
	"}\n";

static const uint32 indexFormats[ 2 ] = { GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };

static const uint32 primitiveTypes[ 5 ] = { GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_LINES, GL_POINTS, 0 };

static const uint32 textureTypes[ 3 ] = { GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };

static const uint32 memoryBarrierType[ 3 ] = { GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT, GL_ELEMENT_ARRAY_BARRIER_BIT, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT };

static const uint32 bufferMappingTypes[ 3 ] = { GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT };

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
	{ GL_R16F, GL_RED, GL_R16F, GL_HALF_FLOAT },																// TextureFormats::R16F
	{ GL_R32F, GL_RED, GL_R32F, GL_FLOAT },																	// TextureFormats::R32F
	{ GL_RG8, GL_RG, GL_RG8, GL_UNSIGNED_BYTE },															// TextureFormats::RG8
	{ GL_RG16F, GL_RG, GL_RG16F, GL_HALF_FLOAT },															// TextureFormats::RG16F
	{ GL_RG32F, GL_RG, GL_RG32F, GL_FLOAT },																// TextureFormats::RG32F
	{ GL_RGBA8, GL_RGBA, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE },												// TextureFormats::BGRA8
	{ GL_RGBA16F, GL_RGBA, GL_RGBA16F, GL_HALF_FLOAT },														// TextureFormats::RGBA16F
	{ GL_RGBA32F, GL_RGBA, GL_RGBA32F, GL_FLOAT },															// TextureFormats::RGBA32F
	{ GL_RGBA32UI, GL_RGBA_INTEGER, GL_RGBA32UI, GL_UNSIGNED_INT },											// TextureFormats::RGBA32UI
	{ GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0 },						// TextureFormats::DXT1
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0 },						// TextureFormats::DXT3
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0 },						// TextureFormats::DXT5
	{ GL_ETC1_RGB8_OES, 0, GL_ETC1_RGB8_OES, 0 },															// TextureFormats::ETC1
	{ GL_COMPRESSED_RGB8_ETC2, 0, GL_COMPRESSED_SRGB8_ETC2, 0 },											// TextureFormats::RGB8_ETC2
	{ GL_COMPRESSED_RGBA8_ETC2_EAC, 0, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 0 },							// TextureFormats::RGBA8_ETC2
	{ GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 0, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 0 },			// TextureFormats::BC6_UF16
	{ GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, 0, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, 0 },				// TextureFormats::BC6_SF16
	{ GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 0, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT, GL_UNSIGNED_BYTE },	// TextureFormats::BC7
	{ GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 0 },						// TextureFormats::ASTC_4x4
	{ GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 0 },						// TextureFormats::ASTC_5x4
	{ GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 0 },						// TextureFormats::ASTC_5x5
	{ GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 0 },						// TextureFormats::ASTC_6x5
	{ GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 0 },						// TextureFormats::ASTC_6x6
	{ GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 0 },						// TextureFormats::ASTC_8x5
	{ GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 0 },						// TextureFormats::ASTC_8x6
	{ GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 0 },						// TextureFormats::ASTC_8x8
	{ GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 0 },					// TextureFormats::ASTC_10x5
	{ GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 0 },					// TextureFormats::ASTC_10x6
	{ GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 0 },					// TextureFormats::ASTC_10x8
	{ GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 0 },					// TextureFormats::ASTC_10x10
	{ GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 0 },					// TextureFormats::ASTC_12x10
	{ GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 0, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 0 },					// TextureFormats::ASTC_12x12
	{ GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT }								// TextureFormats::DEPTH
} };

// =================================================================================================
// GPUTimer
// =================================================================================================

GPUTimerGLES3::GPUTimerGLES3() : _numQueries( 0 ), _queryFrame( 0 ), _activeQuery( false )
{
	_beginQuery.bind< GPUTimerGLES3, &GPUTimerGLES3::beginQuery >( this );
	_endQuery.bind< GPUTimerGLES3, &GPUTimerGLES3::endQuery >( this );
	_updateResults.bind< GPUTimerGLES3, &GPUTimerGLES3::updateResults >( this );
	_reset.bind< GPUTimerGLES3, &GPUTimerGLES3::reset >( this );

	reset();
}


GPUTimerGLES3::~GPUTimerGLES3()
{
	if( !_queryPool.empty() )
		glDeleteQueries( (uint32)_queryPool.size(), &_queryPool[0] );
}


void GPUTimerGLES3::beginQuery( uint32 frameID )
{
	if ( !glESExt::EXT_disjoint_timer_query ) return;
	
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
	glQueryCounterEXT( queryObjs[ 0 ], GL_TIMESTAMP_EXT );
}


void GPUTimerGLES3::endQuery()
{
	if( _activeQuery )
	{	
		glQueryCounterEXT( _queryPool[ _numQueries * 2 - 1 ], GL_TIMESTAMP_EXT );
		_activeQuery = false;
	}
}


bool GPUTimerGLES3::updateResults()
{
	if( _numQueries == 0 )
	{
		_time = 0;
		return true;
	}
	
	// Make sure that last query is available
	GLint available;
	glGetQueryObjectivEXT( _queryPool[ _numQueries * 2 - 1 ], GL_QUERY_RESULT_AVAILABLE_EXT, &available );
	if( !available ) return false;
	
	//  Accumulate time
	GLuint64 timeStart = 0, timeEnd = 0, timeAccum = 0;
	for( uint32 i = 0; i < _numQueries; ++i )
	{
		glGetQueryObjectui64vEXT( _queryPool[ i * 2 ], GL_QUERY_RESULT_EXT, &timeStart );
		glGetQueryObjectui64vEXT( _queryPool[ i * 2 + 1 ], GL_QUERY_RESULT_EXT, &timeEnd );
		timeAccum += timeEnd - timeStart;
	}
	
	_time = (float)((double)timeAccum / 1000000.0);
	return true;
}


void GPUTimerGLES3::reset()
{
	_time = 0.f;
}


// =================================================================================================
// RenderDevice
// =================================================================================================

RenderDeviceGLES3::RenderDeviceGLES3()
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
	_curGeometryIndex = 1;
	_defaultFBO = 0;
	_defaultFBOMultisampled = false;
 	_indexFormat = (uint32)IDXFMT_16;
	_activeVertexAttribsMask = 0;
	_pendingMask = 0;
	_tessPatchVerts = _lastTessPatchVertsValue = 0;
	_memBarriers = NotSet;

	_maxTexSlots = 96; // for most modern hardware it is 192 (GeForce 400+, Radeon 7000+, Intel 4000+). Although 96 should probably be enough.
// 	_texSlots.reserve( _maxTexSlots ); // reserve memory

	// add default geometry for resetting
	RDIGeometryInfoGLES3 defGeom;
	defGeom.atrribsBinded = true;
	_vaos.add( defGeom );
}


RenderDeviceGLES3::~RenderDeviceGLES3()
{
}


void RenderDeviceGLES3::initRDIFuncs()
{
	_delegate_init.bind< RenderDeviceGLES3, &RenderDeviceGLES3::init >( this );
	_delegate_initStates.bind< RenderDeviceGLES3, &RenderDeviceGLES3::initStates >( this );
	_delegate_registerVertexLayout.bind< RenderDeviceGLES3, &RenderDeviceGLES3::registerVertexLayout >( this );
	_delegate_beginRendering.bind< RenderDeviceGLES3, &RenderDeviceGLES3::beginRendering >( this );

	_delegate_beginCreatingGeometry.bind< RenderDeviceGLES3, &RenderDeviceGLES3::beginCreatingGeometry >( this );
	_delegate_finishCreatingGeometry.bind< RenderDeviceGLES3, &RenderDeviceGLES3::finishCreatingGeometry >( this );
	_delegate_destroyGeometry.bind< RenderDeviceGLES3, &RenderDeviceGLES3::destroyGeometry >( this );
	_delegate_setGeomVertexParams.bind< RenderDeviceGLES3, &RenderDeviceGLES3::setGeomVertexParams >( this );
	_delegate_setGeomIndexParams.bind< RenderDeviceGLES3, &RenderDeviceGLES3::setGeomIndexParams >( this );
	_delegate_createVertexBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createVertexBuffer >( this );
	_delegate_createIndexBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createIndexBuffer >( this );
	_delegate_createTextureBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createTextureBuffer >( this );
	_delegate_createShaderStorageBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createShaderStorageBuffer >( this );
	_delegate_destroyBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::destroyBuffer >( this );
	_delegate_destroyTextureBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::destroyTextureBuffer >( this );
	_delegate_updateBufferData.bind< RenderDeviceGLES3, &RenderDeviceGLES3::updateBufferData >( this );
	_delegate_mapBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::mapBuffer >( this );
	_delegate_unmapBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::unmapBuffer >( this );

	_delegate_createTexture.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createTexture >( this );
	_delegate_generateTextureMipmap.bind< RenderDeviceGLES3, &RenderDeviceGL3::generateTextureMipmap >( this );
	_delegate_uploadTextureData.bind< RenderDeviceGLES3, &RenderDeviceGLES3::uploadTextureData >( this );
	_delegate_destroyTexture.bind< RenderDeviceGLES3, &RenderDeviceGLES3::destroyTexture >( this );
	_delegate_updateTextureData.bind< RenderDeviceGLES3, &RenderDeviceGLES3::updateTextureData >( this );
	_delegate_getTextureData.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getTextureData >( this );
	_delegate_bindImageToTexture.bind< RenderDeviceGLES3, &RenderDeviceGLES3::bindImageToTexture >( this );

	_delegate_createShader.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createShader >( this );
	_delegate_destroyShader.bind< RenderDeviceGLES3, &RenderDeviceGLES3::destroyShader >( this );
	_delegate_bindShader.bind< RenderDeviceGLES3, &RenderDeviceGLES3::bindShader >( this );
	_delegate_getShaderConstLoc.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getShaderConstLoc >( this );
	_delegate_getShaderSamplerLoc.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getShaderSamplerLoc >( this );
	_delegate_getShaderBufferLoc.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getShaderBufferLoc >( this );
	_delegate_runComputeShader.bind< RenderDeviceGLES3, &RenderDeviceGLES3::runComputeShader >( this );
	_delegate_setShaderConst.bind< RenderDeviceGLES3, &RenderDeviceGLES3::setShaderConst >( this );
	_delegate_setShaderSampler.bind< RenderDeviceGLES3, &RenderDeviceGLES3::setShaderSampler >( this );
	_delegate_getDefaultVSCode.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getDefaultVSCode >( this );
	_delegate_getDefaultFSCode.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getDefaultFSCode >( this );

	_delegate_createRenderBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createRenderBuffer >( this );
	_delegate_destroyRenderBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::destroyRenderBuffer >( this );
	_delegate_getRenderBufferTex.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getRenderBufferTex >( this );
	_delegate_setRenderBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::setRenderBuffer >( this );
	_delegate_getRenderBufferData.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getRenderBufferData >( this );
	_delegate_getRenderBufferDimensions.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getRenderBufferDimensions >( this );

	_delegate_createOcclusionQuery.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createOcclusionQuery >( this );
	_delegate_destroyQuery.bind< RenderDeviceGLES3, &RenderDeviceGLES3::destroyQuery >( this );
	_delegate_beginQuery.bind< RenderDeviceGLES3, &RenderDeviceGLES3::beginQuery >( this );
	_delegate_endQuery.bind< RenderDeviceGLES3, &RenderDeviceGLES3::endQuery >( this );
	_delegate_getQueryResult.bind< RenderDeviceGLES3, &RenderDeviceGLES3::getQueryResult >( this );

	_delegate_createGPUTimer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::createGPUTimer >( this );
	_delegate_commitStates.bind< RenderDeviceGLES3, &RenderDeviceGLES3::commitStates >( this );
	_delegate_resetStates.bind< RenderDeviceGLES3, &RenderDeviceGLES3::resetStates >( this );
	_delegate_clear.bind< RenderDeviceGLES3, &RenderDeviceGLES3::clear >( this );

	_delegate_draw.bind< RenderDeviceGLES3, &RenderDeviceGLES3::draw >( this );
	_delegate_drawIndexed.bind< RenderDeviceGLES3, &RenderDeviceGLES3::drawIndexed >( this );
	_delegate_setStorageBuffer.bind< RenderDeviceGLES3, &RenderDeviceGLES3::setStorageBuffer >( this );
}


void RenderDeviceGLES3::initStates()
{
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	GLint value;
	glGetIntegerv( GL_SAMPLE_BUFFERS, &value );
	_defaultFBOMultisampled = value > 0;
}


bool RenderDeviceGLES3::init()
{
	bool failed = false;

	char *vendor = (char *)glGetString( GL_VENDOR );
	char *renderer = (char *)glGetString( GL_RENDERER );
	char *version = (char *)glGetString( GL_VERSION );
	
 	if( !version || !renderer || !vendor )
    	{
        	Modules::log().writeError("OpenGL ES not initialized. Make sure you have a valid OpenGL ES context");
        	return false;
    	}

	Modules::log().writeInfo( "Initializing GLES3 backend using OpenGL driver '%s' by '%s' on '%s'",
	                          version, vendor, renderer );
	
	// Init extensions
	if( !initOpenGLExtensions() )
	{	
		Modules::log().writeError( "Could not find all required OpenGL function entry points" );
	}

	// Check that OpenGL ES 3.0 is available
	if( glESExt::majorVersion * 10 + glESExt::minorVersion < 30 )
	{
		Modules::log().writeError( "OpenGL ES 3.0 not available" );
		failed = true;
	}
	
	// Check that required extensions are supported
	if( !glESExt::EXT_texture_filter_anisotropic )
	{
		Modules::log().writeWarning( "Extension EXT_texture_filter_anisotropic not supported" );
	//	failed = true;
	}
	if( !glESExt::EXT_color_buffer_float )
	{
		Modules::log().writeWarning( "Extension EXT_color_buffer_float not supported" );
		//failed = true;
	}
	if( !glESExt::EXT_texture_compression_s3tc )
	{
		Modules::log().writeWarning( "Extension EXT_texture_compression_s3tc not supported" );
	}
	if ( !glESExt::EXT_disjoint_timer_query )
	{
		Modules::log().writeWarning( "Extension EXT_disjoint_timer_query not supported" );
	}
	if ( !glESExt::EXT_texture_border_clamp )
	{
		Modules::log().writeWarning( "Extension EXT_texture_border_clamp not supported" );
	}
	if ( !glESExt::EXT_geometry_shader )
	{
		Modules::log().writeWarning( "Extension EXT_geometry_shader not supported" );
	}
	
	if( failed )
	{
		Modules::log().writeError( "Failed to init renderer backend (OpenGL ES %d.%d)", 
									glESExt::majorVersion, glESExt::minorVersion );
// 		char *exts = (char *)glGetString( GL_EXTENSIONS );
// 		Modules::log().writeInfo( "Supported extensions: '%s'", exts );

		return false;
	}
	
	// Set capabilities
	_caps.texFloat = glESExt::EXT_color_buffer_float;
	_caps.texNPOT = true;
	_caps.rtMultisampling = true;
	_caps.geometryShaders = glESExt::EXT_geometry_shader;
	_caps.tesselation = false;
	_caps.computeShaders = true;
	_caps.instancing = true;
	_caps.maxJointCount = 75; // currently, will be changed soon
	_caps.maxTexUnitCount = 16; // 
	_caps.texDXT = glESExt::EXT_texture_compression_dxt1 && glESExt::EXT_texture_compression_s3tc;
	_caps.texETC2 = true;
	_caps.texBPTC = glESExt::EXT_texture_compression_bptc;
	_caps.texASTC = glESExt::KHR_texture_compression_astc;

	// Find supported depth format
	_depthFormat = GL_DEPTH_COMPONENT32F;
	uint32 testBuf = createRenderBuffer( 32, 32, TextureFormats::BGRA8, true, 1, 0, false ); 
	if( testBuf == 0 )
	{	
		_depthFormat = GL_DEPTH_COMPONENT16;
		Modules::log().writeWarning( "Render target depth precision limited to 16 bit" );
	}
	else
		destroyRenderBuffer( testBuf );
	
	initStates();
	resetStates();

	return true;
}


// =================================================================================================
// Vertex layouts
// =================================================================================================

uint32 RenderDeviceGLES3::registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs )
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

void RenderDeviceGLES3::beginRendering()
{	
	//	Get the currently bound frame buffer object. 
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &_defaultFBO );
	resetStates();
}

uint32 RenderDeviceGLES3::beginCreatingGeometry( uint32 vlObj )
{
	RDIGeometryInfoGLES3 vao;
	vao.layout = vlObj;

	uint32 vaoID;
	glGenVertexArrays( 1, &vaoID );
	vao.vao = vaoID;

	return _vaos.add( vao );
}

void RenderDeviceGLES3::finishCreatingGeometry( uint32 geoObj )
{
	ASSERT( geoObj > 0 )
	
	RDIGeometryInfoGLES3 &curVao = _vaos.getRef( geoObj );
	glBindVertexArray( curVao.vao );

	// bind index buffer, if present
	if ( curVao.indexBuf )
	{
		RDIBufferGLES3 &buf = _buffers.getRef( curVao.indexBuf );
		ASSERT( buf.glObj > 0 )

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buf.glObj );
	}

	uint32 newVertexAttribMask = 0;

	RDIVertexLayout vl = _vertexLayouts[ curVao.layout - 1 ];

	// Set vertex attrib pointers
	for ( uint32 i = 0; i < vl.numAttribs; ++i )
	{
// 		int8 attribIndex = inputLayout.attribIndices[ i ];
// 		if ( attribIndex >= 0 )
		{
			VertexLayoutAttrib &attrib = vl.attribs[ i ];
			const RDIVertBufSlotGLES3 &vbSlot = curVao.vertexBufInfo[ attrib.vbSlot ];

			RDIBufferGLES3 &buf = _buffers.getRef( curVao.vertexBufInfo[ attrib.vbSlot ].vbObj );
			ASSERT( buf.glObj != 0 &&
					buf.type == GL_ARRAY_BUFFER ||
					buf.type == GL_SHADER_STORAGE_BUFFER ); // special case for compute buffer

			glBindBuffer( GL_ARRAY_BUFFER, buf.glObj );
			glVertexAttribPointer( i, attrib.size, GL_FLOAT, GL_FALSE,
								   vbSlot.stride, ( char * ) 0 + vbSlot.offset + attrib.offset );

			newVertexAttribMask |= 1 << i;
		}
	}


	for ( uint32 i = 0; i < 16; ++i )
	{
		uint32 curBit = 1 << i;
		if ( ( newVertexAttribMask & curBit ) != ( _activeVertexAttribsMask & curBit ) )
		{
			if ( newVertexAttribMask & curBit ) glEnableVertexAttribArray( i );
			else glDisableVertexAttribArray( i );
		}
	}

	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void RenderDeviceGLES3::setGeomVertexParams( uint32 geoObj, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride )
{
	RDIGeometryInfoGLES3 &curVao = _vaos.getRef( geoObj );
	RDIBufferGLES3 &buf = _buffers.getRef( vbo );

	buf.geometryRefCount++;

	RDIVertBufSlotGLES3 attribInfo;
	attribInfo.vbObj = vbo;
	attribInfo.offset = offset;
	attribInfo.stride = stride;

	curVao.vertexBufInfo.push_back( attribInfo );
}

void RenderDeviceGLES3::setGeomIndexParams( uint32 geoObj, uint32 indBuf, RDIIndexFormat format )
{
	RDIGeometryInfoGLES3 &curVao = _vaos.getRef( geoObj );
	RDIBufferGLES3 &buf = _buffers.getRef( indBuf );

	buf.geometryRefCount++;

	curVao.indexBuf = indBuf;
	curVao.indexBuf32Bit = ( format == IDXFMT_32 ? true : false );
}

void RenderDeviceGLES3::destroyGeometry( uint32 &geoObj, bool destroyBindedBuffers )
{
	if ( geoObj == 0 )
		return;

	RDIGeometryInfoGLES3 &curVao = _vaos.getRef( geoObj );

	glDeleteVertexArrays( 1, &curVao.vao );
	glBindVertexArray( 0 );

	if ( destroyBindedBuffers )
	{
		for ( unsigned int i = 0; i < curVao.vertexBufInfo.size(); ++i )
		{
			decreaseBufferRefCount( curVao.vertexBufInfo[ i ].vbObj );
			destroyBuffer( curVao.vertexBufInfo[ i ].vbObj );
		}

		decreaseBufferRefCount( curVao.indexBuf );
		destroyBuffer( curVao.indexBuf );
	}
	else
	{
		for ( size_t i = 0; i < curVao.vertexBufInfo.size(); ++i )
		{
			decreaseBufferRefCount( curVao.vertexBufInfo[ i ].vbObj );
		}
		decreaseBufferRefCount( curVao.indexBuf );
	}

	_vaos.remove( geoObj );
	geoObj = 0;
}

void RenderDeviceGLES3::decreaseBufferRefCount( uint32 bufObj )
{
	if ( bufObj == 0 ) return;
	
	RDIBufferGLES3 &buf = _buffers.getRef( bufObj );

	buf.geometryRefCount--;
}

uint32 RenderDeviceGLES3::createVertexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ARRAY_BUFFER, size, data );
}


uint32 RenderDeviceGLES3::createIndexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ELEMENT_ARRAY_BUFFER, size, data );
}


uint32 RenderDeviceGLES3::createShaderStorageBuffer( uint32 size, const void *data )
{
	if ( _caps.computeShaders )
		return createBuffer( GL_SHADER_STORAGE_BUFFER, size, data );
	else
	{
		Modules::log().writeError( "Shader storage buffers are not supported on this OpenGL ES device." );
		
		return 0;
	}
}


uint32 RenderDeviceGLES3::createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data )
{
// 	RDITextureBufferGLES3 buf;
// 
// 	buf.bufObj = createBuffer( GL_TEXTURE_BUFFER, bufSize, data );
// 
// 	glGenTextures( 1, &buf.glTexID );
// 	glActiveTexture( GL_TEXTURE15 );
// 	glBindTexture( GL_TEXTURE_BUFFER, buf.glTexID );
// 
// 	switch ( format )
// 	{
// 		case TextureFormats::BGRA8:
// 			buf.glFmt = GL_RGBA8;
// 			break;
// 		case TextureFormats::RGBA16F:
// 			buf.glFmt = GL_RGBA16F;
// 			break;
// 		case TextureFormats::RGBA32F:
// 			buf.glFmt = GL_RGBA32F;
// 			break;
// 		case TextureFormats::R32:
// 			buf.glFmt = GL_R32F;
// 			break;
// 		case TextureFormats::RG32:
// 			buf.glFmt = GL_RG32F;
// 			break;
// 		default:
// 			ASSERT( 0 );
// 			break;
// 	};
// 
// 	// bind texture to buffer
// 	glTexBuffer( GL_TEXTURE_BUFFER, buf.glFmt, _buffers.getRef( buf.bufObj ).glObj );
// 
// 	glBindTexture( GL_TEXTURE_BUFFER, 0 );
// 	if ( _texSlots[ 15 ].texObj )
// 		glBindTexture( _textures.getRef( _texSlots[ 15 ].texObj ).type, _textures.getRef( _texSlots[ 15 ].texObj ).glObj );
// 
// 	return _textureBuffs.add( buf );

	return 0;
}


uint32 RenderDeviceGLES3::createBuffer( uint32 bufType, uint32 size, const void *data )
{
	RDIBufferGLES3 buf;

	buf.type = bufType;
	buf.size = size;
	glGenBuffers( 1, &buf.glObj );
	glBindBuffer( buf.type, buf.glObj );
	glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );
	glBindBuffer( buf.type, 0 );

	_bufferMem += size;
	return _buffers.add( buf );
}


void RenderDeviceGLES3::destroyBuffer( uint32 &bufObj )
{
	if( bufObj == 0 ) return;
	
	RDIBufferGLES3 &buf = _buffers.getRef( bufObj );
	if ( buf.geometryRefCount < 1 )
	{
		glDeleteBuffers( 1, &buf.glObj );

		_bufferMem -= buf.size;
		_buffers.remove( bufObj );
		bufObj = 0;
	}
}


void RenderDeviceGLES3::destroyTextureBuffer( uint32 &bufObj )
{
	if ( bufObj == 0 ) return;

	RDITextureBufferGLES3 &buf = _textureBuffs.getRef( bufObj );
	destroyBuffer( buf.bufObj );

	glDeleteTextures( 1, &buf.glTexID );

	_textureBuffs.remove( bufObj );
	bufObj = 0;
}

void RenderDeviceGLES3::updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data )
{
//	const RDIGeometryInfoGLES3 &geo = _vaos.getRef( geoObj );
	const RDIBufferGLES3 &buf = _buffers.getRef( bufObj );
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


void * RenderDeviceGLES3::mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType )
{
	const RDIBufferGLES3 &buf = _buffers.getRef( bufObj );
	ASSERT( offset + size <= buf.size );

	glBindBuffer( buf.type, buf.glObj );
	
	if ( offset == 0 && size == buf.size && mapType == Write )
	{
		return glMapBufferRange( buf.type, offset, size, bufferMappingTypes[ mapType ] | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
	}

	return glMapBufferRange( buf.type, offset, size, bufferMappingTypes[ mapType ] );
}

void RenderDeviceGLES3::unmapBuffer( uint32 geoObj, uint32 bufObj )
{
	const RDIBufferGLES3 &buf = _buffers.getRef( bufObj );

	// multiple buffers can be mapped at the same time, so bind the one that needs to be unmapped
	glBindBuffer( buf.type, buf.glObj );

	glUnmapBuffer( buf.type );
}


// =================================================================================================
// Textures
// =================================================================================================

uint32 RenderDeviceGLES3::createTexture( TextureTypes::List type, int width, int height, int depth,
										 TextureFormats::List format,
										 bool hasMips, bool genMips, bool compress, bool sRGB )
{
	ASSERT( depth > 0 );

	if ( !_caps.texFloat && ( format == TextureFormats::RGBA16F || format == TextureFormats::RGBA32F ) )
	{
		Modules::log().writeWarning( "Unsupported texture formats: RGBA16F, RGBA32F" );
		return 0;
	}
	if ( !_caps.texDXT && ( format == TextureFormats::DXT1 || format == TextureFormats::DXT3 || format == TextureFormats::DXT5 ) )
	{
		Modules::log().writeWarning( "Unsupported texture formats: DXT1, DXT3, DXT5" );
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
	
	RDITextureGLES3 tex;
	tex.type = textureTypes[ type ];
	tex.format = format;
	tex.width = width;
	tex.height = height;
	tex.depth = depth;
	tex.sRGB = sRGB && Modules::config().sRGBLinearization;
	tex.genMips = genMips;
	tex.hasMips = hasMips;

	if ( format > ( int ) textureGLFormats.size() ) { ASSERT( 0 ); return 0; }

	tex.glFmt = format != TextureFormats::DEPTH 
				? ( tex.sRGB ? textureGLFormats[ format ].glSRGBFormat : textureGLFormats[ format ].glCreateFormat )
				: _depthFormat;
 	
	glGenTextures( 1, &tex.glObj );
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	
	tex.samplerState = 0;
	applySamplerState( tex );
	
	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );

	// Calculate memory requirements
	tex.memSize = calcTextureSize( format, width, height, depth );
	if( hasMips || genMips ) tex.memSize += ftoi_r( tex.memSize * 1.0f / 3.0f );
	if( type == TextureTypes::TexCube ) tex.memSize *= 6;
	_textureMem += tex.memSize;
	
	return _textures.add( tex );
}


void RenderDeviceGLES3::generateTextureMipmap( uint32 texObj )
{
	const RDITextureGL4 &tex = _textures.getRef( texObj );

	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	glGenerateMipmap( tex.type );
	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
}


void RenderDeviceGLES3::uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	const RDITextureGLES3 &tex = _textures.getRef( texObj );
	TextureFormats::List format = tex.format;

	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	
	bool compressed = isCompressedTextureFormat( format );

	int inputType = textureGLFormats[ format ].glInputType;
	int inputFormat = textureGLFormats[ format ].glInputFormat;

	// Calculate size of next mipmap using "floor" convention
	int width = std::max( tex.width >> mipLevel, 1 ), height = std::max( tex.height >> mipLevel, 1 );
	
	if ( tex.type == textureTypes[ TextureTypes::Tex2D ] || tex.type == textureTypes[ TextureTypes::TexCube ] )
	{
		int target = ( tex.type == textureTypes[ TextureTypes::Tex2D ] ) ?
			GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice);
		
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
		glGenerateMipmap( tex.type );
	}

	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
}


void RenderDeviceGLES3::destroyTexture( uint32 &texObj )
{
	if( texObj == 0 ) return;
	
	const RDITextureGLES3 &tex = _textures.getRef( texObj );
	if( tex.glObj ) glDeleteTextures( 1, &tex.glObj );

	_textureMem -= tex.memSize;
	_textures.remove( texObj );
	texObj = 0;
}


void RenderDeviceGLES3::updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	uploadTextureData( texObj, slice, mipLevel, pixels );
}


bool RenderDeviceGLES3::getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer )
{
 	const RDITextureGLES3 &tex = _textures.getRef( texObj );
 	
 	int target = tex.type == textureTypes[ TextureTypes::TexCube ] ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
 	if( target == GL_TEXTURE_CUBE_MAP ) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;
	
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );

	// create a temporary fbo 
	GLint currentFBO = 0;
    GLuint tempFBO = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &currentFBO );
    glGenFramebuffers( 1, &tempFBO );
	glBindFramebuffer( GL_FRAMEBUFFER, tempFBO );

    GLenum status;

    switch (target) {
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.glObj, mipLevel );
		status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
        if ( status != GL_FRAMEBUFFER_COMPLETE ) 
		{
			return false;
        }

        glReadPixels( 0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
        break;
    case GL_TEXTURE_3D:
        for (int i = 0; i < tex.depth; i++) {
			glFramebufferTexture3DOES( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tex.glObj, mipLevel, i );
            glReadPixels(0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, ( uint8 * ) buffer + 4 * i * tex.width * tex.height );
        }
        break;
    }

	glBindFramebuffer( GL_FRAMEBUFFER, currentFBO );
	glDeleteFramebuffers( 1, &tempFBO );
 
	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );

	return true;
}


bool RenderDeviceGLES3::isCompressedTextureFormat( TextureFormats::List fmt )
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
		( fmt == TextureFormats::ASTC_12x12 );
}


void RenderDeviceGLES3::bindImageToTexture(uint32 texObj, void *eglImage)
{
	if( !glESExt::OES_EGL_image_external )
		Modules::log().writeError("OES_egl_image not supported");
	else
	{
		const RDITextureGLES3 &tex = _textures.getRef( texObj );
		glActiveTexture( GL_TEXTURE15 );
		glBindTexture( tex.type, tex.glObj );
		glEGLImageTargetTexture2DOES( tex.type, eglImage );
		checkError();
		glBindTexture( tex.type, 0 );
		if( _texSlots[15].texObj )
			glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
	}
}


// =================================================================================================
// Shaders
// =================================================================================================

uint32 RenderDeviceGLES3::createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc, 
											 const char *tessControlShaderSrc, const char *tessEvalShaderSrc, const char *computeShaderSrc )
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog = 0x0;
	int status;

	_shaderLog = "";
	
	uint32 vs, fs, gs, tsC, tsE, cs;
	vs = fs = gs = tsC = tsE = cs = 0;

	// Vertex shader
	if ( vertexShaderSrc )
	{
		vs = glCreateShader( GL_VERTEX_SHADER );
		glShaderSource( vs, 1, &vertexShaderSrc, 0x0 );
		glCompileShader( vs );
		glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
		if ( !status )
		{
			// Get info
			glGetShaderiv( vs, GL_INFO_LOG_LENGTH, &infologLength );
			if ( infologLength > 1 )
			{
				infoLog = new char[ infologLength ];
				glGetShaderInfoLog( vs, infologLength, &charsWritten, infoLog );
				_shaderLog = _shaderLog + "[Vertex Shader]\n" + infoLog;
				delete[] infoLog; infoLog = 0x0;
			}

			glDeleteShader( vs );
			return 0;
		}
	}
	
	// Fragment shader
	if ( fragmentShaderSrc )
	{
		fs = glCreateShader( GL_FRAGMENT_SHADER );
		glShaderSource( fs, 1, &fragmentShaderSrc, 0x0 );
		glCompileShader( fs );
		glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
		if ( !status )
		{
			glGetShaderiv( fs, GL_INFO_LOG_LENGTH, &infologLength );
			if ( infologLength > 1 )
			{
				infoLog = new char[ infologLength ];
				glGetShaderInfoLog( fs, infologLength, &charsWritten, infoLog );
				_shaderLog = _shaderLog + "[Fragment Shader]\n" + infoLog;
				delete[] infoLog; infoLog = 0x0;
			}

			glDeleteShader( vs );
			glDeleteShader( fs );
			return 0;
		}
	}
	
	// Geometry shader
	if ( geometryShaderSrc )
	{
		gs = glCreateShader( GL_GEOMETRY_SHADER_EXT );
		glShaderSource( gs, 1, &geometryShaderSrc, 0x0 );
		glCompileShader( gs );
		glGetShaderiv( gs, GL_COMPILE_STATUS, &status );
		if ( !status )
		{
			glGetShaderiv( gs, GL_INFO_LOG_LENGTH, &infologLength );
			if ( infologLength > 1 )
			{
				infoLog = new char[ infologLength ];
				glGetShaderInfoLog( gs, infologLength, &charsWritten, infoLog );
				_shaderLog = _shaderLog + "[Geometry Shader]\n" + infoLog;
				delete[] infoLog; infoLog = 0x0;
			}

			glDeleteShader( vs );
			glDeleteShader( fs );
			glDeleteShader( gs );
			return 0;
		}
	}
	
	// Tesselation control shader
// 	if ( tessControlShaderSrc )
// 	{
// 		tsC = glCreateShader( GL_TESS_CONTROL_SHADER );
// 		glShaderSource( tsC, 1, &tessControlShaderSrc, 0x0 );
// 		glCompileShader( tsC );
// 		glGetShaderiv( tsC, GL_COMPILE_STATUS, &status );
// 		if ( !status )
// 		{
// 			glGetShaderiv( tsC, GL_INFO_LOG_LENGTH, &infologLength );
// 			if ( infologLength > 1 )
// 			{
// 				infoLog = new char[ infologLength ];
// 				glGetShaderInfoLog( tsC, infologLength, &charsWritten, infoLog );
// 				_shaderLog = _shaderLog + "[Tesselation Control Shader]\n" + infoLog;
// 				delete[] infoLog; infoLog = 0x0;
// 			}
// 
// 			glDeleteShader( vs );
// 			glDeleteShader( fs );
// 			if ( gs ) glDeleteShader( gs );
// 			glDeleteShader( tsC );
// 			return 0;
// 		}
// 	}
// 
// 	// Tesselation evaluation shader
// 	if ( tessEvalShaderSrc )
// 	{
// 		tsE = glCreateShader( GL_TESS_EVALUATION_SHADER );
// 		glShaderSource( tsE, 1, &tessEvalShaderSrc, 0x0 );
// 		glCompileShader( tsE );
// 		glGetShaderiv( tsE, GL_COMPILE_STATUS, &status );
// 		if ( !status )
// 		{
// 			glGetShaderiv( tsE, GL_INFO_LOG_LENGTH, &infologLength );
// 			if ( infologLength > 1 )
// 			{
// 				infoLog = new char[ infologLength ];
// 				glGetShaderInfoLog( tsE, infologLength, &charsWritten, infoLog );
// 				_shaderLog = _shaderLog + "[Tesselation Evaluation Shader]\n" + infoLog;
// 				delete[] infoLog; infoLog = 0x0;
// 			}
// 
// 			glDeleteShader( vs );
// 			glDeleteShader( fs );
// 			if ( gs ) glDeleteShader( gs );
// 			glDeleteShader( tsC );
// 			glDeleteShader( tsE );
// 			return 0;
// 		}
// 	}

	// Compute shader
	if ( computeShaderSrc )
	{
		cs = glCreateShader( GL_COMPUTE_SHADER );
		glShaderSource( cs, 1, &computeShaderSrc, 0x0 );
		glCompileShader( cs );
		glGetShaderiv( cs, GL_COMPILE_STATUS, &status );
		if ( !status )
		{
			glGetShaderiv( cs, GL_INFO_LOG_LENGTH, &infologLength );
			if ( infologLength > 1 )
			{
				infoLog = new char[ infologLength ];
				glGetShaderInfoLog( cs, infologLength, &charsWritten, infoLog );
				_shaderLog = _shaderLog + "[Compute Shader]\n" + infoLog;
				delete[] infoLog; infoLog = 0x0;
			}

			// other shader types should not be present in compute context, but better check
			if ( vs ) glDeleteShader( vs );
			if ( fs ) glDeleteShader( fs );
			if ( gs ) glDeleteShader( gs );
			if ( tsC ) glDeleteShader( tsC );
			if ( tsE ) glDeleteShader( tsE );
			glDeleteShader( cs );
			return 0;
		}
	}

	// Shader program
	uint32 program = glCreateProgram();
	if ( vs && fs )
	{
		glAttachShader( program, vs );
		glAttachShader( program, fs );

		glDeleteShader( vs );
		glDeleteShader( fs );
	}

	if ( gs )
	{
		glAttachShader( program, gs );
		glDeleteShader( gs );
	}
	if ( tsC )
	{
		glAttachShader( program, tsC );
		glDeleteShader( tsC );
	}
	if ( tsE )
	{
		glAttachShader( program, tsE );
		glDeleteShader( tsE );
	}
	if ( cs )
	{
		glAttachShader( program, cs );
		glDeleteShader( cs );
	}

	return program;
}


bool RenderDeviceGLES3::linkShaderProgram( uint32 programObj )
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


uint32 RenderDeviceGLES3::createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
									  const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc )
{
	// Compile and link shader
	uint32 programObj = createShaderProgram( vertexShaderSrc, fragmentShaderSrc, geometryShaderSrc, tessControlShaderSrc, tessEvaluationShaderSrc, computeShaderSrc );
	if( programObj == 0 ) return 0;
	if( !linkShaderProgram( programObj ) ) return 0;

//	int loc = glGetFragDataLocation( programObj, "fragColor" );

	uint32 shaderId = _shaders.add( RDIShaderGLES3() );
	RDIShaderGLES3 &shader = _shaders.getRef( shaderId );
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


void RenderDeviceGLES3::destroyShader( uint32 &shaderId )
{
	if( shaderId == 0 ) return;

	RDIShaderGLES3 &shader = _shaders.getRef( shaderId );
	glDeleteProgram( shader.oglProgramObj );
	_shaders.remove( shaderId );
	shaderId = 0;
}


void RenderDeviceGLES3::bindShader( uint32 shaderId )
{
	if( shaderId != 0 )
	{
		RDIShaderGLES3 &shader = _shaders.getRef( shaderId );
		glUseProgram( shader.oglProgramObj );
	}
	else
	{
		glUseProgram( 0 );
	}
	
	_curShaderId = shaderId;
	_pendingMask |= PM_GEOMETRY;
} 


int RenderDeviceGLES3::getShaderConstLoc( uint32 shaderId, const char *name )
{
	RDIShaderGLES3 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


int RenderDeviceGLES3::getShaderSamplerLoc( uint32 shaderId, const char *name )
{
	RDIShaderGLES3 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}

int RenderDeviceGLES3::getShaderBufferLoc( uint32 shaderId, const char *name )
{
	if ( _caps.computeShaders )
	{
		RDIShaderGLES3 &shader = _shaders.getRef( shaderId );
		int idx = glGetProgramResourceIndex( shader.oglProgramObj, GL_SHADER_STORAGE_BLOCK, name );
		if( idx != -1 )
		{
			const GLenum bufBindingPoint[ 1 ] = { GL_BUFFER_BINDING };
			glGetProgramResourceiv( shader.oglProgramObj, GL_SHADER_STORAGE_BLOCK, idx, 1, bufBindingPoint, 1, 0, &idx );
		}
		return idx;
	}
	else
	{
		Modules::log().writeError( "Shader storage buffers are not supported on this device." );

		return -1;
	}
}


void RenderDeviceGLES3::setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count )
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


void RenderDeviceGLES3::setShaderSampler( int loc, uint32 texUnit )
{
	glUniform1i( loc, (int)texUnit );
}


const char *RenderDeviceGLES3::getDefaultVSCode()
{
	return defaultShaderVS;
}


const char *RenderDeviceGLES3::getDefaultFSCode()
{
	return defaultShaderFS;
}


void RenderDeviceGLES3::runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim )
{
	bindShader( shaderId );

	if ( commitStates( ~PM_GEOMETRY ) )
		glDispatchCompute( xDim, yDim, zDim );
}

// =================================================================================================
// Renderbuffers
// =================================================================================================

/*
uint32 RenderDeviceGLES3::createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
                                         bool depth, uint32 numColBufs, uint32 samples )
{
	if( (format == TextureFormats::RGBA16F || format == TextureFormats::RGBA32F) && !_caps.texFloat )
	{
		return 0;
	}

	if( numColBufs > RDIRenderBufferGLES3::MaxColorAttachmentCount ) return 0;

	uint32 maxSamples = 0;
	if( _caps.rtMultisampling )
	{
		GLint value;
		glGetIntegerv( GL_MAX_SAMPLES, &value );
		maxSamples = (uint32)value;
	}
	if( samples > maxSamples )
	{
		samples = maxSamples;
		Modules::log().writeWarning( "GPU does not support desired multisampling quality for render target" );
	}

	RDIRenderBufferGLES3 rb;
	rb.width = width;
	rb.height = height;
	rb.samples = samples;

	// Create framebuffers
	glGenFramebuffers( 1, &rb.fbo );
	if( samples > 0 ) glGenFramebuffers( 1, &rb.fboMS );

	if( numColBufs > 0 )
	{
		// Attach color buffers
		for( uint32 j = 0; j < numColBufs; ++j )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
			// Create a color texture
			uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, format, false, false, false, false );
			ASSERT( texObj != 0 );
			uploadTextureData( texObj, 0, 0, 0x0 );
			rb.colTexs[j] = texObj;
			RDITextureGLES3 &tex = _textures.getRef( texObj );
			// Attach the texture
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, tex.glObj, 0 );

			if( samples > 0 )
			{
				glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
				// Create a multisampled renderbuffer
				glGenRenderbuffers( 1, &rb.colBufs[j] );
				glBindRenderbuffer( GL_RENDERBUFFER, rb.colBufs[j] );
				glRenderbufferStorageMultisample( GL_RENDERBUFFER, rb.samples, tex.glFmt, rb.width, rb.height );
				// Attach the renderbuffer
				glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j,
				                              GL_RENDERBUFFER, rb.colBufs[j] );
			}
		}

		uint32 buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
		                     GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		glDrawBuffers( numColBufs, buffers );
		
		if( samples > 0 )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
			glDrawBuffers( numColBufs, buffers );
		}
	}
// 	else
// 	{	
// 		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
// 		glDrawBuffer( GL_NONE );
// 		glReadBuffer( GL_NONE );
// 		
// 		if( samples > 0 )
// 		{
// 			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
// 			glDrawBuffer( GL_NONE );
// 			glReadBuffer( GL_NONE );
// 		}
// 	}

	// Attach depth buffer
	if( depth )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		// Create a depth texture
		uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, TextureFormats::DEPTH, false, false, false, false );
		ASSERT( texObj != 0 );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
		uploadTextureData( texObj, 0, 0, 0x0 );
		rb.depthTex = texObj;
		RDITextureGLES3 &tex = _textures.getRef( texObj );
		// Attach the texture
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex.glObj, 0 );

		if( samples > 0 )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
			// Create a multisampled renderbuffer
			glGenRenderbuffers( 1, &rb.depthBuf );
			glBindRenderbuffer( GL_RENDERBUFFER, rb.depthBuf );
			glRenderbufferStorageMultisample( GL_RENDERBUFFER, rb.samples, _depthFormat, rb.width, rb.height );
			// Attach the renderbuffer
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			                              GL_RENDERBUFFER, rb.depthBuf );
		}
	}

	uint32 rbObj = _rendBufs.add( rb );
	
	// Check if FBO is complete
	bool valid = true;
	glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
	uint32 status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
	if( status != GL_FRAMEBUFFER_COMPLETE ) valid = false;
	
	if( samples > 0 )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
		status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
		if( status != GL_FRAMEBUFFER_COMPLETE ) valid = false;
	}

	if( !valid )
	{
		destroyRenderBuffer( rbObj );
		return 0;
	}
	
	return rbObj;
}*/

uint32 RenderDeviceGLES3::createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
	                                          bool depth, uint32 numColBufs, uint32 samples, bool hasMipmaps )
{
	if ( ( format == TextureFormats::RGBA16F || format == TextureFormats::RGBA32F ) && !_caps.texFloat )
	{
		return 0;
	}

	if ( numColBufs > RDIRenderBufferGLES3::MaxColorAttachmentCount ) return 0;

	uint32 maxSamples = 0;
	GLint value;
	glGetIntegerv( GL_MAX_SAMPLES, &value );
	maxSamples = ( uint32 ) value;

	if ( samples > maxSamples )
	{
		samples = maxSamples;
		Modules::log().writeWarning( "GPU does not support desired multisampling quality for render target" );
	}

	RDIRenderBufferGLES3 rb;
	rb.width = width;
	rb.height = height;
	rb.samples = samples;

	// Create framebuffers
	glGenFramebuffers( 1, &rb.fbo );
	if ( samples > 0 ) glGenFramebuffers( 1, &rb.fboMS );

	if ( numColBufs > 0 )
	{
		// Attach color buffers
		for ( uint32 j = 0; j < numColBufs; ++j )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
			// Create a color texture
			uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, format, hasMipmaps, hasMipmaps, false, false );
			ASSERT( texObj != 0 );
			uploadTextureData( texObj, 0, 0, 0x0 );
			rb.colTexs[ j ] = texObj;
			RDITextureGLES3 &tex = _textures.getRef( texObj );
			// Attach the texture
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, tex.glObj, 0 );

			if ( samples > 0 )
			{
				glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
				// Create a multisampled renderbuffer
				glGenRenderbuffers( 1, &rb.colBufs[ j ] );
				glBindRenderbuffer( GL_RENDERBUFFER, rb.colBufs[ j ] );
				glRenderbufferStorageMultisample( GL_RENDERBUFFER, rb.samples, tex.glFmt, rb.width, rb.height );
				// Attach the renderbuffer
				glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_RENDERBUFFER, rb.colBufs[ j ] );
			}
		}

		uint32 buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		glDrawBuffers( numColBufs, buffers );

		if ( samples > 0 )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
			glDrawBuffers( numColBufs, buffers );
		}
	}
	else
	{
		uint32 buffers[] = { GL_NONE };

		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		glDrawBuffers( 0, buffers );
		glReadBuffer( GL_NONE );

		if ( samples > 0 )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
			glDrawBuffers( 0, buffers );
			glReadBuffer( GL_NONE );
		}
	}

	// Attach depth buffer
	if ( depth )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );

		// Create a depth texture
		if ( samples > 0 )
		{
			glGenRenderbuffers( 1, &rb.depthBuf );
			glBindRenderbuffer( GL_RENDERBUFFER, rb.depthBuf );
			if ( samples > 0 )
				glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, _depthFormat, rb.width, rb.height );
			else
				glRenderbufferStorage( GL_RENDERBUFFER, _depthFormat, rb.width, rb.height );

			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb.depthBuf );
		}
		else
		{
			uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, TextureFormats::DEPTH, false, false, false, false );
			ASSERT( texObj != 0 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
			uploadTextureData( texObj, 0, 0, 0x0 );
			rb.depthTex = texObj;
			RDITextureGLES3 &tex = _textures.getRef( texObj );
			// Attach the texture
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex.glObj, 0 );
		}

		// Create a multisampled renderbuffer
		if ( samples > 0 )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
			glGenRenderbuffers( 1, &rb.depthBuf );
			glBindRenderbuffer( GL_RENDERBUFFER, rb.depthBuf );
			glRenderbufferStorageMultisample( GL_RENDERBUFFER, rb.samples, _depthFormat, rb.width, rb.height );
			// Attach the renderbuffer
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GL_RENDERBUFFER, rb.depthBuf );
		}
	}

	uint32 rbObj = _rendBufs.add( rb );

	// Check if FBO is complete
	bool valid = true;
	glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
	uint32 status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	if ( status != GL_FRAMEBUFFER_COMPLETE ) valid = false;

	if ( samples > 0 )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
		status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		if ( status != GL_FRAMEBUFFER_COMPLETE ) valid = false;
	}

	if ( !valid )
	{
		destroyRenderBuffer( rbObj );
		return 0;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );

	return rbObj;
}


void RenderDeviceGLES3::destroyRenderBuffer( uint32 &rbObj )
{
	RDIRenderBufferGLES3 &rb = _rendBufs.getRef( rbObj );
	
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
	
	if( rb.depthTex != 0 ) destroyTexture( rb.depthTex );
	if( rb.depthBuf != 0 ) glDeleteRenderbuffers( 1, &rb.depthBuf );
	rb.depthTex = rb.depthBuf = 0;
		
	for( uint32 i = 0; i < RDIRenderBufferGLES3::MaxColorAttachmentCount; ++i )
	{
		if( rb.colTexs[i] != 0 ) destroyTexture( rb.colTexs[i] );
		if( rb.colBufs[i] != 0 ) glDeleteRenderbuffers( 1, &rb.colBufs[i] );
		rb.colTexs[i] = rb.colBufs[i] = 0;
	}

	if( rb.fbo != 0 ) glDeleteFramebuffers( 1, &rb.fbo );
	if( rb.fboMS != 0 ) glDeleteFramebuffers( 1, &rb.fboMS );
	rb.fbo = rb.fboMS = 0;

	_rendBufs.remove( rbObj );
	rbObj = 0;
}


void RenderDeviceGLES3::getRenderBufferDimensions( uint32 rbObj, int *width, int *height )
{
	RDIRenderBufferGLES3 &rb = _rendBufs.getRef( rbObj );

	*width = rb.width;
	*height = rb.height;
}


uint32 RenderDeviceGLES3::getRenderBufferTex( uint32 rbObj, uint32 bufIndex )
{
	RDIRenderBufferGLES3 &rb = _rendBufs.getRef( rbObj );
	
	if( bufIndex < RDIRenderBufferGLES3::MaxColorAttachmentCount ) return rb.colTexs[bufIndex];
	else if( bufIndex == 32 ) return rb.depthTex;
	else return 0;
}


void RenderDeviceGLES3::resolveRenderBuffer( uint32 rbObj )
{
	RDIRenderBufferGLES3 &rb = _rendBufs.getRef( rbObj );
	
	if( rb.fboMS == 0 ) return;
	
	glBindFramebuffer( GL_READ_FRAMEBUFFER, rb.fboMS );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, rb.fbo );

	bool depthResolved = false;
	for( uint32 i = 0; i < RDIRenderBufferGLES3::MaxColorAttachmentCount; ++i )
	{
		if( rb.colBufs[i] != 0 )
		{
			uint32 buffers[] = { GL_COLOR_ATTACHMENT0 + i };

			glReadBuffer( GL_COLOR_ATTACHMENT0 + i );
			glDrawBuffers( 1, buffers );
			//glReadBuffer( GL_COLOR_ATTACHMENT0 + i );
			//glDrawBuffer( GL_COLOR_ATTACHMENT0 + i );
			
			int mask = GL_COLOR_BUFFER_BIT;
			if( !depthResolved && rb.depthBuf != 0 )
			{
				mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
				depthResolved = true;
			}
			glBlitFramebuffer( 0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height, mask, GL_NEAREST );
		}
	}

	if( !depthResolved && rb.depthBuf != 0 )
	{
		uint32 buffers[] = { GL_NONE };

		glReadBuffer( GL_NONE );
		glDrawBuffers( 0, buffers );

		//glReadBuffer( GL_NONE );
		//glDrawBuffer( GL_NONE );
		glBlitFramebuffer( 0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height,
							  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST );
	}

	glBindFramebuffer( GL_READ_FRAMEBUFFER, _defaultFBO );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, _defaultFBO );
}


void RenderDeviceGLES3::setRenderBuffer( uint32 rbObj )
{
	// Resolve render buffer if necessary
	if( _curRendBuf != 0 ) resolveRenderBuffer( _curRendBuf );
	
	// Set new render buffer
	_curRendBuf = rbObj;
	
	if( rbObj == 0 )
	{
// 		glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
// 		if( _defaultFBO == 0 ) glDrawBuffer( _outputBufferIndex == 1 ? GL_BACK_RIGHT : GL_BACK_LEFT );

		int currentFrameBuffer;
		glGetIntegerv( GL_FRAMEBUFFER_BINDING, &currentFrameBuffer );

		//check if it is already, because this call can be extremely expensive on some platforms
		if ( currentFrameBuffer != _defaultFBO )
			glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );

		_fbWidth = _vpWidth + _vpX;
		_fbHeight = _vpHeight + _vpY;
//         if( _defaultFBOMultisampled ) glEnable( GL_MULTISAMPLE );
// 		else glDisable( GL_MULTISAMPLE );
	}
	else
	{
		// Unbind all textures to make sure that no FBO attachment is bound any more
		for( uint32 i = 0; i < 16; ++i ) setTexture( i, 0, 0, 0 );
		commitStates( PM_TEXTURES );
		
		RDIRenderBufferGLES3 &rb = _rendBufs.getRef( rbObj );

		glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS != 0 ? rb.fboMS : rb.fbo );
		ASSERT( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
		_fbWidth = rb.width;
		_fbHeight = rb.height;

// 		if( rb.fboMS != 0 ) glEnable( GL_MULTISAMPLE );
// 		else glDisable( GL_MULTISAMPLE );
	}
}


bool RenderDeviceGLES3::getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
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

		glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
		if( bufIndex != 32 ) glReadBuffer( GL_BACK );
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
	}
	else
	{
		resolveRenderBuffer( rbObj );
		RDIRenderBufferGLES3 &rb = _rendBufs.getRef( rbObj );
		
		if( bufIndex == 32 && rb.depthTex == 0 ) return false;
		if( bufIndex != 32 )
		{
			if( (unsigned)bufIndex >= RDIRenderBufferGLES3::MaxColorAttachmentCount || rb.colTexs[bufIndex] == 0 )
				return false;
		}
		if( width != 0x0 ) *width = rb.width;
		if( height != 0x0 ) *height = rb.height;

		x = 0; y = 0; w = rb.width; h = rb.height;
		
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		if( bufIndex != 32 ) glReadBuffer( GL_COLOR_ATTACHMENT0 + bufIndex );
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
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );

	return retVal;
}


// =================================================================================================
// Queries
// =================================================================================================

uint32 RenderDeviceGLES3::createOcclusionQuery()
{
	uint32 queryObj;
	glGenQueries( 1, &queryObj );
	return queryObj;
}


void RenderDeviceGLES3::destroyQuery( uint32 queryObj )
{
	if( queryObj == 0 ) return;
	
	glDeleteQueries( 1, &queryObj );
}


void RenderDeviceGLES3::beginQuery( uint32 queryObj )
{
	glBeginQuery( GL_ANY_SAMPLES_PASSED, queryObj );
}


void RenderDeviceGLES3::endQuery( uint32 /*queryObj*/ )
{
	glEndQuery( GL_ANY_SAMPLES_PASSED );
}


uint32 RenderDeviceGLES3::getQueryResult( uint32 queryObj )
{
	uint32 samples = 0;
	glGetQueryObjectuiv( queryObj, GL_QUERY_RESULT, &samples );
	return samples;
}


// =================================================================================================
// Internal state management
// =================================================================================================

void RenderDeviceGLES3::checkError()
{
	uint32 error = glGetError();
	ASSERT( error != GL_INVALID_ENUM );
	ASSERT( error != GL_INVALID_VALUE );
	ASSERT( error != GL_INVALID_OPERATION );
	ASSERT( error != GL_OUT_OF_MEMORY );
}


bool RenderDeviceGLES3::applyVertexLayout( RDIGeometryInfoGLES3 &geo )
{
	uint32 newVertexAttribMask = 0;
	
	if( _curShaderId == 0 ) return false;
	
	RDIVertexLayout &vl = _vertexLayouts[ geo.layout - 1 ];
	RDIShaderGLES3 &shader = _shaders.getRef( _curShaderId );
	RDIInputLayoutGLES3 &inputLayout = shader.inputLayouts[ geo.layout - 1 ];
		
	if( !inputLayout.valid )
		return false;

	// Set vertex attrib pointers
	for( uint32 i = 0; i < vl.numAttribs; ++i )
	{
		int8 attribIndex = inputLayout.attribIndices[i];
		if( attribIndex >= 0 )
		{
			VertexLayoutAttrib &attrib = vl.attribs[i];
			const RDIVertBufSlotGLES3 &vbSlot = geo.vertexBufInfo[ attrib.vbSlot ];
				
			ASSERT( _buffers.getRef( geo.vertexBufInfo[ attrib.vbSlot ].vbObj ).glObj != 0 &&
					_buffers.getRef( geo.vertexBufInfo[ attrib.vbSlot ].vbObj ).type == GL_ARRAY_BUFFER );
					
			glBindBuffer( GL_ARRAY_BUFFER, _buffers.getRef( geo.vertexBufInfo[ attrib.vbSlot ].vbObj ).glObj );
			glVertexAttribPointer( attribIndex, attrib.size, GL_FLOAT, GL_FALSE,
									vbSlot.stride, (char *)0 + vbSlot.offset + attrib.offset );

			newVertexAttribMask |= 1 << attribIndex;
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

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	return true;
}


void RenderDeviceGLES3::applySamplerState( RDITextureGLES3 &tex )
{
	uint32 state = tex.samplerState;
	uint32 target = tex.type;
	
	const uint32 magFilters[] = { GL_LINEAR, GL_LINEAR, GL_NEAREST };
	const uint32 minFiltersMips[] = { GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST };
	const uint32 maxAniso[] = { 1, 2, 4, 0, 8, 0, 0, 0, 16 };
	const uint32 wrapModes[] = { GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_BORDER_EXT };

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
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	}
}


void RenderDeviceGLES3::applyRenderStates()
{
	// Rasterizer state
	if( _newRasterState.hash != _curRasterState.hash )
	{
		// Since OpenGL ES does not support glPolygonMode, we can only draw with lines for wireframe
		if ( _newRasterState.fillMode != RS_FILL_SOLID ) _drawType = PRIM_LINES;

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
			uint32 oglBlendFuncs[ 16 ] = { GL_ZERO, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,GL_DST_COLOR, GL_SRC_COLOR, 
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

	// Number of vertices in patch. Used in tesselation.
	if ( _tessPatchVerts != _lastTessPatchVertsValue )
	{
//		glPatchParameteri( GL_PATCH_VERTICES, _tessPatchVerts );

		_lastTessPatchVertsValue = _tessPatchVerts;
	}
}


void RenderDeviceGLES3::setStorageBuffer( uint8 slot, uint32 bufObj )
{
	ASSERT( slot < _maxComputeBufferAttachments && _storageBufs.size() < _maxComputeBufferAttachments );

	RDIBufferGLES3 &buf = _buffers.getRef( bufObj );
	_storageBufs.push_back( RDIShaderStorageGLES3( slot, buf.glObj ) );

	_pendingMask |= PM_COMPUTE;
}


bool RenderDeviceGLES3::commitStates( uint32 filter )
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

		// Bind textures and set sampler state
		if( mask & PM_TEXTURES )
		{
			for( uint32 i = 0; i < 16/*_texSlots.size()*/; ++i )
			{
				glActiveTexture( GL_TEXTURE0 + i );

				// add texture usage from GL4
				if( _texSlots[i].texObj != 0 )
				{
					RDITextureGLES3 &tex = _textures.getRef( _texSlots[i].texObj );
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
				RDIGeometryInfoGLES3 &geo = _vaos.getRef( _curGeometryIndex );

				glBindVertexArray( geo.vao );

				_indexFormat = geo.indexBuf32Bit;
// 				_curVertLayout = _newVertLayout;
				_prevShaderId = _curShaderId;
				_pendingMask &= ~PM_GEOMETRY;
			}
		}

		// Place memory barriers
		if ( mask & PM_BARRIER )
		{
			if ( _memBarriers != NotSet ) glMemoryBarrier( memoryBarrierType[ ( uint32 ) _memBarriers - 1 ] );
			_pendingMask &= ~PM_BARRIER;
		}

		// Bind storage buffers
		if ( mask & PM_COMPUTE )
		{
			for ( size_t i = 0; i < _storageBufs.size(); ++i )
			{
				glBindBufferBase( GL_SHADER_STORAGE_BUFFER, _storageBufs[ i ].slot, _storageBufs[ i ].oglObject );
			}

			_pendingMask &= ~PM_COMPUTE;
		}

		CHECK_GL_ERROR
	}

	return true;
}


void RenderDeviceGLES3::resetStates()
{
// 	_curIndexBuf = 1; _newIndexBuf = 0;
// 	_curVertLayout = 1; _newVertLayout = 0;
	_curGeometryIndex = 1;
	_curRasterState.hash = 0xFFFFFFFF; _newRasterState.hash = 0;
	_curBlendState.hash = 0xFFFFFFFF; _newBlendState.hash = 0;
	_curDepthStencilState.hash = 0xFFFFFFFF; _newDepthStencilState.hash = 0;

	_memBarriers = NotSet;

//	_texSlots.clear();
	for( uint32 i = 0; i < 16; ++i )
		setTexture( i, 0, 0, 0 );

	_storageBufs.clear();

	setColorWriteMask( true );
	_pendingMask = 0xFFFFFFFF;
	commitStates();

	glBindVertexArray( 0 );
 	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
}


// =================================================================================================
// Draw calls and clears
// =================================================================================================

void RenderDeviceGLES3::clear( uint32 flags, float *colorRGBA, float depth )
{
	uint32 prevBuffers[4] = { 0 };

	if( _curRendBuf != 0x0 )
	{
		RDIRenderBufferGLES3 &rb = _rendBufs.getRef( _curRendBuf );
		
		if( (flags & CLR_DEPTH) && rb.depthTex == 0 ) flags &= ~CLR_DEPTH;
		
		// Store state of glDrawBuffers
		for( uint32 i = 0; i < 4; ++i )
			glGetIntegerv( GL_DRAW_BUFFER0 + i, (int *)&prevBuffers[i] );
		
		uint32 buffers[4], cnt = 0;
		
		if( (flags & CLR_COLOR_RT0) && rb.colTexs[0] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT0;
		if( (flags & CLR_COLOR_RT1) && rb.colTexs[1] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT1;
		if( (flags & CLR_COLOR_RT2) && rb.colTexs[2] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT2;
		if( (flags & CLR_COLOR_RT3) && rb.colTexs[3] != 0 ) buffers[cnt++] = GL_COLOR_ATTACHMENT3;

		if( cnt == 0 )
			flags &= ~(CLR_COLOR_RT0 | CLR_COLOR_RT1 | CLR_COLOR_RT2 | CLR_COLOR_RT3);
		else
			glDrawBuffers( cnt, buffers );
	}
	
	uint32 oglClearMask = 0;
	
	if( flags & CLR_DEPTH )
	{
		oglClearMask |= GL_DEPTH_BUFFER_BIT;
		glClearDepthf( depth );
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


void RenderDeviceGLES3::draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
{
	_drawType = primType;

	if( commitStates() )
	{
		glDrawArrays( RDI_GLES3::primitiveTypes[ _drawType ], firstVert, numVerts );
	}

	CHECK_GL_ERROR
}


void RenderDeviceGLES3::drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
                                uint32 firstVert, uint32 numVerts )
{
	_drawType = primType;

	if( commitStates() )
	{
		firstIndex *= (_indexFormat == IDXFMT_16) ? sizeof( short ) : sizeof( int );
		
		glDrawRangeElements( RDI_GLES3::primitiveTypes[ _drawType ], firstVert, firstVert + numVerts,
							 numIndices, RDI_GLES3::indexFormats[ _indexFormat ], ( char * ) 0 + firstIndex );
	}

	CHECK_GL_ERROR
}

} // namespace RDI_GLES3
}  // namespace
