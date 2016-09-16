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

#include "egRendererBaseGL4.h"
#include "egModules.h"
#include "egCom.h"
#include "utOpenGL.h"

#include "utDebug.h"


namespace Horde3D {
namespace RDI_GL4 {

#ifdef H3D_VALIDATE_DRAWCALLS
#	define CHECK_GL_ERROR checkError();
#else
#	define CHECK_GL_ERROR
#endif

static const char *defaultShaderVS =
	"#version 330\n"
	"uniform mat4 viewProjMat;\n"
	"uniform mat4 worldMat;\n"
	"layout ( location = 0 ) in vec3 vertPos;\n"
	"void main() {\n"
	"	gl_Position = viewProjMat * worldMat * vec4( vertPos, 1.0 );\n"
	"}\n";

static const char *defaultShaderFS =
	"#version 330\n"
	"out vec4 fragColor;\n"
	"uniform vec4 color;\n"
	"void main() {\n"
	"	fragColor = color;\n"
	"}\n";

static const uint32 indexFormats[ 2 ] = { GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };

static const uint32 primitiveTypes[ 5 ] = { GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_LINES, GL_POINTS, GL_PATCHES };

static const uint32 textureTypes[ 3 ] = { GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };

// =================================================================================================
// GPUTimer
// =================================================================================================

GPUTimerGL4::GPUTimerGL4() : _numQueries( 0 ), _queryFrame( 0 ), _activeQuery( false )
{
	GPUTimer::initFunctions< GPUTimerGL4 >();
	reset();
}


GPUTimerGL4::~GPUTimerGL4()
{
	if( !_queryPool.empty() )
		glDeleteQueries( (uint32)_queryPool.size(), &_queryPool[0] );
}


void GPUTimerGL4::beginQuery( uint32 frameID )
{
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
	 glQueryCounter( queryObjs[0], GL_TIMESTAMP );
}


void GPUTimerGL4::endQuery()
{
	if( _activeQuery )
	{	
		glQueryCounter( _queryPool[_numQueries * 2 - 1], GL_TIMESTAMP );
		_activeQuery = false;
	}
}


bool GPUTimerGL4::updateResults()
{
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
		glGetQueryObjectui64v( _queryPool[i * 2], GL_QUERY_RESULT, &timeStart );
		glGetQueryObjectui64v( _queryPool[i * 2 + 1], GL_QUERY_RESULT, &timeEnd );
		timeAccum += timeEnd - timeStart;
	}
	
