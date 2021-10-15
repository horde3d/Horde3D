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


// =================================================================================================
// GPUTimer
// =================================================================================================

GPUTimerNull::GPUTimerNull() : _numQueries( 0 ),  _queryFrame( 0 ), _activeQuery( false )
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


void GPUTimerNull::endQuery()
{
	if( _activeQuery )
	{	
		glQueryCounterARB( _queryPool[_numQueries * 2 - 1], GL_TIMESTAMP );
		_activeQuery = false;
	}
}


bool GPUTimerNull::updateResults()
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
		glGetQueryObjectui64vARB( _queryPool[i * 2], GL_QUERY_RESULT, &timeStart );
		glGetQueryObjectui64vARB( _queryPool[i * 2 + 1], GL_QUERY_RESULT, &timeEnd );
		timeAccum += timeEnd - timeStart;
	}
	
	_time = (float)((double)timeAccum / 1000000.0);
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


void RenderDeviceNull::decreaseBufferRefCount( uint32 bufObj )
{
	if ( bufObj == 0 ) return;

	RDIBufferGL2 &buf = _buffers.getRef( bufObj );

	buf.geometryRefCount--;
}


uint32 RenderDeviceNull::createVertexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ARRAY_BUFFER, size, data );
}


uint32 RenderDeviceNull::createIndexBuffer( uint32 size, const void *data )
{
	return createBuffer( GL_ELEMENT_ARRAY_BUFFER, size, data );;
}


uint32 RenderDeviceNull::createTextureBuffer( TextureFormats::List format, uint32 bufSize, const void *data )
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


uint32 RenderDeviceNull::createShaderStorageBuffer( uint32 size, const void *data )
{
	H3D_UNUSED_VAR( size );
	H3D_UNUSED_VAR( data );

	Modules::log().writeError( "Shader storage buffers are not supported on OpenGL 2 devices." );

	return 0;
}


uint32 RenderDeviceNull::createBuffer( uint32 bufType, uint32 size, const void *data )
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


void RenderDeviceNull::destroyBuffer( uint32& bufObj )
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


void RenderDeviceNull::destroyTextureBuffer( uint32& bufObj )
{
	
}


void RenderDeviceNull::updateBufferData( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, void *data )
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


void * RenderDeviceNull::mapBuffer( uint32 geoObj, uint32 bufObj, uint32 offset, uint32 size, RDIBufferMappingTypes mapType )
{
	const RDIBufferGL2 &buf = _buffers.getRef( bufObj );
	ASSERT( offset + size <= buf.size );

	glBindBuffer( buf.type, buf.glObj );

	return glMapBuffer( buf.type, bufferMappingTypes[ mapType ] );
}


