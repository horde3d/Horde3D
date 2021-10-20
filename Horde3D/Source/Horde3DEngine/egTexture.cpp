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

#include "utEndian.h"
#include "egTexture.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include "utImage.h"
#include <cstring>

#include "utDebug.h"
#include <array>
#include <tuple>


namespace Horde3D {

using namespace std;

// *************************************************************************************************
// Class TextureResource
// *************************************************************************************************

//
// DDS
//
#define FOURCC( c0, c1, c2, c3 ) ((c0) | (c1<<8) | (c2<<16) | (c3<<24))

#define DDSD_MIPMAPCOUNT      0x00020000

#define DDPF_ALPHAPIXELS      0x00000001
#define DDPF_FOURCC           0x00000004
#define DDPF_RGB              0x00000040

#define DDSCAPS2_CUBEMAP      0x00000200
#define DDSCAPS2_CM_COMPLETE  (0x00000400 | 0x00000800 | 0x00001000 | 0x00002000 | 0x00004000 | 0x00008000)
#define DDSCAPS2_VOLUME       0x00200000

#define D3DFMT_A16B16G16R16F  113
#define D3DFMT_A32B32G32R32F  116

#define D3DFMT_DXGI_BC6H_UF16 96
#define D3DFMT_DXGI_BC6H_SF16 97

#define D3DFMT_DXGI_BC7		  98
#define D3DFMT_DXGI_BC7U	  99

struct DDSHeader
{
	uint32  dwMagic;
	uint32  dwSize;
	uint32  dwFlags;
	uint32  dwHeight, dwWidth;
	uint32  dwPitchOrLinearSize;
	uint32  dwDepth;
	uint32  dwMipMapCount;
	uint32  dwReserved1[11];

	struct {
		uint32  dwSize;
		uint32  dwFlags;
		uint32  dwFourCC;
		uint32  dwRGBBitCount;
		uint32  dwRBitMask, dwGBitMask, dwBBitMask, dwABitMask;
	} pixFormat;

	struct {
		uint32  dwCaps, dwCaps2, dwCaps3, dwCaps4;
	} caps;