	_time = (float)((double)timeAccum / 1000000.0);
	return true;
}


void GPUTimerGL4::reset()
{
	_time = 0.f;
}


// =================================================================================================
// RenderDevice
// =================================================================================================

RenderDeviceGL4::RenderDeviceGL4()
{
	RenderDeviceInterface::initRDIFunctions< RenderDeviceGL4 >();

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

	_maxTexSlots = 96; // for most modern hardware it is 192 (GeForce 400+, Radeon 7000+, Intel 4000+). Although 96 should probably be enough.
// 	_texSlots.reserve( _maxTexSlots ); // reserve memory

	// add default geometry for resetting
	RDIGeometryInfoGL4 defGeom;
	defGeom.atrribsBinded = true;
	_vaos.add( defGeom );
}


RenderDeviceGL4::~RenderDeviceGL4()
{
}


void RenderDeviceGL4::initStates()
{
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	GLint value;
	glGetIntegerv( GL_SAMPLE_BUFFERS, &value );
	_defaultFBOMultisampled = value > 0;
}


bool RenderDeviceGL4::init()
{
	bool failed = false;

	char *vendor = (char *)glGetString( GL_VENDOR );
	char *renderer = (char *)glGetString( GL_RENDERER );
	char *version = (char *)glGetString( GL_VERSION );
	
	Modules::log().writeInfo( "Initializing GL4 backend using OpenGL driver '%s' by '%s' on '%s'",
	                          version, vendor, renderer );
	
	// Init extensions
	if( !initOpenGLExtensions( false ) )
	{	
		Modules::log().writeError( "Could not find all required OpenGL function entry points" );
		failed = true;
	}

	// Check that OpenGL 3.3 is available
	if( glExt::majorVersion * 10 + glExt::minorVersion < 33 )
	{
		Modules::log().writeError( "OpenGL 3.3 not available" );
		failed = true;
	}
	
	// Check that required extensions are supported
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
	
	if( failed )
	{
		Modules::log().writeError( "Failed to init renderer backend (OpenGL %d.%d), retrying with legacy OpenGL 2.1 backend", 
									glExt::majorVersion, glExt::minorVersion );
// 		char *exts = (char *)glGetString( GL_EXTENSIONS );
// 		Modules::log().writeInfo( "Supported extensions: '%s'", exts );

		return false;
	}
	
	// Set capabilities
	_caps.texFloat = true;
	_caps.texNPOT = true;
	_caps.rtMultisampling = true;
	_caps.geometryShaders = true;
	_caps.tesselation = glExt::majorVersion >= 4 && glExt::minorVersion >= 1;
	_caps.computeShaders = glExt::majorVersion >= 4 && glExt::minorVersion >= 3;
	_caps.instancing = true;
	_caps.maxJointCount = 75; // currently, will be changed soon
	_caps.maxTexUnitCount = 96; // for most modern hardware it is 192 (GeForce 400+, Radeon 7000+, Intel 4000+). Although 96 should probably be enough.

	// Find supported depth format (some old ATI cards only support 16 bit depth for FBOs)
	_depthFormat = GL_DEPTH_COMPONENT24;
	uint32 testBuf = createRenderBuffer( 32, 32, TextureFormats::BGRA8, true, 1, 0 ); 
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

uint32 RenderDeviceGL4::registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs )
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

void RenderDeviceGL4::beginRendering()
{	
	//	Get the currently bound frame buffer object. 
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &_defaultFBO );
	resetStates();
}

uint32 RenderDeviceGL4::beginCreatingGeometry( uint32 vlObj )
{
	RDIGeometryInfoGL4 vao;
	vao.layout = vlObj;

	uint32 vaoID;
	glGenVertexArrays( 1, &vaoID );
	vao.vao = vaoID;

	return _vaos.add( vao );
}

void RenderDeviceGL4::finishCreatingGeometry( uint32 geoObj )
{
	ASSERT( geoObj > 0 )
	
	RDIGeometryInfoGL4 &curVao = _vaos.getRef( geoObj );
	glBindVertexArray( curVao.vao );

	// bind index buffer, if present
	if ( curVao.indexBuf )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, curVao.indexBuf );
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
			const RDIVertBufSlotGL4 &vbSlot = curVao.vertexBufInfo[ attrib.vbSlot ];

			ASSERT( _buffers.getRef( curVao.vertexBufInfo[ attrib.vbSlot ].vbObj ).glObj != 0 &&
					_buffers.getRef( curVao.vertexBufInfo[ attrib.vbSlot ].vbObj ).type == GL_ARRAY_BUFFER );

			glBindBuffer( GL_ARRAY_BUFFER, _buffers.getRef( curVao.vertexBufInfo[ attrib.vbSlot ].vbObj ).glObj );
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

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
}

void RenderDeviceGL4::setGeomVertexParams( uint32 geoObj, uint32 vbo, uint32 vbSlot, uint32 offset, uint32 stride )
{
	RDIGeometryInfoGL4 &curVao = _vaos.getRef( geoObj );

	RDIVertBufSlotGL4 attribInfo;
	attribInfo.vbObj = vbo;
	attribInfo.offset = offset;
	attribInfo.stride = stride;

	curVao.vertexBufInfo.push_back( attribInfo );
}

void RenderDeviceGL4::setGeomIndexParams( uint32 geoObj, uint32 indBuf, RDIIndexFormat format )
{
	RDIGeometryInfoGL4 &curVao = _vaos.getRef( geoObj );

	curVao.indexBuf = indBuf;
	curVao.indexBuf32Bit = ( format == IDXFMT_32 ? true : false );
}

