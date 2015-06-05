// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _egTexture_H_
#define _egTexture_H_

#include "egPrerequisites.h"
#include "egResource.h"
#include "egRendererBase.h"


namespace Horde3D {

struct RenderBuffer;


// =================================================================================================
// Texture Resource
// =================================================================================================

struct TextureResData
{
	enum List
	{
		TextureElem = 700,
		ImageElem,
		TexFormatI,
		TexSliceCountI,
		ImgWidthI,
		ImgHeightI,
		ImgPixelStream,
		TexNativeRefI
	};
};

// =================================================================================================

class TextureResource : public Resource
{
public:
	static void initializationFunc();
	static void releaseFunc();
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new TextureResource( name, flags ); }
	
	TextureResource( const std::string &name, int flags );
	TextureResource( const std::string &name, uint32 width, uint32 height, uint32 depth,
                    TextureFormats::List fmt, int flags );
	~TextureResource();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );

	int getElemCount( int elem );
	int getElemParamI( int elem, int elemIdx, int param );
	void *mapStream( int elem, int elemIdx, int stream, bool read, bool write );
	void unmapStream();

	TextureTypes::List getTexType() { return _texType; }
	TextureFormats::List getTexFormat() { return _texFormat; }
	uint32 getWidth() const { return _width; }
	uint32 getHeight() const { return _height; }
	uint32 getDepth() const { return _depth; }
	uint32 getTexObject() { return _texObject; }
	uint32 getRBObject()  { return _rbObj; }
	bool hasMipMaps() { return _hasMipMaps; }

public:
	static uint32 defTex2DObject;
	static uint32 defTex3DObject;
	static uint32 defTexCubeObject;

protected:
	bool raiseError( const std::string &msg );
	bool checkDDS( const char *data, int size );
	bool loadDDS( const char *data, int size );
	bool loadSTBI( const char *data, int size );
	int getMipCount();
	
protected:
	static unsigned char  *mappedData;
	static int            mappedWriteImage;
	
	TextureTypes::List    _texType;
	TextureFormats::List  _texFormat;
	int                   _width, _height, _depth;
	uint32                _texObject;
	uint32                _rbObj;  // Used when texture is renderable
	bool                  _sRGB;
	bool                  _hasMipMaps;
	uint32                _texNativeRef; // Raw OpenGL texture id

	friend class ResourceManager;
};

typedef SmartResPtr< TextureResource > PTextureResource;

}
#endif // _egTexture_H_