	uint32  dwReserved2;
} ddsHeader;

//
// KTX
//
const unsigned char ktxIdentifier[ 12 ] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

struct KTXHeader
{
	unsigned char identifier[ 12 ];
	uint32 endianness;
	uint32 glType;
	uint32 glTypeSize;
	uint32 glFormat;
	uint32 glInternalFormat;
	uint32 glBaseInternalFormat;
	uint32 pixelWidth;
	uint32 pixelHeight;
	uint32 pixelDepth;
	uint32 numberOfArrayElements;
	uint32 numberOfFaces;
	uint32 numberOfMipmapLevels;
	uint32 bytesOfKeyValueData;
} ktxHeader;

struct ktxTexFormat
{
	uint32 h3dTexFormat;
	uint32 glFormat;
	uint32 glSRGBFormat;
};

const std::array< ktxTexFormat, 25 > ktxSupportedFormats = { {
//			 h3dTexFormat, glFormat, glSRGBFormat
	{ TextureFormats::BGRA8, 0x8051, 0x8051 }, // GL_RGB8
	{ TextureFormats::BGRA8, 0x8058, 0x8058 }, // GL_RGBA8
	{ TextureFormats::BGRA8, 0x80E1, 0x80E1 }, // GL_BGRA
	{ TextureFormats::RGBA16F, 0x881A, 0x881A }, // GL_RGBA16F
	{ TextureFormats::RGBA32F, 0x8814, 0x8814 }, // GL_RGBA32F
	{ TextureFormats::DXT1, 0x83F1, 0x83F1 }, // GL_COMPRESSED_RGBA_S3TC_DXT1
	{ TextureFormats::DXT3, 0x83F2, 0x83F2 }, // GL_COMPRESSED_RGBA_S3TC_DXT3
	{ TextureFormats::DXT5, 0x83F3, 0x83F3 }, // GL_COMPRESSED_RGBA_S3TC_DXT5
	{ TextureFormats::ETC1, 0x8D64, 0x8D64 }, // GL_ETC1_RGB8_OES
	{ TextureFormats::RGB8_ETC2, 0x9274, 0x9275 }, // GL_COMPRESSED_RGB8_ETC2, GL_COMPRESSED_SRGB8_ETC2
	{ TextureFormats::RGBA8_ETC2, 0x9278, 0x9279 }, // GL_COMPRESSED_RGBA8_ETC2_EAC, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
	{ TextureFormats::ASTC_4x4, 0x93B0, 0x93D0 }, // GL_COMPRESSED_RGBA_ASTC_4x4_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
	{ TextureFormats::ASTC_5x4, 0x93B1, 0x93D1 }, // GL_COMPRESSED_RGBA_ASTC_5x4_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
	{ TextureFormats::ASTC_5x5, 0x93B2, 0x93D2 }, // GL_COMPRESSED_RGBA_ASTC_5x5_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR
	{ TextureFormats::ASTC_6x5, 0x93B3, 0x93D3 }, // GL_COMPRESSED_RGBA_ASTC_6x5_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
	{ TextureFormats::ASTC_6x6, 0x93B4, 0x93D4 }, // GL_COMPRESSED_RGBA_ASTC_6x6_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR
	{ TextureFormats::ASTC_8x5, 0x93B5, 0x93D5 }, // GL_COMPRESSED_RGBA_ASTC_8x5_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
	{ TextureFormats::ASTC_8x6, 0x93B6, 0x93D6 }, // GL_COMPRESSED_RGBA_ASTC_8x6_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
	{ TextureFormats::ASTC_8x8, 0x93B7, 0x93D7 }, // GL_COMPRESSED_RGBA_ASTC_8x8_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
	{ TextureFormats::ASTC_10x5, 0x93B8, 0x93D8 }, // GL_COMPRESSED_RGBA_ASTC_10x5_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
	{ TextureFormats::ASTC_10x6, 0x93B9, 0x93D9 }, // GL_COMPRESSED_RGBA_ASTC_10x6_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
	{ TextureFormats::ASTC_10x8, 0x93BA, 0x93DA }, // GL_COMPRESSED_RGBA_ASTC_10x8_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
	{ TextureFormats::ASTC_10x10, 0x93BB, 0x93DB }, // GL_COMPRESSED_RGBA_ASTC_10x10_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
	{ TextureFormats::ASTC_12x10, 0x93BC, 0x93DC }, // GL_COMPRESSED_RGBA_ASTC_12x10_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
	{ TextureFormats::ASTC_12x12, 0x93BD, 0x93DD }, // GL_COMPRESSED_RGBA_ASTC_12x12_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR
} };


unsigned char *TextureResource::mappedData = 0x0;
int TextureResource::mappedWriteImage = -1;
uint32 TextureResource::defTex2DObject = 0;
uint32 TextureResource::defTex3DObject = 0;
uint32 TextureResource::defTexCubeObject = 0;
bool TextureResource::bgraSwizzleRequired = true;

void TextureResource::initializationFunc()
{
	unsigned char texData[] = 
		{ 128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
		  128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
		  128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
		  128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255 };

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// Check if RGBA->BGRA swizzle required by render device
	if ( Modules::renderer().getRenderDeviceType() == RenderBackendType::OpenGLES3 )
	{
		bgraSwizzleRequired = false;
	}

	// Upload default textures
	defTex2DObject = rdi->createTexture( TextureTypes::Tex2D, 4, 4, 1,
	                                      TextureFormats::BGRA8, 2, true, false, false );
	rdi->uploadTextureData( defTex2DObject, 0, 0, texData );
	
	defTexCubeObject = rdi->createTexture( TextureTypes::TexCube, 4, 4, 1,
	                                        TextureFormats::BGRA8, 2, true, false, false );
	for( uint32 i = 0; i < 6; ++i ) 
	{
		rdi->uploadTextureData( defTexCubeObject, i, 0, texData );
	}

	unsigned char *texData2 = new unsigned char[256];
	memcpy( texData2, texData, 64 ); memcpy( texData2 + 64, texData, 64 );
	memcpy( texData2 + 128, texData, 64 ); memcpy( texData2 + 192, texData, 64 );

	defTex3DObject = rdi->createTexture( TextureTypes::Tex3D, 4, 4, 4,
	                                      TextureFormats::BGRA8, 2, true, false, false );
	rdi->uploadTextureData( defTex3DObject, 0, 0, texData2 );
	delete[] texData2;
}


void TextureResource::releaseFunc()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	rdi->destroyTexture( defTex2DObject );
	rdi->destroyTexture( defTex3DObject );
	rdi->destroyTexture( defTexCubeObject );
}