void RenderDeviceGL4::destroyGeometry( uint32 geoObj )
{
	if ( geoObj == 0 ) return;
	
	RDIGeometryInfoGL4 &curVao = _vaos.getRef( geoObj );
	destroyBuffer( curVao.indexBuf );
	
	for ( unsigned int i = 0; i < curVao.vertexBufInfo.size(); ++i )
	{
		destroyBuffer( curVao.vertexBufInfo[ i ].vbObj );
	}

	glDeleteVertexArrays( 1, &geoObj );
}

uint32 RenderDeviceGL4::createVertexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ARRAY_BUFFER, size, data );
}


uint32 RenderDeviceGL4::createIndexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ELEMENT_ARRAY_BUFFER, size, data );
}


uint32 RenderDeviceGL4::createShaderStorageBuffer( uint32 size, const void *data )
{
	if ( _caps.computeShaders )
		return createBuffer( GL_SHADER_STORAGE_BUFFER, size, data );
	else
	{
		Modules::log().writeError( "Shader storage buffers are not supported on this OpenGL 4 device." );
		
		return 0;
	}
}


uint32 RenderDeviceGL4::createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data )
{
	RDITextureBufferGL4 buf;

	buf.bufObj = createBuffer( GL_TEXTURE_BUFFER, bufSize, data );

	glGenTextures( 1, &buf.glTexID );
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( GL_TEXTURE_BUFFER, buf.glTexID );

	switch ( format )
	{
		case TextureFormats::BGRA8:
			buf.glFmt = GL_RGBA8;
			break;
		case TextureFormats::RGBA16F:
			buf.glFmt = GL_RGBA16F;
			break;
		case TextureFormats::RGBA32F:
			buf.glFmt = GL_RGBA32F;
			break;
		case TextureFormats::R32:
			buf.glFmt = GL_R32F;
			break;
		case TextureFormats::RG32:
			buf.glFmt = GL_RG32F;
			break;
		default:
			ASSERT( 0 );
			break;
	};

	// bind texture to buffer
	glTexBuffer( GL_TEXTURE_BUFFER, buf.glFmt, _buffers.getRef( buf.bufObj ).glObj );

	glBindTexture( GL_TEXTURE_BUFFER, 0 );
	if ( _texSlots[ 15 ].texObj )
		glBindTexture( _textures.getRef( _texSlots[ 15 ].texObj ).type, _textures.getRef( _texSlots[ 15 ].texObj ).glObj );

	return _textureBuffs.add( buf );
}


uint32 RenderDeviceGL4::createBuffer( uint32 bufType, uint32 size, const void *data )
{
	RDIBufferGL4 buf;

	buf.type = bufType;
	buf.size = size;
	glGenBuffers( 1, &buf.glObj );
	glBindBuffer( buf.type, buf.glObj );
	glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );
	glBindBuffer( buf.type, 0 );

	_bufferMem += size;
	return _buffers.add( buf );
}


void RenderDeviceGL4::destroyBuffer( uint32 bufObj )
{
	if( bufObj == 0 ) return;
	
	RDIBufferGL4 &buf = _buffers.getRef( bufObj );
	glDeleteBuffers( 1, &buf.glObj );

	_bufferMem -= buf.size;
	_buffers.remove( bufObj );
}


void RenderDeviceGL4::destroyTextureBuffer( uint32 bufObj )
{
	if ( bufObj == 0 ) return;

	RDITextureBufferGL4 &buf = _textureBuffs.getRef( bufObj );
	destroyBuffer( buf.bufObj );

	glDeleteTextures( 1, &buf.glTexID );

	_textureBuffs.remove( bufObj );
}

void RenderDeviceGL4::updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data )
{
//	const RDIGeometryInfoGL4 &geo = _vaos.getRef( geoObj );
	const RDIBufferGL4 &buf = _buffers.getRef( bufObj );
	ASSERT( offset + size <= buf.size );
	
//	glBindVertexArray( geo.vao );

	glBindBuffer( buf.type, buf.glObj );
	
	if( offset == 0 && size == buf.size )
	{
		// Replacing the whole buffer can help the driver to avoid pipeline stalls
		glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );

// 		glBindBuffer( buf.type, 0 );
// 		glBindVertexArray( 0 );

		return;
	}
	
//	void *bufMem = glMapBufferRange( buf.type, offset, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );

