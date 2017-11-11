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

struct DefaultPipelineCommands
{
	enum List
	{
		SwitchTarget,
		BindBuffer,
		UnbindBuffers,
		ClearTarget,
		DrawGeometry,
//		DrawOverlays,
		DrawQuad,
		DoForwardLightLoop,
		DoDeferredLightLoop,
		SetUniform,
		ExternalCommand = 256 // must be the last command
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
	PipeCmdParam() { _basic.ptr = 0x0; }
	PipeCmdParam( const PipeCmdParam &copy )
		: _basic( copy._basic ), _resource( copy._resource )
		{ if( !copy._string.empty() ) setString( copy._string.c_str() ); }
	~PipeCmdParam() {}
	
	float getFloat() const { return _basic.f; }
	int getInt() const { return _basic.i; }
	bool getBool() const { return _basic.b; }
	void *getPtr() const { return _basic.ptr; }
	const std::string &getString() const { return _string; }
	Resource *getResource() const { return _resource.getPtr(); }

	void setFloat( float f ) { _basic.f = f; }
	void setInt( int i ) { _basic.i = i; }
	void setBool( bool b ) { _basic.b = b; }
	void setPtr( void *ptr ) { _basic.ptr = ptr; }
	void setString( const char *str ) { _string = std::string( str ); } 
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
	std::string    _string; 
	PResource      _resource;
};


struct PipelineCommand
{
	std::vector< PipeCmdParam >			params;
	DefaultPipelineCommands::List       command;
	int									externalCommandID;

	PipelineCommand( DefaultPipelineCommands::List command )
	{
		this->command = command;
		externalCommandID = -1;
	}
};


struct PipelineStage
{
	std::string                     id;
	PMaterialResource               matLink;
	std::vector< PipelineCommand >  commands;
	bool                            enabled;

	PipelineStage() : matLink( 0x0 ), enabled( false ) {}
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
		width = height = 0;
		samples = 0;
		scale = 0;
		format = TextureFormats::Unknown;
	}
};

// =================================================================================================

typedef const char *( *parsePipelineCommandFunc )( const char *commandName, void *xmlNodeParams, PipelineCommand &cmd );
typedef void( *executePipelineCommandFunc )( const PipelineCommand *commandParams );

struct PipelineCommandRegEntry
{
	std::string					comNameString;
	parsePipelineCommandFunc	parseFunc;    // Called when pipeline command is parsed
	executePipelineCommandFunc	executeFunc;  // Called when pipeline command is executed during rendering
};

class ExternalPipelineCommandsManager
{
public:

	static void registerPipelineCommand( const std::string &commandName,  
										 parsePipelineCommandFunc pf, executePipelineCommandFunc ef )
	{
		ASSERT( !commandName.empty() )
		ASSERT( pf != 0x0 )
		ASSERT( ef != 0x0 )

		PipelineCommandRegEntry entry;
		entry.comNameString = commandName;
		entry.parseFunc = pf;
		entry.executeFunc = ef;
		_registeredCommands.emplace_back( entry );
	}

	static uint32 registeredCommandsCount() { return ( uint32 ) _registeredCommands.size(); }

	static const char *parseCommand( const char *commandName, void *xmlData, PipelineCommand &cmd, bool &success )
	{
		for ( size_t i = 0; i < _registeredCommands.size(); ++i )
		{
			PipelineCommandRegEntry &entry = _registeredCommands[ i ];
			if ( entry.comNameString.compare( commandName ) == 0 && entry.parseFunc != 0x0 )
			{
				const char *msg = entry.parseFunc( commandName, xmlData, cmd );
				if ( strlen( msg ) == 0 )
				{
					cmd.externalCommandID = ( int ) i;
				}
				else
				{
					success = false;
				}
				
				return msg;
			}
		}

		return ""; // pipeline command skipped
	}

	static void executeCommand( const PipelineCommand &command )
	{
		if ( command.externalCommandID != -1 )
		{
			_registeredCommands[ command.externalCommandID ].executeFunc( &command );
		}
	}

private:

	static std::vector< PipelineCommandRegEntry >  _registeredCommands;
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

	int getElemCount( int elem ) const;
	int getElemParamI( int elem, int elemIdx, int param ) const;
	void setElemParamI( int elem, int elemIdx, int param, int value );
	const char *getElemParamStr( int elem, int elemIdx, int param ) const;

	bool getRenderTargetData( const std::string &target, int bufIndex, int *width, int *height,
	                          int *compCount, void *dataBuffer, int bufferSize ) const;

private:
	bool raiseError( const std::string &msg, int line = -1 );
	const std::string parseStage( XMLNode &node, PipelineStage &stage );

	void addRenderTarget( const std::string &id, bool depthBuffer, uint32 numBuffers,
	                      TextureFormats::List format, uint32 samples,
	                      uint32 width, uint32 height, float scale );
	RenderTarget *findRenderTarget( const std::string &id ) const;
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