TextureResource::TextureResource( const string &name, int flags ) :
	Resource( ResourceTypes::Texture, name, flags )
{
	_texType = TextureTypes::Tex2D;
	initDefault();
}


TextureResource::TextureResource( const string &name, uint32 width, uint32 height, uint32 depth,
                                  TextureFormats::List fmt, int flags ) :
	Resource( ResourceTypes::Texture, name, flags ),
	_width( width ), _height( height ), _depth( depth ), _rbObj( 0 )
{	
	_loaded = true;
	_texFormat = fmt;
	_maxMipLevel = (_flags & ResourceFlags::NoTexMipmaps) ? 0 : getMaxAtMipFullLevel();

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	if( flags & ResourceFlags::TexRenderable )
	{
		_flags &= ~ResourceFlags::TexCubemap;
		_flags &= ~ResourceFlags::TexSRGB;
		_flags |= ResourceFlags::NoTexCompression;
		_texType = TextureTypes::Tex2D;
		_sRGB = false;
		_rbObj = rdi->createRenderBuffer( width, height, fmt, flags & ResourceFlags::TexDepthBuffer ? true : false, 1, 0, _maxMipLevel );
		_texObject = rdi->getRenderBufferTex( _rbObj, 0 );
	}
	else
	{
		uint32 size = rdi->calcTextureSize( _texFormat, width, height, depth );
		unsigned char *pixels = new unsigned char[size];
		memset( pixels, 0, size );

		_texType = flags & ResourceFlags::TexCubemap ? TextureTypes::TexCube : TextureTypes::Tex2D;
		if( depth > 1 ) _texType = TextureTypes::Tex3D;
		_sRGB = (_flags & ResourceFlags::TexSRGB) != 0;
		_texObject = rdi->createTexture( _texType, _width, _height, _depth, _texFormat,
		                                 _maxMipLevel, _maxMipLevel > 0, false, _sRGB );

		int nSlices = _texType != TextureTypes::TexCube ? 6 : 1;
		for ( uint32 mipLevel = 0; mipLevel <= _maxMipLevel; ++mipLevel )
		{
			for (int slice = 0; slice < nSlices; ++slice ) {
				rdi->uploadTextureData( _texObject, slice, (int) mipLevel, pixels );
			}
		}

		delete[] pixels;
		if( _texObject == 0 ) initDefault();
	}
}


TextureResource::~TextureResource()
{
	release();
}


void TextureResource::initDefault()
{
	_rbObj = 0;
	_texFormat = TextureFormats::BGRA8;
	_width = 0; _height = 0; _depth = 0;
	_sRGB = false;
	_maxMipLevel = 0;
	
	if( _texType == TextureTypes::TexCube )
		_texObject = defTexCubeObject;
	else if( _texType == TextureTypes::Tex3D )
		_texObject = defTex3DObject;
	else
		_texObject = defTex2DObject;
}


void TextureResource::release()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	if( _rbObj != 0 )
	{
		// In this case _texObject is just points to the render buffer
		rdi->destroyRenderBuffer( _rbObj );
	}
	else if( _texObject != 0 && _texObject != defTex2DObject && _texObject != defTexCubeObject )
	{
		rdi->destroyTexture( _texObject );
	}

	_texObject = 0;
}