// 	memcpy( bufMem, data, size );
// 
// 	glUnmapBuffer( buf.type );
	glBufferSubData( buf.type, offset, size, data );

// 	glBindBuffer( buf.type, 0 );
// 	glBindVertexArray( 0 );
}


// =================================================================================================
// Textures
// =================================================================================================

uint32 RenderDeviceGL4::calcTextureSize( TextureFormats::List format, int width, int height, int depth )
{
	switch( format )
	{
	case TextureFormats::BGRA8:
		return width * height * depth * 4;
	case TextureFormats::DXT1:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 8;
	case TextureFormats::DXT3:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 16;
	case TextureFormats::DXT5:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 16;
	case TextureFormats::RGBA16F:
		return width * height * depth * 8;
	case TextureFormats::RGBA32F:
		return width * height * depth * 16;
	default:
		return 0;
	}
}


uint32 RenderDeviceGL4::createTexture( TextureTypes::List type, int width, int height, int depth,
                                    TextureFormats::List format,
                                    bool hasMips, bool genMips, bool compress, bool sRGB )
{
	ASSERT( depth > 0 );

	if( !_caps.texNPOT )
	{
		// Check if texture is NPOT
		if( (width & (width-1)) != 0 || (height & (height-1)) != 0 )
			Modules::log().writeWarning( "Texture has non-power-of-two dimensions although NPOT is not supported by GPU" );
	}
	
	RDITextureGL4 tex;
	tex.type = textureTypes[ type ];
	tex.format = format;
	tex.width = width;
	tex.height = height;
	tex.depth = depth;
	tex.sRGB = sRGB && Modules::config().sRGBLinearization;
	tex.genMips = genMips;
	tex.hasMips = hasMips;
	
	switch( format )
	{
	case TextureFormats::BGRA8:
		tex.glFmt = tex.sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		break;
	case TextureFormats::DXT1:
		tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case TextureFormats::DXT3:
		tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormats::DXT5:
		tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case TextureFormats::RGBA16F:
		tex.glFmt = GL_RGBA16F;
		break;
	case TextureFormats::RGBA32F:
		tex.glFmt = GL_RGBA32F;
		break;
	case TextureFormats::DEPTH:
		tex.glFmt = _depthFormat;
		break;
	default:
		ASSERT( 0 );
		break;
	};
	
	glGenTextures( 1, &tex.glObj );
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );
	
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


void RenderDeviceGL4::uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	const RDITextureGL4 &tex = _textures.getRef( texObj );
	TextureFormats::List format = tex.format;

	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	
	int inputFormat = GL_BGRA, inputType = GL_UNSIGNED_BYTE;
	bool compressed = (format == TextureFormats::DXT1) || (format == TextureFormats::DXT3) ||
	                  (format == TextureFormats::DXT5);
	
	switch( format )
	{
	case TextureFormats::RGBA16F:
		inputFormat = GL_RGBA;
		inputType = GL_FLOAT;
		break;
	case TextureFormats::RGBA32F:
		inputFormat = GL_RGBA;
		inputType = GL_FLOAT;
		break;
	case TextureFormats::DEPTH:
		inputFormat = GL_DEPTH_COMPONENT;
		inputType = GL_FLOAT;
	};
	
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
		glEnable( tex.type );  // Workaround for ATI driver bug
		glGenerateMipmap( tex.type );
		glDisable( tex.type );
	}

	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
}


void RenderDeviceGL4::destroyTexture( uint32 texObj )
{
	if( texObj == 0 ) return;
	
	const RDITextureGL4 &tex = _textures.getRef( texObj );
	glDeleteTextures( 1, &tex.glObj );

	_textureMem -= tex.memSize;
	_textures.remove( texObj );
}


void RenderDeviceGL4::updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	uploadTextureData( texObj, slice, mipLevel, pixels );
}


