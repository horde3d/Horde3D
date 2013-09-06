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

#ifndef _egPipeline_H_
#define _egPipeline_H_

#include "egPrerequisites.h"
#include "egMaterial.h"
#include <string>
#include <vector>


namespace Horde3D {

class XMLNode;


// =================================================================================================
// Pipeline Resource
// =================================================================================================

struct PipelineResData
{
	enum List
	{
		StageElem = 900,
		StageNameStr,
		StageActivationI
	};
};

// =================================================================================================

struct PipelineCommands
{
	enum List
	{
		SwitchTarget,
		BindBuffer,
		UnbindBuffers,
		ClearTarget,
		DrawGeometry,
		DrawOverlays,
		DrawQuad,
		DoForwardLightLoop,
		DoDeferredLightLoop,
		SetUniform
	};
};

struct RenderingOrder
{
	enum List
	{
		None = 0,
		FrontToBack,
		BackToFront,
		StateChanges
	};
};


class PipeCmdParam
{
public:
	PipeCmdParam() : _string( 0x0 ) { _basic.ptr = 0x0; }
	PipeCmdParam( const PipeCmdParam &copy )
		: _basic( copy._basic ), _string( 0x0 ), _resource( copy._resource )
		{ if( copy._string ) setString( copy._string->c_str() ); }
	~PipeCmdParam() { delete _string; }
	
	float getFloat() { return _basic.f; }
	int getInt() { return _basic.i; }
	bool getBool() { return _basic.b; }
	void *getPtr() { return _basic.ptr; }
	const char *getString() { return _string ? _string->c_str() : 0x0; }
	Resource *getResource() { return _resource.getPtr(); }

	void setFloat( float f ) { _basic.f = f; }
	void setInt( int i ) { _basic.i = i; }
	void setBool( bool b ) { _basic.b = b; }
	void setPtr( void *ptr ) { _basic.ptr = ptr; }
	void setString( const char *str ) { _string = new std::string( str ); }
	void setResource( Resource *resource ) { _resource = resource; }

protected:
	union BasicType
	{
		float  f;
		int    i;
		bool   b;
		void   *ptr;
	};

	BasicType      _basic;
	std::string    *_string;
	PResource      _resource;
};


struct PipelineCommand
{
	PipelineCommands::List       command;
	std::vector< PipeCmdParam >  params;

	PipelineCommand( PipelineCommands::List	command )
	{
		this->command = command;
	}
};


struct PipelineStage
{
	std::string                     id;
	PMaterialResource               matLink;
	std::vector< PipelineCommand >  commands;
	bool                            enabled;

	PipelineStage() : matLink( 0x0 ) {}
};


struct RenderTarget
{
	std::string           id;
	uint32                numColBufs;
	TextureFormats::List  format;
	uint32                width, height;
	uint32                samples;
	float                 scale;  // Scale factor for FB width and height
	bool                  hasDepthBuf;
	uint32                rendBuf;

	RenderTarget()
	{
		hasDepthBuf = false;
		numColBufs = 0;
		rendBuf = 0;
	}
};

// =================================================================================================

class PipelineResource : public Resource
{
public:
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new PipelineResource( name, flags ); }
	
	PipelineResource( const std::string &name, int flags );
	~PipelineResource();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );
	void resize( uint32 width, uint32 height );

	int getElemCount( int elem );
	int getElemParamI( int elem, int elemIdx, int param );
	void setElemParamI( int elem, int elemIdx, int param, int value );
	const char *getElemParamStr( int elem, int elemIdx, int param );

	bool getRenderTargetData( const std::string &target, int bufIndex, int *width, int *height,
	                          int *compCount, void *dataBuffer, int bufferSize );

private:
	bool raiseError( const std::string &msg, int line = -1 );
	const std::string parseStage( XMLNode &node, PipelineStage &stage );

	void addRenderTarget( const std::string &id, bool depthBuffer, uint32 numBuffers,
	                      TextureFormats::List format, uint32 samples,
	                      uint32 width, uint32 height, float scale );
	RenderTarget *findRenderTarget( const std::string &id );
	bool createRenderTargets();
	void releaseRenderTargets();

private:
	std::vector< RenderTarget >   _renderTargets;
	std::vector< PipelineStage >  _stages;
	uint32                        _baseWidth, _baseHeight;

	friend class ResourceManager;
	friend class Renderer;
};

typedef SmartResPtr< PipelineResource > PPipelineResource;

}
#endif // _egPipeline_H_