bool TextureResource::raiseError( const string &msg )
{
	// Reset
	release();
	initDefault();

	Modules::log().writeError( "Texture resource '%s': %s", _name.c_str(), msg.c_str() );
	
	return false;
}


bool TextureResource::checkDDS( const char *data, int size ) const
{
    return size > 128 && *((uint32 *)data) == FOURCC( 'D', 'D', 'S', ' ' );
}


bool TextureResource::loadDDS( const char *data, int size )
{
	ASSERT_STATIC( sizeof( DDSHeader ) == 128 );

	// all of the dds header is uint32 data, so we consider it a array of uint32s.
	elemcpy_le((uint32*)(&ddsHeader), (uint32*)(data), 128 / sizeof(uint32));

	// Check header
	// There are some flags that are required to be set for every dds but we don't check them
	if( ddsHeader.dwSize != 124 )
	{
		return raiseError( "Invalid DDS header" );
	}

	// Store properties
	_width = ddsHeader.dwWidth;
	_height = ddsHeader.dwHeight;
	_depth = 1;
	_texFormat = TextureFormats::Unknown;
	_texObject = 0;
	_sRGB = (_flags & ResourceFlags::TexSRGB) != 0;
	int mipCount = ddsHeader.dwFlags & DDSD_MIPMAPCOUNT ? ddsHeader.dwMipMapCount : 1;
	_maxMipLevel = mipCount > 1 ? mipCount - 1 : 0;
	bool dx10HeaderAvailable = false;

	// Get texture type
	if( ddsHeader.caps.dwCaps2 == 0 )
	{
		_texType = TextureTypes::Tex2D;
	}
	else if( ddsHeader.caps.dwCaps2 & DDSCAPS2_CUBEMAP )
	{
		if( (ddsHeader.caps.dwCaps2 & DDSCAPS2_CM_COMPLETE) != DDSCAPS2_CM_COMPLETE )
			raiseError( "DDS cubemap does not contain all cube sides" );
		_texType = TextureTypes::TexCube;
	}
	else if( ddsHeader.caps.dwCaps2 & DDSCAPS2_VOLUME )
	{
		_depth = ddsHeader.dwDepth;
		_texType = TextureTypes::Tex3D;
	}
	else
	{
		return raiseError( "Unsupported DDS texture type" );
	}
	
	// Get pixel format
	int blockSize = 1, bytesPerBlock = 4;
	enum { pfBGRA, pfBGR, pfBGRX, pfRGB, pfRGBX, pfRGBA } pixFmt = pfBGRA;
	
	if( ddsHeader.pixFormat.dwFlags & DDPF_FOURCC )
	{
		switch( ddsHeader.pixFormat.dwFourCC )
		{
		case FOURCC( 'D', 'X', 'T', '1' ):
			_texFormat = TextureFormats::DXT1;
			blockSize = 4; bytesPerBlock = 8;
			break;
		case FOURCC( 'D', 'X', 'T', '3' ):
			_texFormat = TextureFormats::DXT3;
			blockSize = 4; bytesPerBlock = 16;
			break;
		case FOURCC( 'D', 'X', 'T', '5' ):
			_texFormat = TextureFormats::DXT5;
			blockSize = 4; bytesPerBlock = 16;
			break;
		case D3DFMT_A16B16G16R16F: 
			_texFormat = TextureFormats::RGBA16F;
			bytesPerBlock = 8;
			break;
		case D3DFMT_A32B32G32R32F: 
			_texFormat = TextureFormats::RGBA32F;
			bytesPerBlock = 16;
			break;
		case FOURCC( 'D', 'X', '1', '0' ):
			{
				// DX10 header contains another 20 bytes
				uint32 dx10Header[ 5 ];
				elemcpy_le( ( uint32* ) ( &dx10Header ), ( uint32* ) ( data + 128 ), 20 / sizeof( uint32 ) );

				uint32 dx10Format = dx10Header[ 0 ];
				switch ( dx10Format )
				{
					case D3DFMT_DXGI_BC7:
					case D3DFMT_DXGI_BC7U:
						_texFormat = TextureFormats::BC7;
						blockSize = 4; bytesPerBlock = 16;
						break;
					case D3DFMT_DXGI_BC6H_UF16:
						_texFormat = TextureFormats::BC6_UF16;
						blockSize = 4; bytesPerBlock = 16;
						break;
					case D3DFMT_DXGI_BC6H_SF16:
						_texFormat = TextureFormats::BC6_SF16;
						blockSize = 4; bytesPerBlock = 16;
						break;
				}

				dx10HeaderAvailable = true;
				break;
			}
		}
	}
	else if( ddsHeader.pixFormat.dwFlags & DDPF_RGB )
	{
		bytesPerBlock = ddsHeader.pixFormat.dwRGBBitCount / 8;
		
		if( ddsHeader.pixFormat.dwRBitMask == 0x00ff0000 &&
		    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
		    ddsHeader.pixFormat.dwBBitMask == 0x000000ff ) pixFmt = pfBGR;
		else
		if( ddsHeader.pixFormat.dwRBitMask == 0x000000ff &&
		    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
		    ddsHeader.pixFormat.dwBBitMask == 0x00ff0000 ) pixFmt = pfRGB;

		if( pixFmt == pfBGR || pixFmt == pfRGB )
		{
			if( ddsHeader.pixFormat.dwRGBBitCount == 24 )
			{
				_texFormat = TextureFormats::BGRA8;
			}
			else if( ddsHeader.pixFormat.dwRGBBitCount == 32 )
			{
				if( !(ddsHeader.pixFormat.dwFlags & DDPF_ALPHAPIXELS) ||
				    ddsHeader.pixFormat.dwABitMask == 0x00000000 )
				{
					_texFormat = TextureFormats::BGRA8;
					pixFmt = pixFmt == pfBGR ? pfBGRX : pfRGBX;
				}
				else
				{	
					_texFormat = TextureFormats::BGRA8;
					pixFmt = pixFmt == pfBGR ? pfBGRA : pfRGBA;
				}
			}
		}
	}

	if( _texFormat == TextureFormats::Unknown )
		return raiseError( "Unsupported DDS pixel format" );

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// Create texture
	_texObject = rdi->createTexture( _texType, _width, _height, _depth, _texFormat,
	                                 _maxMipLevel, false, false, _sRGB );
	
	if ( _texObject == 0 ) return raiseError( "Failed to create DDS texture" );

	// Upload texture subresources
	int numSlices = _texType == TextureTypes::TexCube ? 6 : 1;
	unsigned char *pixels =  dx10HeaderAvailable ? ( unsigned char * ) ( data + 128 + 20 ) : ( unsigned char * )( data + 128 );

	for( int i = 0; i < numSlices; ++i )
	{
		int width = _width, height = _height, depth = _depth;
		uint32 *dstBuf = 0x0;

		for( int j = 0; j < mipCount; ++j )
		{
			size_t mipSize = idivceil(width, blockSize) * idivceil(height, blockSize) *
			                 depth * bytesPerBlock;
			
			if( pixels + mipSize > (unsigned char *)data + size )
				return raiseError( "Corrupt DDS" );

			if( _texFormat == TextureFormats::BGRA8 && pixFmt != pfBGRA )
			{
				// Convert 8 bit DDS formats to BGRA
				uint32 pixCount = width * height * depth;
				if( dstBuf == 0x0 ) dstBuf = new uint32[pixCount * 4];
				uint32 *p = dstBuf;

				if( pixFmt == pfBGR )
					for( uint32 k = 0; k < pixCount * 3; k += 3 )
						*p++ = pixels[k+0] | pixels[k+1]<<8 | pixels[k+2]<<16 | 0xFF000000;
				else if( pixFmt == pfBGRX )
					for( uint32 k = 0; k < pixCount * 4; k += 4 )
						*p++ = pixels[k+0] | pixels[k+1]<<8 | pixels[k+2]<<16 | 0xFF000000;
				else if( pixFmt == pfRGB )
					for( uint32 k = 0; k < pixCount * 3; k += 3 )
						*p++ = pixels[k+2] | pixels[k+1]<<8 | pixels[k+0]<<16 | 0xFF000000;
				else if( pixFmt == pfRGBX )
					for( uint32 k = 0; k < pixCount * 4; k += 4 )
						*p++ = pixels[k+2] | pixels[k+1]<<8 | pixels[k+0]<<16 | 0xFF000000;
				else if( pixFmt == pfRGBA )
					for( uint32 k = 0; k < pixCount * 4; k += 4 )
						*p++ = pixels[k+2] | pixels[k+1]<<8 | pixels[k+0]<<16 | pixels[k+3]<<24;
				
				rdi->uploadTextureData( _texObject, i, j, dstBuf );
			}
			else
			{
				// Upload DDS data directly
				rdi->uploadTextureData( _texObject, i, j, pixels );
			}

			pixels += mipSize;
			if( width > 1 ) width >>= 1;
			if( height > 1 ) height >>= 1;
			if( depth > 1 ) depth >>= 1;
		}

		if( dstBuf != 0x0 ) delete[] dstBuf;
	}

	ASSERT( pixels == (unsigned char *)data + size );

	return true;
}