bool RenderDeviceGL4::getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer )
{
	const RDITextureGL4 &tex = _textures.getRef( texObj );
	
	int target = tex.type == textureTypes[ TextureTypes::TexCube ] ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
	if( target == GL_TEXTURE_CUBE_MAP ) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;
	
	int fmt, type, compressed = 0;
	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );

	switch( tex.format )
	{
	case TextureFormats::BGRA8:
		fmt = GL_BGRA;
		type = GL_UNSIGNED_BYTE;
		break;
	case TextureFormats::DXT1:
	case TextureFormats::DXT3:
	case TextureFormats::DXT5:
		compressed = 1;
		break;
	case TextureFormats::RGBA16F:
	case TextureFormats::RGBA32F:
		fmt = GL_RGBA;
		type = GL_FLOAT;
		break;
	default:
		return false;
	};

	if( compressed )
		glGetCompressedTexImage( target, mipLevel, buffer );
	else
		glGetTexImage( target, mipLevel, fmt, type, buffer );

	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );

	return true;
}


// =================================================================================================
// Shaders
// =================================================================================================

uint32 RenderDeviceGL4::createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc, 
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
		uint32 gs = glCreateShader( GL_GEOMETRY_SHADER );
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
	if ( tessControlShaderSrc )
	{
		tsC = glCreateShader( GL_TESS_CONTROL_SHADER );
		glShaderSource( tsC, 1, &tessControlShaderSrc, 0x0 );
		glCompileShader( tsC );
		glGetShaderiv( tsC, GL_COMPILE_STATUS, &status );
		if ( !status )
		{
			glGetShaderiv( tsC, GL_INFO_LOG_LENGTH, &infologLength );
			if ( infologLength > 1 )
			{
				infoLog = new char[ infologLength ];
				glGetShaderInfoLog( tsC, infologLength, &charsWritten, infoLog );
				_shaderLog = _shaderLog + "[Tesselation Control Shader]\n" + infoLog;
				delete[] infoLog; infoLog = 0x0;
			}

			glDeleteShader( vs );
			glDeleteShader( fs );
			if ( gs ) glDeleteShader( gs );
			glDeleteShader( tsC );
			return 0;
		}
	}

	// Tesselation evaluation shader
	if ( tessEvalShaderSrc )
	{
		tsE = glCreateShader( GL_TESS_EVALUATION_SHADER );
		glShaderSource( tsE, 1, &tessEvalShaderSrc, 0x0 );
		glCompileShader( tsE );
		glGetShaderiv( tsE, GL_COMPILE_STATUS, &status );
		if ( !status )
		{
			glGetShaderiv( tsE, GL_INFO_LOG_LENGTH, &infologLength );
			if ( infologLength > 1 )
			{
				infoLog = new char[ infologLength ];
				glGetShaderInfoLog( tsE, infologLength, &charsWritten, infoLog );
				_shaderLog = _shaderLog + "[Tesselation Evaluation Shader]\n" + infoLog;
				delete[] infoLog; infoLog = 0x0;
			}

			glDeleteShader( vs );
			glDeleteShader( fs );
			if ( gs ) glDeleteShader( gs );
			glDeleteShader( tsC );
			glDeleteShader( tsE );
			return 0;
		}
	}

	// Tesselation evaluation shader
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


bool RenderDeviceGL4::linkShaderProgram( uint32 programObj )
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


uint32 RenderDeviceGL4::createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
									  const char *tessControlShaderSrc, const char *tessEvaluationShaderSrc, const char *computeShaderSrc )
{
	// Compile and link shader
	uint32 programObj = createShaderProgram( vertexShaderSrc, fragmentShaderSrc, geometryShaderSrc, tessControlShaderSrc, tessEvaluationShaderSrc, computeShaderSrc );
	if( programObj == 0 ) return 0;
	if( !linkShaderProgram( programObj ) ) return 0;

	int loc = glGetFragDataLocation( programObj, "fragColor" );

	uint32 shaderId = _shaders.add( RDIShaderGL4() );
	RDIShaderGL4 &shader = _shaders.getRef( shaderId );
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


void RenderDeviceGL4::destroyShader( uint32 shaderId )
{
	if( shaderId == 0 ) return;

	RDIShaderGL4 &shader = _shaders.getRef( shaderId );
	glDeleteProgram( shader.oglProgramObj );
	_shaders.remove( shaderId );
}


void RenderDeviceGL4::bindShader( uint32 shaderId )
{
	if( shaderId != 0 )
	{
		RDIShaderGL4 &shader = _shaders.getRef( shaderId );
		glUseProgram( shader.oglProgramObj );
	}
	else
	{
		glUseProgram( 0 );
	}
	
	_curShaderId = shaderId;
	_pendingMask |= PM_GEOMETRY;
} 


int RenderDeviceGL4::getShaderConstLoc( uint32 shaderId, const char *name )
{
	RDIShaderGL4 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


int RenderDeviceGL4::getShaderSamplerLoc( uint32 shaderId, const char *name )
{
	RDIShaderGL4 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


void RenderDeviceGL4::setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count )
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
	}
}


