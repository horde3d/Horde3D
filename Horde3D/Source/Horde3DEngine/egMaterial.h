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

#ifndef _egMaterial_H_
#define _egMaterial_H_

#include "egPrerequisites.h"
#include "egResource.h"
#include "egShader.h"
#include "egTexture.h"


namespace Horde3D {

// =================================================================================================
// Material Resource
// =================================================================================================

struct MaterialResData
{
	enum List
	{
		MaterialElem = 400,
		SamplerElem,
		UniformElem,
		MatClassStr,
		MatLinkI,
		MatShaderI,
		SampNameStr,
		SampTexResI,
		UnifNameStr,
		UnifValueF4
	};
};

// =================================================================================================

struct MatSampler
{
	std::string       name;
	PTextureResource  texRes;
};


struct MatUniform
{
	std::string  name;
	float        values[4];	


	MatUniform()
	{
		values[0] = 0; values[1] = 0; values[2] = 0; values[3] = 0;
	}
};

// =================================================================================================

class MaterialResource;
typedef SmartResPtr< MaterialResource > PMaterialResource;

class MaterialResource : public Resource
{
public:
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new MaterialResource( name, flags ); }
	
	MaterialResource( const std::string &name, int flags );
	~MaterialResource();
	Resource *clone();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );
	bool setUniform( const std::string &name, float a, float b, float c, float d );
	bool isOfClass( const std::string &theClass );

	int getElemCount( int elem );
	int getElemParamI( int elem, int elemIdx, int param );
	void setElemParamI( int elem, int elemIdx, int param, int value );
	float getElemParamF( int elem, int elemIdx, int param, int compIdx );
	void setElemParamF( int elem, int elemIdx, int param, int compIdx, float value );
	const char *getElemParamStr( int elem, int elemIdx, int param );
	void setElemParamStr( int elem, int elemIdx, int param, const char *value );

private:
	bool raiseError( const std::string &msg, int line = -1 );

private:
	PShaderResource             _shaderRes;
	uint32                      _combMask;
	std::string                 _class;
	std::vector< MatSampler >   _samplers;
	std::vector< MatUniform >   _uniforms;
	std::vector< std::string >  _shaderFlags;
	PMaterialResource           _matLink;

	friend class ResourceManager;
	friend class Renderer;
	friend class MeshNode;
};

}
#endif // _egMaterial_H_