bool TextureResource::checkKTX( const char *data, int size ) const
{
	return size > 64 && memcmp( data, ktxIdentifier, 12 ) == 0;
}


bool TextureResource::loadKTX( const char *data, int size )
{
	ASSERT_STATIC( sizeof( KTXHeader ) == 64 );

	// all of the ktx header is uint32 data, so we consider it a array of uint32s.
	elemcpy_le( ( uint32* ) ( &ktxHeader ), ( uint32* ) ( data ), 64 / sizeof( uint32 ) );

	// Check header
	if ( memcmp( ktxHeader.identifier, ktxIdentifier, 12 ) != 0 )
	{
		Modules::log().writeError( "Invalid KTX header" );
		return false;
	}

	// Store properties
	_width = ktxHeader.pixelWidth;
	_height = ktxHeader.pixelHeight;
	_depth = 1;
	_texFormat = TextureFormats::Unknown;
	_texObject = 0;
	_sRGB = ( _flags & ResourceFlags::TexSRGB ) != 0;
	uint32 mipCount = ktxHeader.numberOfMipmapLevels;
	_maxMipLevel = mipCount > 1 ? mipCount - 1 : 0;

	// Get texture type
	if ( ktxHeader.numberOfFaces > 1 )
	{
		if ( ktxHeader.numberOfFaces != 6 )
		{
			Modules::log().writeError( "Wrong number of cube texture faces (should be 6)" );
			return false;
		}
		else
		{
			_texType = TextureTypes::TexCube;
		}
	}
	else if ( ktxHeader.pixelDepth > 1 )
	{
		_depth = ktxHeader.pixelDepth;
		_texType = TextureTypes::Tex3D;
	}
	else
		_texType = TextureTypes::Tex2D;

	// Texture arrays are not supported yet
	if ( ktxHeader.numberOfArrayElements > 1 )
		Modules::log().writeWarning( "Texture Arrays not supported. using first array element only" );
	else ktxHeader.numberOfArrayElements = 1; // ktx spec note 2 - Replace with 1 if this field is 0.

	// Get pixel format
	for ( const ktxTexFormat &tex : ktxSupportedFormats )
	{
		if ( ktxHeader.glInternalFormat == tex.glFormat || ktxHeader.glInternalFormat == tex.glSRGBFormat )
		{
			_texFormat = (TextureFormats::List) tex.h3dTexFormat;
			break;
		}
	}

	if ( _texFormat == TextureFormats::Unknown )
		return raiseError( "Unsupported KTX pixel format" );
	
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// Create texture
	_texObject = rdi->createTexture( _texType, _width, _height, _depth, _texFormat,
		_maxMipLevel, false, false, _sRGB );

	if ( _texObject == 0 ) return raiseError( "Failed to create KTX texture" );

	//uint32 sliceCount = _texType == TextureTypes::TexCube ? 6 : 1;
	unsigned char *pixels = ( unsigned char * ) ( data + sizeof( KTXHeader ) + ktxHeader.bytesOfKeyValueData );

	int width = _width, height = _height, depth = _depth;
	uint32 *dstBuf = 0x0;

	for ( uint32 mip = 0; mip < mipCount; ++mip )
	{
		uint32 mipSize;
		pixels = (unsigned char *) elemcpy_le( &mipSize, ( uint32* ) ( pixels ), 1 );

		if ( pixels + mipSize > ( unsigned char * )data + size )
			return raiseError( "Corrupt KTX" );

		for ( uint32 element = 0; element < ktxHeader.numberOfArrayElements; ++element ) 
		{
			for ( uint32 slice = 0; slice < ktxHeader.numberOfFaces; ++slice )
			{
				if ( element == 0 )
				{	// using only first element of array now
					if ( _texFormat == TextureFormats::BGRA8 && ktxHeader.glInternalFormat != 0x80E1 ) // GL_BGRA
					{
						// Convert 8 bit KTX formats to BGRA
						uint32 pixCount = width * height * depth;
						if ( dstBuf == 0x0 ) dstBuf = new uint32[ pixCount * 4 ];
						uint32 *p = dstBuf;

						if ( ktxHeader.glInternalFormat == 0x8051 ) // GL_RGB8
							for ( uint32 k = 0; k < pixCount * 3; k += 3 )
								*p++ = pixels[ k + 2 ] | pixels[ k + 1 ] << 8 | pixels[ k + 0 ] << 16 | 0xFF000000;
						else if ( ktxHeader.glInternalFormat == 0x8058 && bgraSwizzleRequired ) // GL_RGBA8
							for ( uint32 k = 0; k < pixCount * 4; k += 4 )
								*p++ = pixels[ k + 2 ] | pixels[ k + 1 ] << 8 | pixels[ k + 0 ] << 16 | pixels[ k + 3 ] << 24;

						rdi->uploadTextureData( _texObject, slice, mip, dstBuf );
					}
					else
					{
						// Upload KTX data directly
						rdi->uploadTextureData( _texObject, slice, mip, pixels );
					}
				}

				pixels += mipSize;	
			}
		}

		if ( width > 1 ) width >>= 1;
		if ( height > 1 ) height >>= 1;
		if ( depth > 1 ) depth >>= 1;
	}

	if ( dstBuf != 0x0 ) delete[] dstBuf;

	ASSERT( pixels == ( unsigned char * ) data + size );
	return true;
}