void RenderDeviceGL4::setShaderSampler( int loc, uint32 texUnit )
{
	glUniform1i( loc, (int)texUnit );
}


const char *RenderDeviceGL4::getDefaultVSCode()
{
	return defaultShaderVS;
}


const char *RenderDeviceGL4::getDefaultFSCode()
{
	return defaultShaderFS;
}


void RenderDeviceGL4::runComputeShader( uint32 shaderId, uint32 xDim, uint32 yDim, uint32 zDim )
{
	bindShader( shaderId );

	glDispatchCompute( xDim, yDim, zDim );
}

// =================================================================================================
// Renderbuffers
// =================================================================================================

uint32 RenderDeviceGL4::createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
                                         bool depth, uint32 numColBufs, uint32 samples )
{
	if( (format == TextureFormats::RGBA16F || format == TextureFormats::RGBA32F) && !_caps.texFloat )
	{
		return 0;
	}

	if( numColBufs > RDIRenderBufferGL4::MaxColorAttachmentCount ) return 0;

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

	RDIRenderBufferGL4 rb;
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
			RDITextureGL4 &tex = _textures.getRef( texObj );
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
	else
	{	
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		glDrawBuffer( GL_NONE );
		glReadBuffer( GL_NONE );
		
		if( samples > 0 )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );
		}
	}

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
		RDITextureGL4 &tex = _textures.getRef( texObj );
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
}


void RenderDeviceGL4::destroyRenderBuffer( uint32 rbObj )
{
	RDIRenderBufferGL4 &rb = _rendBufs.getRef( rbObj );
	
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
	
	if( rb.depthTex != 0 ) destroyTexture( rb.depthTex );
	if( rb.depthBuf != 0 ) glDeleteRenderbuffers( 1, &rb.depthBuf );
	rb.depthTex = rb.depthBuf = 0;
		
	for( uint32 i = 0; i < RDIRenderBufferGL4::MaxColorAttachmentCount; ++i )
	{
		if( rb.colTexs[i] != 0 ) destroyTexture( rb.colTexs[i] );
		if( rb.colBufs[i] != 0 ) glDeleteRenderbuffers( 1, &rb.colBufs[i] );
		rb.colTexs[i] = rb.colBufs[i] = 0;
	}

	if( rb.fbo != 0 ) glDeleteFramebuffers( 1, &rb.fbo );
	if( rb.fboMS != 0 ) glDeleteFramebuffers( 1, &rb.fboMS );
	rb.fbo = rb.fboMS = 0;

	_rendBufs.remove( rbObj );
}


void RenderDeviceGL4::getRenderBufferDimensions( uint32 rbObj, int *width, int *height )
{
	RDIRenderBufferGL4 &rb = _rendBufs.getRef( rbObj );

	*width = rb.width;
	*height = rb.height;
}


uint32 RenderDeviceGL4::getRenderBufferTex( uint32 rbObj, uint32 bufIndex )
{
	RDIRenderBufferGL4 &rb = _rendBufs.getRef( rbObj );
	
	if( bufIndex < RDIRenderBufferGL4::MaxColorAttachmentCount ) return rb.colTexs[bufIndex];
	else if( bufIndex == 32 ) return rb.depthTex;
	else return 0;
}