void RenderDeviceNull::unmapBuffer( uint32 geoObj, uint32 bufObj )
{
	const RDIBufferGL2 &buf = _buffers.getRef( bufObj );

	// multiple buffers can be mapped at the same time, so bind the one that needs to be unmapped
	glBindBuffer( buf.type, buf.glObj );

	glUnmapBuffer( buf.type );
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


void RenderDeviceNull::generateTextureMipmap( uint32 texObj )
{
	const RDITextureGL2 &tex = _textures.getRef( texObj );

	glActiveTexture( GL_TEXTURE15 );
	glBindTexture( tex.type, tex.glObj );
	glGenerateMipmapEXT( tex.type );
	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
}


void RenderDeviceNull::uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
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


void RenderDeviceNull::destroyTexture( uint32& texObj )
{
	if( texObj == 0 )
		return;
	
	const RDITextureGL2 &tex = _textures.getRef( texObj );
	if( tex.glObj ) glDeleteTextures( 1, &tex.glObj );

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

void RenderDeviceNull::bindImageToTexture(uint32 texObj, void *eglImage)
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

uint32 RenderDeviceNull::createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc )
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


bool RenderDeviceNull::linkShaderProgram( uint32 programObj )
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


uint32 RenderDeviceNull::createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc, const char *geometryShaderSrc,
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


void RenderDeviceNull::destroyShader( uint32& shaderId )
{
	if( shaderId == 0 )
		return;

	RDIShaderGL2 &shader = _shaders.getRef( shaderId );
	glDeleteProgram( shader.oglProgramObj );
	_shaders.remove( shaderId );
	shaderId = 0;
}


void RenderDeviceNull::bindShader( uint32 shaderId )
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


int RenderDeviceNull::getShaderConstLoc( uint32 shaderId, const char *name )
{
	RDIShaderGL2 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


int RenderDeviceNull::getShaderSamplerLoc( uint32 shaderId, const char *name )
{
	RDIShaderGL2 &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


int RenderDeviceNull::getShaderBufferLoc( uint32 shaderId, const char *name )
{
	H3D_UNUSED_VAR( shaderId );
	H3D_UNUSED_VAR( name );

	// Not supported on OpenGL 2
	return -1;
}

void RenderDeviceNull::setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count )
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


void RenderDeviceNull::setShaderSampler( int loc, uint32 texUnit )
{
	glUniform1i( loc, (int)texUnit );
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
	H3D_UNUSED_VAR( shaderId );
	H3D_UNUSED_VAR( xDim );
	H3D_UNUSED_VAR( yDim );
	H3D_UNUSED_VAR( zDim );

	Modules::log().writeError( "Compute shaders are not supported on OpenGL 2 render device." );
}

// =================================================================================================
// Renderbuffers
// =================================================================================================

uint32 RenderDeviceNull::createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
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
			uploadTextureData( texObj, 0, 0, 0x0 );
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
		uploadTextureData( texObj, 0, 0, 0x0 );
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


void RenderDeviceNull::destroyRenderBuffer( uint32& rbObj )
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


uint32 RenderDeviceNull::getRenderBufferTex( uint32 rbObj, uint32 bufIndex )
{
	RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );
	
	if( bufIndex < RDIRenderBufferGL2::MaxColorAttachmentCount ) return rb.colTexs[bufIndex];
	else if( bufIndex == 32 ) return rb.depthTex;
	else return 0;
}

void RenderDeviceNull::getRenderBufferDimensions( uint32 rbObj, int *width, int *height )
{
	RDIRenderBufferGL2 &rb = _rendBufs.getRef( rbObj );
	
	*width = rb.width;
	*height = rb.height;
}

void RenderDeviceNull::resolveRenderBuffer( uint32 rbObj )
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


void RenderDeviceNull::setRenderBuffer( uint32 rbObj )
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


bool RenderDeviceNull::getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
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

uint32 RenderDeviceNull::createOcclusionQuery()
{
	uint32 queryObj;
	glGenQueries( 1, &queryObj );
	return queryObj;
}


void RenderDeviceNull::destroyQuery( uint32 queryObj )
{
	if( queryObj == 0 ) return;
	
	glDeleteQueries( 1, &queryObj );
}


void RenderDeviceNull::beginQuery( uint32 queryObj )
{
	glBeginQuery( GL_SAMPLES_PASSED, queryObj );
}


void RenderDeviceNull::endQuery( uint32 /*queryObj*/ )
{
	glEndQuery( GL_SAMPLES_PASSED );
}


uint32 RenderDeviceNull::getQueryResult( uint32 queryObj )
{
	uint32 samples = 0;
	glGetQueryObjectuiv( queryObj, GL_QUERY_RESULT, &samples );
	return samples;
}


// =================================================================================================
// Internal state management
// =================================================================================================

void RenderDeviceNull::checkError()
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


void RenderDeviceNull::setStorageBuffer( uint8 slot, uint32 bufObj )
{
	H3D_UNUSED_VAR( slot );
	H3D_UNUSED_VAR( bufObj );
}


bool RenderDeviceNull::applyVertexLayout( const RDIGeometryInfoGL2 &geo )
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


void RenderDeviceNull::applySamplerState( RDITextureGL2 &tex )
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


void RenderDeviceNull::applyRenderStates()
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


bool RenderDeviceNull::commitStates( uint32 filter )
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


void RenderDeviceNull::resetStates()
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

void RenderDeviceNull::clear( uint32 flags, float *colorRGBA, float depth )
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


void RenderDeviceNull::draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
{
	if( commitStates() )
	{
		glDrawArrays( RDI_GL2::primitiveTypes[ (uint32)primType ], firstVert, numVerts );
	}

	CHECK_GL_ERROR
}


void RenderDeviceNull::drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
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

}  // namespace RDI_Null
}  // namespace Horde3D