bool TextureResource::loadSTBI( const char *data, int size )
{
	bool hdr = false;
	if( stbi_is_hdr_from_memory( (unsigned char *)data, size ) > 0 ) hdr = true;
	
	int comps;
	void *pixels = 0x0;
	if( hdr )
		pixels = stbi_loadf_from_memory( (unsigned char *)data, size, &_width, &_height, &comps, 4 );
	else
		pixels = stbi_load_from_memory( (unsigned char *)data, size, &_width, &_height, &comps, 4 );

	if( pixels == 0x0 )
		return raiseError( "Invalid image format (" + string( stbi_failure_reason() ) + ")" );

	// Swizzle RGBA -> BGRA if required
	if ( bgraSwizzleRequired )
	{
		uint32 *ptr = ( uint32 * ) pixels;
		for ( uint32 i = 0, si = _width * _height; i < si; ++i )
		{
			uint32 col = *ptr;
			*ptr++ = ( col & 0xFF00FF00 ) | ( ( col & 0x000000FF ) << 16 ) | ( ( col & 0x00FF0000 ) >> 16 );
		}
	}
	
	_depth = 1;
	_texType = TextureTypes::Tex2D;
	_texFormat = hdr ? TextureFormats::RGBA16F : TextureFormats::BGRA8;
	_sRGB = (_flags & ResourceFlags::TexSRGB) != 0;
	_maxMipLevel = (_flags & ResourceFlags::NoTexMipmaps) ? 0 : getMaxAtMipFullLevel();

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// Create and upload texture
	_texObject = rdi->createTexture( _texType, _width, _height, _depth, _texFormat,
		_maxMipLevel, _maxMipLevel > 1, !(_flags & ResourceFlags::NoTexCompression), _sRGB );
	rdi->uploadTextureData( _texObject, 0, 0, pixels );

	stbi_image_free( pixels );

	return true;
}