void RenderDeviceGL4::resolveRenderBuffer( uint32 rbObj )
{
	RDIRenderBufferGL4 &rb = _rendBufs.getRef( rbObj );
	
	if( rb.fboMS == 0 ) return;
	
	glBindFramebuffer( GL_READ_FRAMEBUFFER, rb.fboMS );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, rb.fbo );

	bool depthResolved = false;
	for( uint32 i = 0; i < RDIRenderBufferGL4::MaxColorAttachmentCount; ++i )
	{
		if( rb.colBufs[i] != 0 )
		{
			glReadBuffer( GL_COLOR_ATTACHMENT0 + i );
			glDrawBuffer( GL_COLOR_ATTACHMENT0 + i );
			
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
		glReadBuffer( GL_NONE );
		glDrawBuffer( GL_NONE );
		glBlitFramebuffer( 0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height,
							  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST );
	}

	glBindFramebuffer( GL_READ_FRAMEBUFFER, _defaultFBO );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, _defaultFBO );
}


void RenderDeviceGL4::setRenderBuffer( uint32 rbObj )
{
	// Resolve render buffer if necessary
	if( _curRendBuf != 0 ) resolveRenderBuffer( _curRendBuf );
	
	// Set new render buffer
	_curRendBuf = rbObj;
	
	if( rbObj == 0 )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
		if( _defaultFBO == 0 ) glDrawBuffer( _outputBufferIndex == 1 ? GL_BACK_RIGHT : GL_BACK_LEFT );
		_fbWidth = _vpWidth + _vpX;
		_fbHeight = _vpHeight + _vpY;
        if( _defaultFBOMultisampled ) glEnable( GL_MULTISAMPLE );
		else glDisable( GL_MULTISAMPLE );
	}
	else
	{
		// Unbind all textures to make sure that no FBO attachment is bound any more
		for( uint32 i = 0; i < 16; ++i ) setTexture( i, 0, 0 );
		commitStates( PM_TEXTURES );
		
		RDIRenderBufferGL4 &rb = _rendBufs.getRef( rbObj );

		glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS != 0 ? rb.fboMS : rb.fbo );
		ASSERT( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
		_fbWidth = rb.width;
		_fbHeight = rb.height;

		if( rb.fboMS != 0 ) glEnable( GL_MULTISAMPLE );
		else glDisable( GL_MULTISAMPLE );
	}
}