bool TextureResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;

	if ( checkDDS( data, size ) )
		return loadDDS( data, size );
	else if ( checkKTX( data, size ) )
		return loadKTX( data, size );
	else
		return loadSTBI( data, size );
}


uint32_t TextureResource::getMaxAtMipFullLevel() const
{
	return ftoi_t( std::log2( std::max( _width, _height ) ) );
}


int TextureResource::getElemCount( int elem ) const
{
	switch( elem )
	{
	case TextureResData::TextureElem:
		return 1;
	case TextureResData::ImageElem:
		return _texType == TextureTypes::TexCube ? 6 * (_maxMipLevel + 1) : _maxMipLevel + 1;
	default:
		return Resource::getElemCount( elem );
	}
}


int TextureResource::getElemParamI( int elem, int elemIdx, int param ) const
{
	switch( elem )
	{
	case TextureResData::TextureElem:
		switch( param )
		{
		case TextureResData::TexFormatI:
			return _texFormat;
		case TextureResData::TexSliceCountI:
			return _texType == TextureTypes::TexCube ? 6 : 1;
		}
		break;
	case TextureResData::ImageElem:
		switch( param )
		{
		case TextureResData::ImgWidthI:
			if( elemIdx < getElemCount( elem ) )
			{
				int mipLevel = elemIdx % (_maxMipLevel + 1);
				return std::max( 1, _width >> mipLevel );
			}
			break;
		case TextureResData::ImgHeightI:
			if( elemIdx < getElemCount( elem ) )
			{
				int mipLevel = elemIdx % (_maxMipLevel + 1);
				return std::max( 1, _height >> mipLevel );
			}
			break;
		}
		break;
	}
	
	return Resource::getElemParamI( elem, elemIdx, param );
}