bool RenderDeviceGL4::getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
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
		if( bufIndex != 32 ) glReadBuffer( GL_BACK_LEFT );
		//format = GL_BGRA;
		//type = GL_UNSIGNED_BYTE;
	}
	else
	{
		resolveRenderBuffer( rbObj );
		RDIRenderBufferGL4 &rb = _rendBufs.getRef( rbObj );
		
		if( bufIndex == 32 && rb.depthTex == 0 ) return false;
		if( bufIndex != 32 )
		{
			if( (unsigned)bufIndex >= RDIRenderBufferGL4::MaxColorAttachmentCount || rb.colTexs[bufIndex] == 0 )
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

uint32 RenderDeviceGL4::createOcclusionQuery()
{
	uint32 queryObj;
	glGenQueries( 1, &queryObj );
	return queryObj;
}


void RenderDeviceGL4::destroyQuery( uint32 queryObj )
{
	if( queryObj == 0 ) return;
	
	glDeleteQueries( 1, &queryObj );
}


void RenderDeviceGL4::beginQuery( uint32 queryObj )
{
	glBeginQuery( GL_SAMPLES_PASSED, queryObj );
}


void RenderDeviceGL4::endQuery( uint32 /*queryObj*/ )
{
	glEndQuery( GL_SAMPLES_PASSED );
}


uint32 RenderDeviceGL4::getQueryResult( uint32 queryObj )
{
	uint32 samples = 0;
	glGetQueryObjectuiv( queryObj, GL_QUERY_RESULT, &samples );
	return samples;
}


// =================================================================================================
// Internal state management
// =================================================================================================

void RenderDeviceGL4::checkError()
{
	uint32 error = glGetError();
	ASSERT( error != GL_INVALID_ENUM );
	ASSERT( error != GL_INVALID_VALUE );
	ASSERT( error != GL_INVALID_OPERATION );
	ASSERT( error != GL_OUT_OF_MEMORY );
	ASSERT( error != GL_STACK_OVERFLOW && error != GL_STACK_UNDERFLOW );
}


bool RenderDeviceGL4::applyVertexLayout( RDIGeometryInfoGL4 &geo )
{
	uint32 newVertexAttribMask = 0;
	
	if( _curShaderId == 0 ) return false;
	
	RDIVertexLayout &vl = _vertexLayouts[ geo.layout - 1 ];
	RDIShaderGL4 &shader = _shaders.getRef( _curShaderId );
	RDIInputLayoutGL4 &inputLayout = shader.inputLayouts[ geo.layout - 1 ];
		
	if( !inputLayout.valid )
		return false;

	// Set vertex attrib pointers
	for( uint32 i = 0; i < vl.numAttribs; ++i )
	{
		int8 attribIndex = inputLayout.attribIndices[i];
		if( attribIndex >= 0 )
		{
			VertexLayoutAttrib &attrib = vl.attribs[i];
			const RDIVertBufSlotGL4 &vbSlot = geo.vertexBufInfo[ attrib.vbSlot ];
				
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


void RenderDeviceGL4::applySamplerState( RDITextureGL4 &tex )
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


void RenderDeviceGL4::applyRenderStates()
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
			uint32 oglBlendFuncs[8] = { GL_ZERO, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_COLOR, GL_ZERO, GL_ZERO };
			
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
		glPatchParameteri( GL_PATCH_VERTICES, _tessPatchVerts );

		_lastTessPatchVertsValue = _tessPatchVerts;
	}
}


bool RenderDeviceGL4::commitStates( uint32 filter )
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

				if( _texSlots[i].texObj != 0 )
				{
					RDITextureGL4 &tex = _textures.getRef( _texSlots[i].texObj );
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
				RDIGeometryInfoGL4 &geo = _vaos.getRef( _curGeometryIndex );

				glBindVertexArray( geo.vao );

				_indexFormat = geo.indexBuf32Bit;
// 				_curVertLayout = _newVertLayout;
				_prevShaderId = _curShaderId;
				_pendingMask &= ~PM_GEOMETRY;
			}
		}

		CHECK_GL_ERROR
	}

	return true;
}


void RenderDeviceGL4::resetStates()
{
// 	_curIndexBuf = 1; _newIndexBuf = 0;
// 	_curVertLayout = 1; _newVertLayout = 0;
	_curGeometryIndex = 1;
	_curRasterState.hash = 0xFFFFFFFF; _newRasterState.hash = 0;
	_curBlendState.hash = 0xFFFFFFFF; _newBlendState.hash = 0;
	_curDepthStencilState.hash = 0xFFFFFFFF; _newDepthStencilState.hash = 0;

//	_texSlots.clear();
	for( uint32 i = 0; i < 16; ++i )
		setTexture( i, 0, 0 );

	setColorWriteMask( true );
	_pendingMask = 0xFFFFFFFF;
	commitStates();

 	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
}


// =================================================================================================
// Draw calls and clears
// =================================================================================================

void RenderDeviceGL4::clear( uint32 flags, float *colorRGBA, float depth )
{
	uint32 prevBuffers[4] = { 0 };

	if( _curRendBuf != 0x0 )
	{
		RDIRenderBufferGL4 &rb = _rendBufs.getRef( _curRendBuf );
		
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


void RenderDeviceGL4::draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
{
	if( commitStates() )
	{
		glDrawArrays( RDI_GL4::primitiveTypes[ ( uint32 ) primType ], firstVert, numVerts );
	}

	CHECK_GL_ERROR
}


void RenderDeviceGL4::drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
                                uint32 firstVert, uint32 numVerts )
{
	if( commitStates() )
	{
		firstIndex *= (_indexFormat == IDXFMT_16) ? sizeof( short ) : sizeof( int );
		
		glDrawRangeElements( RDI_GL4::primitiveTypes[ ( uint32 ) primType ], firstVert, firstVert + numVerts,
							 numIndices, RDI_GL4::indexFormats[ _indexFormat ], ( char * ) 0 + firstIndex );
	}

	CHECK_GL_ERROR
}

} // namespace RDI_GL4
}  // namespace