void *TextureResource::mapStream( int elem, int elemIdx, int stream, bool read, bool write )
{
	if( (read || write) && mappedData == 0x0 )
	{
		if( elem == TextureResData::ImageElem && stream == TextureResData::ImgPixelStream &&
		    elemIdx < getElemCount( elem ) )
		{
			RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

			mappedData = Modules::renderer().useScratchBuf(
				rdi->calcTextureSize( _texFormat, _width, _height, _depth ), 16 ); // 16 byte aligned
			
			if( read )
			{	
				int slice = elemIdx / (_maxMipLevel + 1);
				int mipLevel = elemIdx % (_maxMipLevel + 1);
				rdi->getTextureData( _texObject, slice, mipLevel, mappedData );
			}

			if( write )
				mappedWriteImage = elemIdx;
			else
				mappedWriteImage = -1;

			return mappedData;
		}
	}

	return Resource::mapStream( elem, elemIdx, stream, read, write );
}


void TextureResource::unmapStream()
{
	if( mappedData != 0x0 )
	{
		if( mappedWriteImage >= 0 )
		{
			int slice = mappedWriteImage / (_maxMipLevel + 1);
			int mipLevel = mappedWriteImage % (_maxMipLevel + 1);
			Modules::renderer().getRenderDevice()->updateTextureData( _texObject, slice, mipLevel, mappedData );
			mappedWriteImage = -1;
		}
		
		mappedData = 0x0;
		return;
	}

	Resource::unmapStream();
}

}  // namespace
