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

#include "egShader.h"
#include "egShaderParser.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include <fstream>
#include <cstring>

#include "utDebug.h"
#include <utEndian.h>


namespace Horde3D {

using namespace std;

// =================================================================================================
// Static variables definition
// =================================================================================================
uint8 *ShaderResource::_mappedData = nullptr;

// =================================================================================================
// Code Resource
// =================================================================================================

CodeResource::CodeResource( const string &name, int flags ) :
	Resource( ResourceTypes::Code, name, flags )
{
	initDefault();
}


CodeResource::~CodeResource()
{
	release();
}


Resource *CodeResource::clone()
{
	CodeResource *res = new CodeResource( "", _flags );

	*res = *this;
	
	return res;
}


void CodeResource::initDefault()
{
	_flagMask = 0;
	_code.clear();
}


void CodeResource::release()
{
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		_includes[i].first = 0x0;
	}
	_includes.clear();
}


bool CodeResource::raiseError( const std::string &msg )
{
	// Reset
	release();
	initDefault();
	
	Modules::log().writeError( "Code resource '%s': %s", _name.c_str(), msg.c_str() );

	return false;
}


bool CodeResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;

	char *code = new char[size+1];
	char *pCode = code;
	const char *pData = data;
	const char *eof = data + size;
	
	bool lineComment = false, blockComment = false;
	
	// Parse code
	while( pData < eof )
	{
		// Check for begin of comment
		if( pData < eof - 1 && !lineComment && !blockComment )
		{
			if( *pData == '/' && *(pData+1) == '/' )
				lineComment = true;
			else if( *pData == '/' &&  *(pData+1) == '*' )
				blockComment = true;
		}

		// Check for end of comment
		if( lineComment && (*pData == '\n' || *pData == '\r') )
			lineComment = false;
		else if( blockComment && pData < eof - 1 && *pData == '*' && *(pData+1) == '/' )
			blockComment = false;

		// Check for includes
		if( !lineComment && !blockComment && pData < eof - 7 )
		{
			if( *pData == '#' && *(pData+1) == 'i' && *(pData+2) == 'n' && *(pData+3) == 'c' &&
			    *(pData+4) == 'l' && *(pData+5) == 'u' && *(pData+6) == 'd' && *(pData+7) == 'e' )
			{
				pData += 6;
				
				// Parse resource name
				const char *nameBegin = 0x0, *nameEnd = 0x0;
				
				while( ++pData < eof )
				{
					if( *pData == '"' )
					{
						if( nameBegin == 0x0 )
							nameBegin = pData+1;
						else
							nameEnd = pData;
					}
					else if( *pData == '\n' || *pData == '\r' ) break;
				}

				if( nameBegin != 0x0 && nameEnd != 0x0 )
				{
					std::string resName( nameBegin, nameEnd );
					
					ResHandle res =  Modules::resMan().addResource(
						ResourceTypes::Code, resName, 0, false );
					CodeResource *codeRes = (CodeResource *)Modules::resMan().resolveResHandle( res );
					_includes.push_back( std::pair< PCodeResource, size_t >( codeRes, pCode - code ) );
				}
				else
				{
					delete[] code;
					return raiseError( "Invalid #include syntax" );
				}
			}
		}

		// Check for flags
		if( !lineComment && !blockComment && pData < eof - 4 )
		{
			if( *pData == '_' && *(pData+1) == 'F' && *(pData+4) == '_' &&
			    *(pData+2) >= 48 && *(pData+2) <= 57 && *(pData+3) >= 48 && *(pData+3) <= 57 )
			{
				// Set flag
				uint32 num = (*(pData+2) - 48) * 10 + (*(pData+3) - 48);
				_flagMask |= 1 << (num - 1);
				
				for( uint32 i = 0; i < 5; ++i ) *pCode++ = *pData++;
				
				// Ignore rest of name
				while( pData < eof && *pData != ' ' && *pData != '\t' && *pData != '\n' && *pData != '\r' )
					++pData;
			}
		}

		*pCode++ = *pData++;
	}

	*pCode = '\0';
	_code = code;
	delete[] code;

	// Compile shaders that require this code block
	updateShaders();

	return true;
}


bool CodeResource::hasDependency( CodeResource *codeRes ) const
{
	// Note: There is no check for cycles
	
	if( codeRes == this ) return true;
	
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		if( _includes[i].first->hasDependency( codeRes ) ) return true;
	}
	
	return false;
}


bool CodeResource::tryLinking( uint32 *flagMask )
{
	if( !_loaded ) return false;
	if( flagMask != 0x0 ) *flagMask |= _flagMask;
	
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		if( !_includes[i].first->tryLinking( flagMask ) ) return false;
	}

	return true;
}


std::string CodeResource::assembleCode() const
{
	if( !_loaded ) return "";

	std::string finalCode = _code;
	uint32 offset = 0;
	
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		std::string depCode = _includes[i].first->assembleCode();
		finalCode.insert( _includes[i].second + offset, depCode );
		offset += (uint32)depCode.length();
	}

	return finalCode;
}


void CodeResource::updateShaders()
{
	auto resources = Modules::resMan().getResources();
	for( uint32 i = 0; i < resources.size(); ++i )
	{
		Resource *res = resources[ i ];

		if( res != 0x0 && res->getType() == ResourceTypes::Shader )
		{
			ShaderResource *shaderRes = (ShaderResource *)res;
			
			// Mark shaders using this code as uncompiled
			for( uint32 j = 0; j < shaderRes->getContexts().size(); ++j )
			{
				ShaderContext &context = shaderRes->getContexts()[j];
				
				if ( ( context.vertCodeIdx >= 0 && shaderRes->getCode( context.vertCodeIdx )->hasDependency( this ) ) ||
					 ( context.fragCodeIdx >= 0 && shaderRes->getCode( context.fragCodeIdx )->hasDependency( this ) ) ||
					 ( context.geomCodeIdx >= 0 && shaderRes->getCode( context.geomCodeIdx )->hasDependency( this ) ) ||
					 ( context.computeCodeIdx >= 0 && shaderRes->getCode( context.computeCodeIdx )->hasDependency( this ) ) ||
					 ( context.tessCtlCodeIdx >= 0 && shaderRes->getCode( context.tessCtlCodeIdx )->hasDependency( this ) ) ||
					 ( context.tessEvalCodeIdx >= 0 && shaderRes->getCode( context.tessEvalCodeIdx )->hasDependency( this ) ) )
				{
					context.compiled = false;
				}
			}
			
			// Recompile shaders
			shaderRes->compileContexts();
		}
	}
}


// =================================================================================================
// Shader Resource
// =================================================================================================

class Tokenizer
{
public:
	
	Tokenizer( const char *data ) : _p( data ), _line( 1 ) { getNextToken(); }

	int getLine() const { return _line; }

	bool hasToken() const { return _token[0] != '\0'; }
	
	bool checkToken( const char *token, bool peekOnly = false )
	{
		if( _stricmp( _token, token ) == 0 )
		{
			if( !peekOnly ) getNextToken();
			return true;
		}
		return false;
	}

	const char *getToken( const char *charset )
	{
		if( charset )
		{
			// Validate token
			const char *p = _token;
			while( *p )
			{
				if( strchr( charset, *p++ ) == 0x0 )
				{
					_prevToken[0] = '\0';
					return _prevToken;
				}
			}
		}
		
		memcpy( _prevToken, _token, tokenSize );
		getNextToken();
		return _prevToken;
	}

	bool seekToken( const char *token )
	{
		while( _stricmp( getToken( 0x0 ), token ) != 0 )
		{
			if( !hasToken() ) return false;
		}
		return true;
	}

protected:	
	
	void checkLineChange()
	{
		if( *_p == '\r' && *(_p+1) == '\n' )
		{
			++_p;
			++_line;
		}
		else if( *_p == '\r' || *_p == '\n' ) ++_line;
	}

	void skip( const char *chars )
	{
		while( *_p )
		{
			if( !strchr( chars, *_p ) ) break;
			checkLineChange();
			++_p;
		}
	}
	
	bool seekChar( const char *chars )
	{
		while( *_p )
		{
			if( strchr( chars, *_p ) ) break;
			checkLineChange();
			++_p;
		}
		return *_p != '\0';
	}
	
	void getNextToken()
	{
		// Skip whitespace
		skip( " \t\n\r" );

		// Parse token
		const char *p0 = _p, *p1 = _p;
		if( *_p == '"' )  // Handle string
		{
			++_p; ++p0;
			if( seekChar( "\"\n\r" ) ) p1 = _p++;
		}
		else
		{
			seekChar( " \t\n\r{}()<>=,;" );  // Advance until whitespace or special char found
			if( _p == p0 && *_p != '\0' ) ++_p;  // Handle special char
			p1 = _p;
		}
		memcpy( _token, p0, std::min( (ptrdiff_t)(p1 - p0), tokenSize-1 ) );
		_token[std::min( (ptrdiff_t)(p1 - p0), tokenSize-1 )] = '\0';
	}

protected:

	static const ptrdiff_t tokenSize = 128;
	
	char        _token[tokenSize], _prevToken[tokenSize];
	const char  *_p;
	int         _line;
};

// =================================================================================================

string ShaderResource::_vertPreamble = "";
string ShaderResource::_fragPreamble = "";
string ShaderResource::_geomPreamble = "";
string ShaderResource::_tessCtlPreamble = "";
string ShaderResource::_tessEvalPreamble = "";
string ShaderResource::_computePreamble = "";
bool ShaderResource::_defaultPreambleSet = false;

string ShaderResource::_tmpCodeVS = "";
string ShaderResource::_tmpCodeFS = "";
string ShaderResource::_tmpCodeGS = "";
string ShaderResource::_tmpCodeCS = "";
string ShaderResource::_tmpCodeTSCtl = "";
string ShaderResource::_tmpCodeTSEval = "";

// Parsing constants
static const char *identifier = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
static const char *intnum = "+-0123456789";
static const char *floatnum = "+-0123456789.eE";


ShaderResource::ShaderResource( const string &name, int flags ) :
	Resource( ResourceTypes::Shader, name, flags )
{
	initDefault();
}


ShaderResource::~ShaderResource()
{
	release();
}


void ShaderResource::initializationFunc()
{
	// specify default version preamble for shaders
	switch ( Modules::renderer().getRenderDeviceType() )
	{
		case RenderBackendType::OpenGL4:
		{
			_vertPreamble = "#version 330\n";
			_fragPreamble = "#version 330\n";
			_geomPreamble = "#version 330\n";
			_tessCtlPreamble = "#version 410\r\n";
			_tessEvalPreamble = "#version 410\r\n";
			_computePreamble = "#version 430\n";
			
			break;
		}
		case RenderBackendType::OpenGLES3:
		{
			_vertPreamble = "#version 300 es\n precision highp float;\n";
			_fragPreamble = "#version 300 es\n precision highp float;\n precision highp sampler2D;\n precision highp sampler2DShadow;\n";
			_geomPreamble = "#version 320 es\n precision highp float;\n";
			_tessCtlPreamble = "#version 320 es\n precision highp float;\n";
			_tessEvalPreamble = "#version 320 es\n precision highp float;\n";
			_computePreamble = "#version 310 es\n";

			break;
		}
		default:
			break;
	}

	//	_defaultPreambleSet = true;
}


void ShaderResource::initDefault()
{
	
}


void ShaderResource::release()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	for( uint32 i = 0; i < _contexts.size(); ++i )
	{
		for( uint32 j = 0; j < _contexts[i].shaderCombs.size(); ++j )
		{
			rdi->destroyShader( _contexts[i].shaderCombs[j].shaderObj );
		}
	}

	_contexts.clear();
	_samplers.clear();
	_uniforms.clear();
	//_preLoadList.clear();
	_codeSections.clear();
}


bool ShaderResource::raiseError( const string &msg, int line )
{
	// Reset
	release();
	initDefault();

	if( line < 0 )
		Modules::log().writeError( "Shader resource '%s': %s", _name.c_str(), msg.c_str() );
	else
		Modules::log().writeError( "Shader resource '%s': %s (line %i)", _name.c_str(), msg.c_str(), line );
	
	return false;
}


bool ShaderResource::parseFXSection( char *data )
{
	// Preprocessing: Replace comments with whitespace
	char *p = data;
	while( *p )
	{
		if( *p == '/' && *(p+1) == '/' )
		{
			while( *p && *p != '\n' && *p != '\r' )
				*p++ = ' ';
			if( *p == '\0' ) break;
		}
		else if( *p == '/' && *(p+1) == '*' )
		{
			*p++ = ' '; *p++ = ' ';
			while( *p && (*p != '*' || *(p+1) != '/') )
				*p++ = ' ';
			if( *p == '\0' ) return raiseError( "FX: Expected */" );
			*p++ = ' '; *p++ = ' ';
		}
		++p;
	}

	std::vector< uint8 > unitFree( Modules::renderer().getRenderDevice()->getCaps().maxTexUnitCount - 4, true ); // I don't understand why marciano excluded 4 texunits, but currently I'll leave it this way  
//	bool unitFree[12] = {true, true, true, true, true, true, true, true, true, true, true, true}; 
	Tokenizer tok( data );

	while( tok.hasToken() )
	{
		if( tok.checkToken( "float" ) )
		{
			ShaderUniform uniform;
			uniform.size = 1;
			uniform.id = tok.getToken( identifier );
			if( uniform.id == "" ) return raiseError( "FX: Invalid identifier", tok.getLine() );
			uniform.defValues[0] = uniform.defValues[1] = uniform.defValues[2] = uniform.defValues[3] = 0.0f;
			
			// Skip annotations
			if( tok.checkToken( "<" ) )
				if( !tok.seekToken( ">" ) ) return raiseError( "FX: expected '>'", tok.getLine() );
			
			if( tok.checkToken( "=" ) )
				uniform.defValues[0] = toFloat( tok.getToken( floatnum ) );
			if( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );

			_uniforms.push_back( uniform );
		}
		else if( tok.checkToken( "float4" ) )
		{
			ShaderUniform uniform;
			uniform.size = 4;
			uniform.id = tok.getToken( identifier );
			if( uniform.id == "" ) return raiseError( "FX: Invalid identifier", tok.getLine() );
			uniform.defValues[0] = uniform.defValues[1] = uniform.defValues[2] = uniform.defValues[3] = 0.0f;
			
			// Skip annotations
			if( tok.checkToken( "<" ) )
				if( !tok.seekToken( ">" ) ) return raiseError( "FX: expected '>'", tok.getLine() );
			
			if( tok.checkToken( "=" ) )
			{
				if( !tok.checkToken( "{" ) ) return raiseError( "FX: expected '{'", tok.getLine() );
				uniform.defValues[0] = toFloat( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[1] = toFloat( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[2] = toFloat( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[3] = toFloat( tok.getToken( floatnum ) );
				if( !tok.checkToken( "}" ) ) return raiseError( "FX: expected '}'", tok.getLine() );
			}
			if( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );

			_uniforms.push_back( uniform );
		}
		else if ( tok.checkToken( "buffer" ) )
		{
			ShaderBuffer buffer;
			buffer.id = tok.getToken( identifier );
			if ( buffer.id == "" ) return raiseError( "FX: Invalid identifier", tok.getLine() );

			// Skip annotations
			if ( tok.checkToken( "<" ) )
				if ( !tok.seekToken( ">" ) ) return raiseError( "FX: expected '>'", tok.getLine() );

			if ( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );

			_buffers.push_back( buffer );
		}
		else if( tok.checkToken( "sampler2D", true ) || tok.checkToken( "samplerCube", true ) ||
				 tok.checkToken( "sampler3D", true ) /*|| tok.checkToken( "samplerBuffer", true )*/ )
		{
			ShaderSampler sampler;
			sampler.sampState = SS_FILTER_TRILINEAR | SS_ANISO8 | SS_ADDR_WRAP;
			sampler.usage = TextureUsage::Texture;

			if( tok.checkToken( "sampler2D" ) )
			{	
				sampler.type = TextureTypes::Tex2D;
				sampler.defTex = (TextureResource *)Modules::resMan().findResource( ResourceTypes::Texture, "$Tex2D" );
			}
			else if( tok.checkToken( "samplerCube" ) )
			{
				sampler.type = TextureTypes::TexCube;
				sampler.defTex = (TextureResource *)Modules::resMan().findResource( ResourceTypes::Texture, "$TexCube" );
			}
			else if( tok.checkToken( "sampler3D" ) )
			{
				sampler.type = TextureTypes::Tex3D;
				sampler.defTex = (TextureResource *)Modules::resMan().findResource( ResourceTypes::Texture, "$Tex3D" );
			}
// 			else if ( tok.checkToken( "samplerBuffer" ) )
// 			{
// 				sampler.type = TextureTypes::Tex2D;
// 				sampler.defTex = ( TextureResource * ) Modules::resMan().findResource( ResourceTypes::Texture, "$Tex2D" );
// 				sampler.sampState = SS_FILTER_POINT | SS_ANISO1 | SS_ADDR_CLAMP;
// 			}

			sampler.id = tok.getToken( identifier );
			if( sampler.id == "" ) return raiseError( "FX: Invalid identifier", tok.getLine() );

			// Skip annotations
			if( tok.checkToken( "<" ) )
				if( !tok.seekToken( ">" ) ) return raiseError( "FX: expected '>'", tok.getLine() );
			
			if( tok.checkToken( "=" ) )
			{
				if( !tok.checkToken( "sampler_state" ) ) return raiseError( "FX: expected 'sampler_state'", tok.getLine() );
				if( !tok.checkToken( "{" ) ) return raiseError( "FX: expected '{'", tok.getLine() );
				while( true )
				{
					if( !tok.hasToken() )
						return raiseError( "FX: expected '}'", tok.getLine() );
					else if( tok.checkToken( "}" ) )
						break;
					else if( tok.checkToken( "Texture" ) )
					{
						if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
						ResHandle texMap =  Modules::resMan().addResource(
							ResourceTypes::Texture, tok.getToken( 0x0 ), 0, false );
						sampler.defTex = (TextureResource *)Modules::resMan().resolveResHandle( texMap );
					}
					else if( tok.checkToken( "TexUnit" ) )
					{
						if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
						sampler.texUnit = (int)atoi( tok.getToken( intnum ) );
						if( sampler.texUnit > ( int ) unitFree.size() - 1/*11*/ ) return raiseError( "FX: texUnit exceeds limit", tok.getLine() );
						if( sampler.texUnit >= 0 ) unitFree[ sampler.texUnit ] = false;
					}
					else if( tok.checkToken( "Address" ) )
					{
						sampler.sampState &= ~SS_ADDR_MASK;
						if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
						if( tok.checkToken( "Wrap" ) ) sampler.sampState |= SS_ADDR_WRAP;
						else if( tok.checkToken( "Clamp" ) ) sampler.sampState |= SS_ADDR_CLAMP;
						else if( tok.checkToken( "Border" ) ) sampler.sampState |= SS_ADDR_CLAMPCOL;
						else return raiseError( "FX: invalid enum value", tok.getLine() );
					}
					else if( tok.checkToken( "Filter" ) )
					{
						sampler.sampState &= ~SS_FILTER_MASK;
						if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
						if( tok.checkToken( "Trilinear" ) ) sampler.sampState |= SS_FILTER_TRILINEAR;
						else if( tok.checkToken( "Bilinear" ) ) sampler.sampState |= SS_FILTER_BILINEAR;
						else if( tok.checkToken( "None" ) ) sampler.sampState |= SS_FILTER_POINT;
						else return raiseError( "FX: invalid enum value", tok.getLine() );
					}
					else if( tok.checkToken( "MaxAnisotropy" ) )
					{
						sampler.sampState &= ~SS_ANISO_MASK;
						if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
						uint32 maxAniso = (uint32)atoi( tok.getToken( intnum ) );
						if( maxAniso <= 1 ) sampler.sampState |= SS_ANISO1;
						else if( maxAniso <= 2 ) sampler.sampState |= SS_ANISO2;
						else if( maxAniso <= 4 ) sampler.sampState |= SS_ANISO4;
						else if( maxAniso <= 8 ) sampler.sampState |= SS_ANISO8;
						else sampler.sampState |= SS_ANISO16;
					}
					else if ( tok.checkToken( "Usage" ) )
					{
						if ( sampler.type != TextureTypes::Tex2D )
							return raiseError( "FX: invalid sampler type is used as compute image, only sampler2D is supported", tok.getLine() );
						if ( !Modules::renderer().getRenderDevice()->getCaps().computeShaders )
							return raiseError( "FX: using texture as compute image is not supported on this version of render interface", tok.getLine() );

						if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
						if ( tok.checkToken( "Texture" ) ) sampler.usage = TextureUsage::Texture;
						else if ( tok.checkToken( "ComputeImageRO" ) ) sampler.usage = TextureUsage::ComputeImageRO;
						else if ( tok.checkToken( "ComputeImageWO" ) ) sampler.usage = TextureUsage::ComputeImageWO;
						else if ( tok.checkToken( "ComputeImageRW" ) ) sampler.usage = TextureUsage::ComputeImageRW;
						else return raiseError( "FX: invalid enum value", tok.getLine() );
					}
					else
						return raiseError( "FX: unexpected token", tok.getLine() );
					if( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );
				}
			}
			if( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );

			_samplers.push_back( sampler );
		}
		else if( tok.checkToken( "context" ) )
		{
			bool success = parseFXSectionContext( tok, identifier, RenderBackendType::OpenGL2 );
			if ( !success )
			{
				return false;
			}
		}
		else if ( tok.checkToken( "OpenGL4" ) )
		{
			if ( !tok.checkToken( "{" ) ) return raiseError( "FX: expected '{'", tok.getLine() );
			while ( true )
			{
				if ( !tok.hasToken() )
					return raiseError( "FX: expected '}'", tok.getLine() );
				else if ( tok.checkToken( "}" ) )
					break;
				else if ( tok.checkToken( "context" ) )
				{
					bool success = parseFXSectionContext( tok, identifier, RenderBackendType::OpenGL4 );
					if ( !success )
					{
						return false;
					}
				}
				else
					return raiseError( "FX: unexpected token", tok.getLine() );
// 				if ( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );
			}
		}
		else if ( tok.checkToken( "OpenGLES3" ) )
		{
			if ( !tok.checkToken( "{" ) ) return raiseError( "FX: expected '{'", tok.getLine() );
			while ( true )
			{
				if ( !tok.hasToken() )
					return raiseError( "FX: expected '}'", tok.getLine() );
				else if ( tok.checkToken( "}" ) )
					break;
				else if ( tok.checkToken( "context" ) )
				{
					bool success = parseFXSectionContext( tok, identifier, RenderBackendType::OpenGLES3 );
					if ( !success )
					{
						return false;
					}
				}
				else
					return raiseError( "FX: unexpected token", tok.getLine() );
				// 				if ( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );
			}
		}
		else
		{
			return raiseError( "FX: unexpected token", tok.getLine() );
		}
	}

	// Automatic texture unit assignment
	for( uint32 i = 0; i < _samplers.size(); ++i )
	{
		if( _samplers[i].texUnit < 0 )
		{	
			for( uint32 j = 0; j < unitFree.size() /*12*/; ++j )
			{
				if( unitFree[j] )
				{
					_samplers[i].texUnit = j;
					unitFree[j] = false;
					break;
				}
			}
			if( _samplers[i].texUnit < 0 )
				return raiseError( "FX: Too many samplers (not enough texture units available)" );
		}
	}
	
	return true;
}


bool ShaderResource::parseFXSectionContext( Tokenizer &tok, const char * identifier, int targetRenderBackend )
{
	ShaderContext context;
	_tmpCodeVS = _tmpCodeFS = _tmpCodeGS = _tmpCodeCS = _tmpCodeTSCtl = _tmpCodeTSEval = "";

	bool vertexShaderAvailable, fragmentShaderAvailable, geometryShaderAvailable, computeShaderAvailable, 
		 tessControlShaderAvailable, tessEvalShaderAvailable;
	vertexShaderAvailable = fragmentShaderAvailable = geometryShaderAvailable = computeShaderAvailable = 
		tessControlShaderAvailable = tessEvalShaderAvailable = false;

	context.id = tok.getToken( identifier );
	if ( context.id == "" ) return raiseError( "FX: Invalid identifier", tok.getLine() );

	// Skip annotations
	if ( tok.checkToken( "<" ) )
		if ( !tok.seekToken( ">" ) ) return raiseError( "FX: expected '>'", tok.getLine() );

	if ( !tok.checkToken( "{" ) ) return raiseError( "FX: expected '{'", tok.getLine() );
	while ( true )
	{
		if ( !tok.hasToken() )
			return raiseError( "FX: expected '}'", tok.getLine() );
		else if ( tok.checkToken( "}" ) )
			break;
		else if ( tok.checkToken( "ZWriteEnable" ) )
		{
			if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
			if ( tok.checkToken( "true" ) ) context.writeDepth = true;
			else if ( tok.checkToken( "false" ) ) context.writeDepth = false;
			else return raiseError( "FX: invalid bool value", tok.getLine() );
		}
		else if ( tok.checkToken( "ZEnable" ) )
		{
			if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
			if ( tok.checkToken( "true" ) ) context.depthTest = true;
			else if ( tok.checkToken( "false" ) ) context.depthTest = false;
			else return raiseError( "FX: invalid bool value", tok.getLine() );
		}
		else if ( tok.checkToken( "ZFunc" ) )
		{
			if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
			if ( tok.checkToken( "LessEqual" ) ) context.depthFunc = TestModes::LessEqual;
			else if ( tok.checkToken( "Always" ) ) context.depthFunc = TestModes::Always;
			else if ( tok.checkToken( "Equal" ) ) context.depthFunc = TestModes::Equal;
			else if ( tok.checkToken( "Less" ) ) context.depthFunc = TestModes::Less;
			else if ( tok.checkToken( "Greater" ) ) context.depthFunc = TestModes::Greater;
			else if ( tok.checkToken( "GreaterEqual" ) ) context.depthFunc = TestModes::GreaterEqual;
			else return raiseError( "FX: invalid enum value", tok.getLine() );
		}
		else if ( tok.checkToken( "BlendMode" ) )
		{
			if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
			if ( tok.checkToken( "Replace" ) )
			{
				context.blendingEnabled = false;
				context.blendStateSrc = BlendModes::Zero;
				context.blendStateDst = BlendModes::Zero;
			}
			else if ( tok.checkToken( "Blend" ) )
			{
				context.blendingEnabled = true; 
				context.blendStateSrc = BlendModes::SrcAlpha;
				context.blendStateDst = BlendModes::OneMinusSrcAlpha;
			}
			else if ( tok.checkToken( "Add" ) )
			{
				context.blendingEnabled = true;
				context.blendStateSrc = BlendModes::One;
				context.blendStateDst = BlendModes::One;
			}
			else if ( tok.checkToken( "AddBlended" ) )
			{
				context.blendingEnabled = true;
				context.blendStateSrc = BlendModes::SrcAlpha;
				context.blendStateDst = BlendModes::One;
			}
			else if ( tok.checkToken( "Mult" ) )
			{
				context.blendingEnabled = true;
				context.blendStateSrc = BlendModes::DestColor;
				context.blendStateDst = BlendModes::Zero;
			}
			else if ( tok.checkToken( "{" ) )
			{
				for ( unsigned int i = 0; i < 2; ++i )
				{
					if ( tok.checkToken( "Zero" ) ) i == 0 ? context.blendStateSrc = BlendModes::Zero : context.blendStateDst = BlendModes::Zero;
					else if ( tok.checkToken( "One" ) ) i == 0 ? context.blendStateSrc = BlendModes::One : context.blendStateDst = BlendModes::One;
					else if ( tok.checkToken( "SrcAlpha" ) ) i == 0 ? context.blendStateSrc = BlendModes::SrcAlpha : context.blendStateDst = BlendModes::SrcAlpha;
					else if ( tok.checkToken( "OneMinusSrcAlpha" ) ) i == 0 ? context.blendStateSrc = BlendModes::OneMinusSrcAlpha : context.blendStateDst = BlendModes::OneMinusSrcAlpha; 
					else if ( tok.checkToken( "DestAlpha" ) ) i == 0 ? context.blendStateSrc = BlendModes::DestAlpha : context.blendStateDst = BlendModes::DestAlpha;
					else if ( tok.checkToken( "OneMinusDestAlpha" ) ) i == 0 ? context.blendStateSrc = BlendModes::OneMinusDestAlpha : context.blendStateDst = BlendModes::OneMinusDestAlpha;
					else if ( tok.checkToken( "DestColor" ) ) i == 0 ? context.blendStateSrc = BlendModes::DestColor : context.blendStateDst = BlendModes::DestColor;
					else if ( tok.checkToken( "SrcColor" ) ) i == 0 ? context.blendStateSrc = BlendModes::SrcColor : context.blendStateDst = BlendModes::SrcColor;
					else if ( tok.checkToken( "OneMinusDestColor" ) ) i == 0 ? context.blendStateSrc = BlendModes::OneMinusDestColor : context.blendStateDst = BlendModes::OneMinusDestColor;
					else if ( tok.checkToken( "OneMinusSrcColor" ) ) i == 0 ? context.blendStateSrc = BlendModes::OneMinusSrcColor : context.blendStateDst = BlendModes::OneMinusSrcColor;
					else return raiseError( "FX: invalid value", tok.getLine() );

					if ( i == 0 && !tok.checkToken( "," ) ) return raiseError( "FX: expected ','", tok.getLine() );
				}

				// Set blending status
				if ( context.blendStateSrc == BlendModes::Zero && context.blendStateDst == BlendModes::Zero ) context.blendingEnabled = false;
				else context.blendingEnabled = true;

				if ( !tok.checkToken( "}" ) ) return raiseError( "FX: expected '}'", tok.getLine() );
			}
			else return raiseError( "FX: invalid enum value", tok.getLine() );
		}
		else if ( tok.checkToken( "CullMode" ) )
		{
			if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
			if ( tok.checkToken( "Back" ) ) context.cullMode = CullModes::Back;
			else if ( tok.checkToken( "Front" ) ) context.cullMode = CullModes::Front;
			else if ( tok.checkToken( "None" ) ) context.cullMode = CullModes::None;
			else return raiseError( "FX: invalid enum value", tok.getLine() );
		}
		else if ( tok.checkToken( "AlphaToCoverage" ) )
		{
			if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
			if ( tok.checkToken( "true" ) || tok.checkToken( "1" ) ) context.alphaToCoverage = true;
			else if ( tok.checkToken( "false" ) || tok.checkToken( "1" ) ) context.alphaToCoverage = false;
			else return raiseError( "FX: invalid bool value", tok.getLine() );
		}
		else if ( tok.checkToken( "TessPatchVertices" ) )
		{
			if ( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
			uint32 patchVerts = ( uint32 ) atoi( tok.getToken( intnum ) );
			if ( patchVerts < 1 ) context.tessVerticesInPatchCount = 1;
			else context.tessVerticesInPatchCount = patchVerts;
		}
		else if ( tok.checkToken( "VertexShader" ) )
		{
			if ( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
				return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
			_tmpCodeVS = tok.getToken( identifier );
			if ( _tmpCodeVS == "" ) return raiseError( "FX: Invalid name", tok.getLine() );

			vertexShaderAvailable = true;
		}
		else if ( tok.checkToken( "GeometryShader" ) )
		{
			if ( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
				return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
			_tmpCodeGS = tok.getToken( identifier );
			if ( _tmpCodeGS == "" ) return raiseError( "FX: Invalid name", tok.getLine() );
			
			geometryShaderAvailable = true;
		}
		else if ( tok.checkToken( "ComputeShader" ) )
		{
			if ( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
				return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
			_tmpCodeCS = tok.getToken( identifier );
			if ( _tmpCodeCS == "" ) return raiseError( "FX: Invalid name", tok.getLine() );

			computeShaderAvailable = true;
		}
		else if ( tok.checkToken( "PixelShader" ) )
		{
			if ( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
				return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
			_tmpCodeFS = tok.getToken( identifier );
			if ( _tmpCodeFS == "" ) return raiseError( "FX: Invalid name", tok.getLine() );

			fragmentShaderAvailable = true;
		}
		else if ( tok.checkToken( "TessControlShader" ) )
		{
			if ( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
				return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
			_tmpCodeTSCtl = tok.getToken( identifier );
			if ( _tmpCodeTSCtl == "" ) return raiseError( "FX: Invalid name", tok.getLine() );

			tessControlShaderAvailable = true;
		}
		else if ( tok.checkToken( "TessEvalShader" ) )
		{
			if ( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
				return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
			_tmpCodeTSEval = tok.getToken( identifier );
			if ( _tmpCodeTSEval == "" ) return raiseError( "FX: Invalid name", tok.getLine() );

			tessEvalShaderAvailable = true;
		}
		else
			return raiseError( "FX: unexpected token", tok.getLine() );
		if ( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );
}

	// Handle shaders
	for ( uint32 i = 0; i < _codeSections.size(); ++i )
	{
		if ( _codeSections[ i ].getName() == _tmpCodeVS ) context.vertCodeIdx = i;
		if ( _codeSections[ i ].getName() == _tmpCodeFS ) context.fragCodeIdx = i;
		if ( _codeSections[ i ].getName() == _tmpCodeCS ) context.computeCodeIdx = i;
		if ( _codeSections[ i ].getName() == _tmpCodeGS ) context.geomCodeIdx = i;
		if ( _codeSections[ i ].getName() == _tmpCodeTSCtl ) context.tessCtlCodeIdx = i;
		if ( _codeSections[ i ].getName() == _tmpCodeTSEval ) context.tessEvalCodeIdx = i;
	}

	// Check that compute shader, if available, is not in the same context with other shaders
	if ( computeShaderAvailable && ( vertexShaderAvailable || fragmentShaderAvailable || geometryShaderAvailable || tessEvalShaderAvailable ) )
	{
		return raiseError( "FX: Compute shader and other types of shaders are detected in context '" + context.id + "'. This is not supported" );
	}

	if ( !computeShaderAvailable ) 
	{
		// Compute shader is a standalone type of shader and is not directly attached to any geometry object, hence it is not a part of VS-GS-FS shader pipeline
		if ( context.vertCodeIdx < 0 )
			return raiseError( "FX: Vertex shader referenced by context '" + context.id + "' not found" );
		if ( context.fragCodeIdx < 0 )
			return raiseError( "FX: Pixel shader referenced by context '" + context.id + "' not found" );
		if ( geometryShaderAvailable )
		{
			if ( !Modules::renderer().getRenderDevice()->getCaps().geometryShaders )
				return raiseError( "FX: Geometry shaders referenced by context '" + context.id + "' are not supported on this device" );
			else if ( context.geomCodeIdx < 0 )
				return raiseError( "FX: Geometry shader referenced by context '" + context.id + "' not found" );
		}
		if ( tessControlShaderAvailable )
		{
			if ( !Modules::renderer().getRenderDevice()->getCaps().tesselation )
				return raiseError( "FX: Tessellation shaders referenced by context '" + context.id + "' are not supported on this device" );
			else if ( context.tessCtlCodeIdx < 0 )
				return raiseError( "FX: Tessellation control shader referenced by context '" + context.id + "' not found" );
		}
		if ( tessEvalShaderAvailable )
		{
			if ( !Modules::renderer().getRenderDevice()->getCaps().tesselation )
				return raiseError( "FX: Tessellation shaders referenced by context '" + context.id + "' are not supported on this device" );
			else if ( context.tessEvalCodeIdx < 0 )
				return raiseError( "FX: Tessellation evaluation shader referenced by context '" + context.id + "' not found" );
		}
	}
	else
	{
		if ( !Modules::renderer().getRenderDevice()->getCaps().computeShaders )
			return raiseError( "FX: Compute shaders referenced by context '" + context.id + "' are not supported on this device" );
		else if ( context.computeCodeIdx < 0 )
			return raiseError( "FX: Compute shader referenced by context '" + context.id + "' not found" );
	}

	// Skip contexts that are intended for other render interfaces
	if ( Modules::renderer().getRenderDeviceType() == targetRenderBackend )
	{
		_contexts.push_back( context );
 	}

	return true;
}


bool ShaderResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;
	
	// Parse sections
	const char *pData = data;
	const char *eof = data + size;
	char *fxCode = 0x0;
	std::vector< std::string > tempCodeSections;
	tempCodeSections.reserve( 16 );
    ShaderParser shp( _name );

    // check if shader is a binary
    if ( size < 5 )
        return raiseError( "Invalid shader resource" );
    
    char header[ 5 ];
    char *pBinData = elemcpy_le( header, (char*)(pData), 5 );
    if( header[ 0 ] == 'H' || header[ 1 ] == '3' || header[ 2 ] == 'D' || header[ 3 ] == 'S' || header[ 4 ] == 'B' )
		_binaryShader = true;
        
    if ( _binaryShader )
    {
        if ( !Modules::renderer().getRenderDevice()->getCaps().binaryShaders )
            return raiseError( "Render device does not support binary shaders" );
        
        if ( !shp.parseBinaryShader( pBinData, size - 5 ) )
        {
            // Reset
            release();
            initDefault();
            
            return false;
        }
        
        // get all data from shader parser and compile contexts
        _contexts.swap( shp._contexts );
        _buffers.swap( shp._buffers );
        _uniforms.swap( shp._uniforms );
        _samplers.swap( shp._samplers );
        _binarySections.swap( shp._binaryShaders );
        
        compileContexts();
        return true;
    }
    
    // standard path (text-based shader)
	while( pData < eof )
	{
		if( pData < eof-1 && *pData == '[' && *(pData+1) == '[' )
		{
			pData += 2;
			
			// Parse section name
			const char *sectionNameStart = pData;
			while( pData < eof && *pData != ']' && *pData != '\n' && *pData != '\r' ) ++pData;
			const char *sectionNameEnd = pData++;

			// Check for correct closing of name
			if( pData >= eof || *pData++ != ']' ) return raiseError( "Error in section name" );
			
			// Parse content
			const char *sectionContentStart = pData;
			while( (pData < eof && *pData != '[') || (pData < eof-1 && *(pData+1) != '[') ) ++pData;
			const char *sectionContentEnd = pData;
			
			if( sectionNameEnd - sectionNameStart == 2 &&
			    *sectionNameStart == 'F' && *(sectionNameStart+1) == 'X' )
			{
				// FX section
				if ( fxCode != 0x0 )
				{
					delete[] fxCode; fxCode = 0; 
					return raiseError( "More than one FX section" );
				}

				fxCode = new char[ sectionContentEnd - sectionContentStart + 1 ];
				memcpy( fxCode, sectionContentStart, sectionContentEnd - sectionContentStart );
				fxCode[sectionContentEnd - sectionContentStart] = '\0';
			}
			else
			{
				// Add section as private code resource which is not managed by resource manager
				_tmpCodeVS.assign( sectionNameStart, sectionNameEnd );
				_codeSections.push_back( CodeResource( _tmpCodeVS, 0 ) );
 				_tmpCodeVS.assign( sectionContentStart, sectionContentEnd );
				tempCodeSections.push_back( _tmpCodeVS );
				// 				_codeSections.back().load( _tmpCodeVS.c_str(), (uint32)_tmpCodeVS.length() );
			}
		}
		else
			++pData;
	}

	if( fxCode == 0x0 ) return raiseError( "Missing FX section" );
	bool result = parseFXSection( fxCode );
	delete[] fxCode; fxCode = 0x0;
	if( !result ) return false;

	// Load only code sections that are required for contexts
	for ( size_t i = 0; i < _contexts.size(); ++i )
	{
		ShaderContext &ctx = _contexts[ i ];
		for ( size_t codeItr = 0; codeItr < _codeSections.size(); ++codeItr )
		{
			if ( ctx.vertCodeIdx == codeItr || ctx.fragCodeIdx == codeItr || ctx.geomCodeIdx == codeItr ||
				 ctx.tessCtlCodeIdx == codeItr || ctx.tessEvalCodeIdx == codeItr || ctx.computeCodeIdx == codeItr )
			{
				_codeSections[ codeItr ].load( tempCodeSections[ codeItr ].c_str(), ( uint32 ) tempCodeSections[ codeItr ].length() );
			}
		}
	}
	
// 	// Remove unneeded code sections !!!Shader index remap is needed for each context, crash otherwise
// 	int counter = ( int ) _codeSections.size() - 1;
// 	while ( counter >= 0 )
// 	{
// 		if ( !_codeSections[ counter ].isLoaded() ) _codeSections.erase( _codeSections.begin() + counter );
// 		counter--;
// 	}

	compileContexts();
	
	return true;
}


void ShaderResource::preLoadCombination( uint32 combMask )
{
	if( !_loaded )
	{
		_preLoadList.insert( combMask );
	}
	else
	{
		for( uint32 i = 0; i < _contexts.size(); ++i )
		{
			if( getCombination( _contexts[i], combMask ) == 0x0 )
				_preLoadList.insert( combMask );
		}
	}
}


bool ShaderResource::compileCombination( ShaderContext &context, ShaderCombination &sc )
{
	uint32 combMask = sc.combMask;
	
	// Add preamble
	_tmpCodeVS = _vertPreamble;
	_tmpCodeFS = _fragPreamble;
	_tmpCodeGS = _geomPreamble;
	_tmpCodeCS = _computePreamble;
	_tmpCodeTSCtl = _tessCtlPreamble;
	_tmpCodeTSEval = _tessEvalPreamble;

	// Insert defines for flags
	if( combMask != 0 )
	{
		_tmpCodeVS += "\r\n// ---- Flags ----\r\n";
		_tmpCodeFS += "\r\n// ---- Flags ----\r\n";
		_tmpCodeGS += "\r\n// ---- Flags ----\r\n";
		_tmpCodeTSCtl += "\r\n// ---- Flags ----\r\n";
		_tmpCodeTSEval += "\r\n// ---- Flags ----\r\n";
		_tmpCodeCS += "\r\n// ---- Flags ----\r\n";

		for( uint32 i = 1; i <= 32; ++i )
		{
			if( combMask & (1 << (i-1)) )
			{
				_tmpCodeVS += "#define _F";
				_tmpCodeVS += ( char ) ( 48 + i / 10 );
				_tmpCodeVS += ( char ) ( 48 + i % 10 );
				_tmpCodeVS += "_\r\n";

				_tmpCodeFS += "#define _F";
				_tmpCodeFS += ( char ) ( 48 + i / 10 );
				_tmpCodeFS += ( char ) ( 48 + i % 10 );
				_tmpCodeFS += "_\r\n";
				
				// geometry
				_tmpCodeGS += "#define _F";
				_tmpCodeGS += ( char ) ( 48 + i / 10 );
				_tmpCodeGS += ( char ) ( 48 + i % 10 );
				_tmpCodeGS += "_\r\n";

				// tessellation
				_tmpCodeTSCtl += "#define _F";
				_tmpCodeTSCtl += ( char ) ( 48 + i / 10 );
				_tmpCodeTSCtl += ( char ) ( 48 + i % 10 );
				_tmpCodeTSCtl += "_\r\n";

				_tmpCodeTSEval += "#define _F";
				_tmpCodeTSEval += ( char ) ( 48 + i / 10 );
				_tmpCodeTSEval += ( char ) ( 48 + i % 10 );
				_tmpCodeTSEval += "_\r\n";

				// compute
				_tmpCodeCS += "#define _F";
				_tmpCodeCS += ( char ) ( 48 + i / 10 );
				_tmpCodeCS += ( char ) ( 48 + i % 10 );
				_tmpCodeCS += "_\r\n";
			}
		}

		_tmpCodeVS += "// ---------------\r\n";
		_tmpCodeFS += "// ---------------\r\n";
		_tmpCodeGS += "// ---------------\r\n";
		_tmpCodeTSCtl += "// ---------------\r\n";
		_tmpCodeTSEval += "// ---------------\r\n";
		_tmpCodeCS += "// ---------------\r\n";
	}

	// Add actual shader code
	bool vsAvailable, fsAvailable, csAvailable, gsAvailable, tscAvailable, tseAvailable;
	vsAvailable = fsAvailable = csAvailable = gsAvailable = tscAvailable = tseAvailable = false;

	if ( context.vertCodeIdx >= 0 )
	{
		_tmpCodeVS += getCode( context.vertCodeIdx )->assembleCode();
		vsAvailable = true;
	}
	if ( context.fragCodeIdx >= 0 )
	{
		_tmpCodeFS += getCode( context.fragCodeIdx )->assembleCode();
		fsAvailable = true;
	}
	if ( context.geomCodeIdx >= 0 )
	{
		_tmpCodeGS += getCode( context.geomCodeIdx )->assembleCode();
		gsAvailable = true;
	}
	if ( context.tessCtlCodeIdx >= 0 )
	{
		_tmpCodeTSCtl += getCode( context.tessCtlCodeIdx )->assembleCode();
		tscAvailable = true;
	}
	if ( context.tessEvalCodeIdx >= 0 )
	{
		_tmpCodeTSEval += getCode( context.tessEvalCodeIdx )->assembleCode();
		tseAvailable = true;
	}
	if ( context.computeCodeIdx >= 0 )
	{
		_tmpCodeCS += getCode( context.computeCodeIdx )->assembleCode();
		csAvailable = true;
	}

	Modules::log().writeInfo( "---- C O M P I L I N G  . S H A D E R . %s@%s[%i] ----",
		_name.c_str(), context.id.c_str(), sc.combMask );
	
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// Unload shader if necessary
	if( sc.shaderObj != 0 )
	{
		rdi->destroyShader( sc.shaderObj );
		sc.shaderObj = 0;
	}
	
	// Compile shader
	bool compiled = Modules::renderer().createShaderComb( sc, 
														  vsAvailable ? _tmpCodeVS.c_str() : 0,
														  fsAvailable ? _tmpCodeFS.c_str() : 0,  
														  gsAvailable ? _tmpCodeGS.c_str() : 0,
														  tscAvailable ? _tmpCodeTSCtl.c_str() : 0,
														  tseAvailable ? _tmpCodeTSEval.c_str() : 0,
														  csAvailable ? _tmpCodeCS.c_str() : 0
														  );
	if( !compiled )
	{
		Modules::log().writeError( "Shader resource '%s': Failed to compile shader context '%s' (comb %i)",
			_name.c_str(), context.id.c_str(), sc.combMask );

		if( Modules::config().dumpFailedShaders )
		{
			bool shaderAvailability[ 6 ] = { vsAvailable, fsAvailable, gsAvailable, tscAvailable, tseAvailable, csAvailable };
			std::string dumpFileName;
			std::string *output;

			for ( size_t i = 0; i < 6; ++i )
			{
				if ( shaderAvailability[ i ] == true )
				{
					switch ( i )
					{
						case 0 : // vertex shader
							dumpFileName = "shdDumpVS.txt"; output = &_tmpCodeVS; break;
						case 1:  // fragment shader
							dumpFileName = "shdDumpFS.txt"; output = &_tmpCodeFS; break;
						case 2:  // geometry shader
							dumpFileName = "shdDumpGS.txt"; output = &_tmpCodeGS; break;
						case 3:  // tessellation control shader
							dumpFileName = "shdDumpTSC.txt"; output = &_tmpCodeTSCtl; break;
						case 4:  // tessellation evaluation shader
							dumpFileName = "shdDumpTSE.txt"; output = &_tmpCodeTSEval; break;
						case 5:  // compute shader
							dumpFileName = "shdDumpCS.txt"; output = &_tmpCodeCS; break;
						default:
							break;
					}

                    std::ofstream out( dumpFileName.c_str(), ios::binary );
					if ( out.good() ) out << *output;
					out.close();
				}
			}
		}
	}
	else
	{
		rdi->bindShader( sc.shaderObj );

		// Find samplers in compiled shader
		sc.samplersLocs.reserve( _samplers.size() );
		for( uint32 i = 0; i < _samplers.size(); ++i )
		{
			int samplerLoc = rdi->getShaderSamplerLoc( sc.shaderObj, _samplers[i].id.c_str() );
			sc.samplersLocs.push_back( samplerLoc );
			
			// Set texture unit
			if( samplerLoc >= 0 )
				rdi->setShaderSampler( samplerLoc, _samplers[i].texUnit );
		}
		
		// Find buffers in compiled shader
		sc.bufferLocs.reserve( _buffers.size() );
		for ( uint32 i = 0; i < _buffers.size(); ++i )
		{
			int bufferLoc = rdi->getShaderBufferLoc( sc.shaderObj, _buffers[ i ].id.c_str() );
			sc.bufferLocs.push_back( bufferLoc );
		}

		// Find uniforms in compiled shader
		sc.uniLocs.reserve( Modules::renderer().totalEngineUniforms() + _uniforms.size() );
		for( uint32 i = 0; i < _uniforms.size(); ++i )
		{
			sc.uniLocs.push_back(
				rdi->getShaderConstLoc( sc.shaderObj, _uniforms[i].id.c_str() ) );
		}
	}

	rdi->bindShader( 0 );

	// Output shader log
	if( rdi->getShaderLog() != "" )
		Modules::log().writeInfo( "Shader resource '%s': ShaderLog: %s", _name.c_str(), rdi->getShaderLog().c_str() );

	return compiled;
}


bool ShaderResource::compileBinaryCombination( ShaderContext &context, ShaderCombination &sc )
{
    // we have all the necessary data, either whole program, or binary shaders
	Modules::log().writeInfo( "---- C O M P I L I N G  . S H A D E R . %s@%s[%i] ----",
                              _name.c_str(), context.id.c_str(), sc.combMask );
	
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// Unload shader if necessary
	if( sc.shaderObj != 0 )
	{
		rdi->destroyShader( sc.shaderObj );
		sc.shaderObj = 0;
	}
	
	// Compile shader
	RDIShaderCreateParams scp;
    
	bool compiled = Modules::renderer().createShaderComb( sc,
                                                          scp
														  );
	if( !compiled )
	{
		Modules::log().writeError( "Shader resource '%s': Failed to compile shader context '%s' (comb %i)",
			_name.c_str(), context.id.c_str(), sc.combMask );
	}
	else
	{
		rdi->bindShader( sc.shaderObj );

		// Find samplers in compiled shader
		sc.samplersLocs.reserve( _samplers.size() );
		for( uint32 i = 0; i < _samplers.size(); ++i )
		{
			int samplerLoc = rdi->getShaderSamplerLoc( sc.shaderObj, _samplers[i].id.c_str() );
			sc.samplersLocs.push_back( samplerLoc );
			
			// Set texture unit
			if( samplerLoc >= 0 )
				rdi->setShaderSampler( samplerLoc, _samplers[i].texUnit );
		}
		
		// Find buffers in compiled shader
		sc.bufferLocs.reserve( _buffers.size() );
		for ( uint32 i = 0; i < _buffers.size(); ++i )
		{
			int bufferLoc = rdi->getShaderBufferLoc( sc.shaderObj, _buffers[ i ].id.c_str() );
			sc.bufferLocs.push_back( bufferLoc );
		}

		// Find uniforms in compiled shader
		sc.uniLocs.reserve( Modules::renderer().totalEngineUniforms() + _uniforms.size() );
		for( uint32 i = 0; i < _uniforms.size(); ++i )
		{
			sc.uniLocs.push_back(
				rdi->getShaderConstLoc( sc.shaderObj, _uniforms[i].id.c_str() ) );
		}
	}

	rdi->bindShader( 0 );

	// Output shader log
	if( rdi->getShaderLog() != "" )
		Modules::log().writeInfo( "Shader resource '%s': ShaderLog: %s", _name.c_str(), rdi->getShaderLog().c_str() );

	return compiled;    
}

void ShaderResource::compileContexts()
{
	for( uint32 i = 0; i < _contexts.size(); ++i )
	{
		ShaderContext &context = _contexts[i];

		if( context.compiled )
			continue;
		
        if ( !_binaryShader )
        {
            context.flagMask = 0;

            if ( ( context.vertCodeIdx >= 0 && !getCode( context.vertCodeIdx )->tryLinking( &context.flagMask ) ) ||
                ( context.fragCodeIdx >= 0 && !getCode( context.fragCodeIdx )->tryLinking( &context.flagMask ) ) ||
                ( context.geomCodeIdx >= 0 && !getCode( context.geomCodeIdx )->tryLinking( &context.flagMask ) ) ||
                ( context.tessCtlCodeIdx >= 0 && !getCode( context.tessCtlCodeIdx )->tryLinking( &context.flagMask ) ) ||
                ( context.tessEvalCodeIdx >= 0 && !getCode( context.tessEvalCodeIdx )->tryLinking( &context.flagMask ) ) ||
                ( context.computeCodeIdx >= 0 && !getCode( context.computeCodeIdx )->tryLinking( &context.flagMask ) )
            )
            {
                continue;
            }
        }
        
		// Add preloaded combinations
		for( std::set< uint32 >::iterator itr = _preLoadList.begin(); itr != _preLoadList.end(); ++itr )
		{
			uint32 combMask = *itr & context.flagMask;
				
			// Check if combination already exists
			bool found = false;
			for( size_t j = 0; j < context.shaderCombs.size(); ++j )
			{
				if( context.shaderCombs[j].combMask == combMask )
				{
					found = true;
					break;
				}
			}

			if( !found )
			{	
				context.shaderCombs.push_back( ShaderCombination() );
				context.shaderCombs.back().combMask = combMask;
			}
		}
			
		bool combinationsCompileStatus = true;
		for( size_t j = 0; j < context.shaderCombs.size(); ++j )
		{
            if ( !_binaryShader )
                combinationsCompileStatus &= compileCombination( context, context.shaderCombs[j] );
            else
                combinationsCompileStatus &= compileBinaryCombination( context, context.shaderCombs[j] );
		}

		context.compiled = combinationsCompileStatus;
	}
}


ShaderCombination *ShaderResource::getCombination( ShaderContext &context, uint32 combMask )
{
	if( !context.compiled ) return 0x0;
	
	// Kill combination bits that are not used by the context
	combMask &= context.flagMask;
	
	// Try to find combination
	std::vector< ShaderCombination > &combs = context.shaderCombs;
	for( size_t i = 0, s = combs.size(); i < s; ++i )
	{
		if( combs[i].combMask == combMask ) return &combs[i];
	}

	// Add combination
	combs.push_back( ShaderCombination() );
	combs.back().combMask = combMask;
	compileCombination( context, combs.back() );

	return &combs.back();
}


uint32 ShaderResource::calcCombMask( const std::vector< std::string > &flags )
{	
	uint32 combMask = 0;
	
	for( size_t i = 0, s = flags.size(); i < s; ++i )
	{
		const string &flag = flags[i];
		
		// Check format: _F<digit><digit>_
		if( flag.length() < 5 ) continue;
		if( flag[0] != '_' || flag[1] != 'F' || flag[4] != '_' ||
		    flag[2] < 48 || flag[2] > 57 || flag[3] < 48 || flag[3] > 57 ) continue;
		
		uint32 num = (flag[2] - 48) * 10 + (flag[3] - 48);
		combMask |= 1 << (num - 1);
	}
	
	return combMask;
}


bool ShaderResource::createBinaryShaderStream( uint8 *&data, uint32 &dataSize )
{
	if ( !data || !*data ) return false;

	char *d = ( char *) data;
	uint32 usedMem = 0;

	auto raiseErrorAndClean = [&]( const std::string &msg )
	{
		Modules::log().writeError( msg.c_str() );
		return false;
	};

	auto pushElemU16 = [&]( uint16 value )
	{
		usedMem += 2;
		return elemset_le<uint16>( (uint16 *) d, value );
	};

	auto pushElemU32 = [&]( uint32 value )
	{
		usedMem += 4;
		return elemset_le<uint32>( (uint32 *) d, value );
	};

	auto pushElemF32 = [&]( float value )
	{
		usedMem += 4;
		return elemset_le<float>( (float *) d, value );
	};

	auto pushElemChar = [&]( const char *str, uint32 count )
	{
		usedMem += count;
		return elemcpyd_le( (char*) d, str, count );
	};

	// add header
	d = pushElemChar( "H3DSB", 5 );

	// add version
	d = pushElemU16( 1 );

	// add render backend type
	d = pushElemU16( Modules::renderer().getRenderDeviceType() );

	// add generator name
	char nameBuf[ 64 ] = { 0 };
	auto rndName = Modules::renderer().getRenderDevice()->getRendererName();
	strncpy_s( nameBuf, 64, rndName.c_str(), 64 );
//	auto rndSize = rndName.size() > 64 ? 64 : rndName.size();
	d = pushElemChar( nameBuf, 64 );

	// add generator version
	char verBuf[ 64 ] = { 0 };
	auto rndVersion = Modules::renderer().getRenderDevice()->getRendererVersion();
	strncpy_s( verBuf, 64, rndVersion.c_str(), 64 );
	d = pushElemChar( verBuf, 64 );

	// add shader binary type
	d = pushElemU16( ShaderForm::BinaryDeviceDependent );

	// add fx section
	// samplers
	d = pushElemU16( _samplers.size() );
	for( ShaderSampler &s : _samplers )
	{
		// type
		int samplerType = 0;
		if ( s.type == TextureTypes::Tex2D ) samplerType = 2;
		else if ( s.type == TextureTypes::Tex3D ) samplerType = 3;
		else if ( s.type == TextureTypes::TexCube ) samplerType = 6;

		d = pushElemU16( samplerType );

		// id
		if ( s.id.size() <= 255 )
		{
			d = pushElemU16( s.id.size() );
			d = pushElemChar( s.id.c_str(), s.id.size() );
		}
		else
		{
			d = pushElemU16( 255 );
			d = pushElemChar( s.id.substr( 0, 255 ).c_str(), 255 );
		}

		// texid
		auto &samplerTexName = s.defTex.getPtr()->getName();
		if ( samplerTexName.size() <= 255 )
		{
			d = pushElemU16( samplerTexName.size() );
			d = pushElemChar( samplerTexName.c_str(), samplerTexName.size() );
		}
		else
		{
			d = pushElemU16( 255 );
			d = pushElemChar( samplerTexName.substr( 0, 255 ).c_str(), 255 );
		}

		// texunit
		d = pushElemU16( s.texUnit );

		// tex address
		if ( s.sampState & SS_ADDR_WRAP ) 			d = pushElemU16( 0 );
		else if ( s.sampState & SS_ADDR_CLAMP ) 	d = pushElemU16( 1 );
		else if ( s.sampState & SS_ADDR_CLAMPCOL ) 	d = pushElemU16( 2 );
		else 										d = pushElemU16( 0 ); // default - wrap

		// tex filter
		if ( s.sampState & SS_FILTER_POINT ) 			d = pushElemU16( 0 );
		else if ( s.sampState & SS_FILTER_BILINEAR ) 	d = pushElemU16( 1 );
		else if ( s.sampState & SS_FILTER_TRILINEAR ) 	d = pushElemU16( 2 );
		else 											d = pushElemU16( 2 ); // default - trilinear

		// anisotropy
		if ( s.sampState & SS_ANISO1 ) 			d = pushElemU16( 1 );
		else if ( s.sampState & SS_ANISO2 ) 	d = pushElemU16( 2 );
		else if ( s.sampState & SS_ANISO4 ) 	d = pushElemU16( 4 );
		else if ( s.sampState & SS_ANISO8 ) 	d = pushElemU16( 8 );
		else if ( s.sampState & SS_ANISO16 ) 	d = pushElemU16( 16 );
		else 									d = pushElemU16( 8 ); // default - 8 aniso

		// usage
		d = pushElemU16( s.usage );
	}

	// uniforms
	d = pushElemU16( _uniforms.size() );
	for( ShaderUniform &u : _uniforms )
	{
		// type
		if ( u.size == 1 )
			d = pushElemU16( 0 );
		else if ( u.size == 4 )
			d = pushElemU16( 1 );

		// id
		if ( u.id.size() <= 255 )
		{
			d = pushElemU16( u.id.size() );
			d = pushElemChar( u.id.c_str(), u.id.size() );
		}
		else
		{
			d = pushElemU16( 255 );
			d = pushElemChar( u.id.substr( 0, 255 ).c_str(), 255 );
		}

		// number of default values
		d = pushElemU16( u.size );

		// default values
		if ( u.size == 1 )
			d = pushElemF32( u.defValues[ 0 ] );
		else if ( u.size == 4 )
		{
			d = pushElemChar( (const char *) u.defValues, 4 * sizeof( float ) );
//			d = elemcpyd_le( (float *) d, u.defValues, 4 );
		}
	}

	// buffers
	d = pushElemU16( _buffers.size() );
	for( ShaderBuffer &b : _buffers )
	{
		// id
		if ( b.id.size() <= 255 )
		{
			d = pushElemU16( b.id.size() );
			d = pushElemChar( b.id.c_str(), b.id.size() );
		}
		else
		{
			d = pushElemU16( 255 );
			d = pushElemChar( b.id.substr( 0, 255 ).c_str(), 255 );
		}
	}

	// flags
	// TODO: currently write zero flags
	d = pushElemU16( 0 );

	// contexts
	uint32 totalShaderCombs = 0;

	d = pushElemU16( _contexts.size() );
	for ( ShaderContext &ctx : _contexts )
	{
		// id
		if ( ctx.id.size() <= 255 )
		{
			d = pushElemU16( ctx.id.size() );
			d = pushElemChar( ctx.id.c_str(), ctx.id.size() );
		}
		else
		{
			d = pushElemU16( 255 );
			d = pushElemChar( ctx.id.substr( 0, 255 ).c_str(), 255 );
		}

		// render interface
		d = pushElemU16( Modules::renderer().getRenderDeviceType() );

		// context options
		d = pushElemU16( ctx.writeDepth ); // ZWriteEnable
		d = pushElemU16( ctx.depthTest ); // ZEnable
		d = pushElemU16( ctx.depthFunc ); // ZFunc

		// blend
		if ( ctx.blendStateSrc == BlendModes::Zero && ctx.blendStateDst == BlendModes::Zero )
		{
			d = pushElemU16( 0 ); // replace
			d = pushElemU16( 0 );
		}
		else if ( ctx.blendStateSrc == BlendModes::SrcAlpha && ctx.blendStateDst == BlendModes::OneMinusSrcAlpha )
		{
			d = pushElemU16( 1 ); // blend
			d = pushElemU16( 0 );
		}
		else if ( ctx.blendStateSrc == BlendModes::One && ctx.blendStateDst == BlendModes::One )
		{
			d = pushElemU16( 2 ); // add
			d = pushElemU16( 0 );
		}
		else if ( ctx.blendStateSrc == BlendModes::SrcAlpha && ctx.blendStateDst == BlendModes::One )
		{
			d = pushElemU16( 3 ); // AddBlended
			d = pushElemU16( 0 );
		}
		else if ( ctx.blendStateSrc == BlendModes::DestColor && ctx.blendStateDst == BlendModes::Zero )
		{
			d = pushElemU16( 4 ); // Mult
			d = pushElemU16( 0 );
		}
		else // separate blend modes
		{
			d = pushElemU16( ctx.blendStateSrc + 10 );
			d = pushElemU16( ctx.blendStateDst + 10 );
		}

		// options continued
		d = pushElemU16( ctx.cullMode );
		d = pushElemU16( ctx.alphaToCoverage );
		d = pushElemU16( ctx.tessVerticesInPatchCount );

		// flag mask
		d = pushElemU32( ctx.flagMask );

		// number of shader combinations
		d = pushElemU16( ctx.shaderCombs.size() );
		totalShaderCombs += ctx.shaderCombs.size();
	}

	// shader combinations
	uint32 curContext = 0, curShaderComb = 0;

	d = pushElemU16( totalShaderCombs );
	for( uint32 sc = 0; sc < totalShaderCombs; ++sc )
	{
		if ( curShaderComb >= _contexts[ curContext ].shaderCombs.size() )
		{
			curContext++;
			curShaderComb = 0;
		}

		auto &ctx = _contexts[ curContext ];
		auto &comb = ctx.shaderCombs[ curShaderComb ];

		d = pushElemU16( curContext );
		d = pushElemU32( comb.combMask );

		// OpenGL only supports dumping whole programs, so always say that we have one shader and it is always Vertex shader
		d = pushElemU16( 1 );
		d = pushElemU16( ShaderType::Vertex );

		uint32 binSize = 0;
		uint32 binFormat = 0;
		uint8 *bin = nullptr;
		if ( !Modules::renderer().getRenderDevice()->getShaderBinary( comb.shaderObj, bin, &binFormat, &binSize ) )
		{
			return raiseErrorAndClean( "Failed to obtain binary program!" );
		}

		// check the size of the binary, do we have enough space?
		if ( binSize > dataSize - ( usedMem + 8 ) ) // 8 is bytes for binSize and binFormat
		{
			delete[] bin;
			return raiseErrorAndClean( "Not enough memory to hold binary shader data!" );
		}

		// add size
		d = pushElemU32( binSize );

		// add format
		d = pushElemU32( binFormat );

		// copy data
		memcpy( d, bin, binSize );
		d += binSize + 1;
		usedMem += binSize;
		delete[] bin;

		curShaderComb++;
	}

	dataSize = usedMem;
	return true;
}


int ShaderResource::getElemCount( int elem ) const
{
	switch( elem )
	{
	case ShaderResData::ContextElem:
		return (int)_contexts.size();
	case ShaderResData::SamplerElem:
		return (int)_samplers.size();
	case ShaderResData::UniformElem:
		return (int)_uniforms.size();
	case ShaderResData::ShaderElem:
		return 1;
	default:
		return Resource::getElemCount( elem );
	}
}


int ShaderResource::getElemParamI( int elem, int elemIdx, int param ) const
{
	switch( elem )
	{
	case ShaderResData::UniformElem:
		if( (unsigned)elemIdx < _uniforms.size() )
		{
			switch( param )
			{
			case ShaderResData::UnifSizeI:
				return _uniforms[elemIdx].size;
			}
		}
		break;
	case ShaderResData::SamplerElem:
		if( (unsigned)elemIdx < _samplers.size() )
		{
			switch( param )
			{
			case ShaderResData::SampDefTexResI:
				return _samplers[elemIdx].defTex ? _samplers[elemIdx].defTex->getHandle() : 0;
			}
		}
	case ShaderResData::ShaderElem:
		switch( param )
		{
			case ShaderResData::ShaderTypeI:
				return _binaryShader ? 1 : 0;
			case ShaderResData::ShaderBinarySizeI:
				return (int) _binaryShaderSize;
		}
	}
	
	return Resource::getElemParamI( elem, elemIdx, param );
}


float ShaderResource::getElemParamF( int elem, int elemIdx, int param, int compIdx ) const
{
	switch( elem )
	{
	case ShaderResData::UniformElem:
		if( (unsigned)elemIdx < _uniforms.size() )
		{
			switch( param )
			{
			case ShaderResData::UnifDefValueF4:
				if( (unsigned)compIdx < 4 ) return _uniforms[elemIdx].defValues[compIdx];
				break;
			}
		}
		break;
	}
	
	return Resource::getElemParamF( elem, elemIdx, param, compIdx );
}


void ShaderResource::setElemParamF( int elem, int elemIdx, int param, int compIdx, float value )
{
	switch( elem )
	{
	case ShaderResData::UniformElem:
		if( (unsigned)elemIdx < _uniforms.size() )
		{	
			switch( param )
			{
			case ShaderResData::UnifDefValueF4:
				if( (unsigned)compIdx < 4 )
				{	
					_uniforms[elemIdx].defValues[compIdx] = value;
					return;
				}
				break;
			}
		}
		break;
	}
	
	Resource::setElemParamF( elem, elemIdx, param, compIdx, value );
}


const char *ShaderResource::getElemParamStr( int elem, int elemIdx, int param ) const
{
	switch( elem )
	{
	case ShaderResData::ContextElem:
		if( (unsigned)elemIdx < _contexts.size() )
		{
			switch( param )
			{
			case ShaderResData::ContNameStr:
				return _contexts[elemIdx].id.c_str();
			}
		}
		break;
	case ShaderResData::SamplerElem:
		if( (unsigned)elemIdx < _samplers.size() )
		{
			switch( param )
			{
			case ShaderResData::SampNameStr:
				return _samplers[elemIdx].id.c_str();
			}
		}
		break;
	case ShaderResData::UniformElem:
		if( (unsigned)elemIdx < _uniforms.size() )
		{
			switch( param )
			{
			case ShaderResData::UnifNameStr:
				return _uniforms[elemIdx].id.c_str();
			}
		}
		break;
	}
	
	return Resource::getElemParamStr( elem, elemIdx, param );
}


void *ShaderResource::mapStream( int elem, int elemIdx, int stream, bool read, bool write )
{
	if ( read )
	{
		if( elem == ShaderResData::ShaderElem && stream == ShaderResData::ShaderBinaryStream &&
		    elemIdx < getElemCount( elem ) )
		{
			RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

			uint32 bufSize = 2 * 1024 * 1024;
			_mappedData = Modules::renderer().useScratchBuf( bufSize, 0 ); // 2 mb should be enough for most cases
																		   // additional checks are done during dumping
			if ( !createBinaryShaderStream( _mappedData, bufSize ) )
			{
				// allocate a bigger buffer and try again
				bufSize = 8 * 1024 * 1024;
				_mappedData = Modules::renderer().useScratchBuf( bufSize, 0 );
				if ( !createBinaryShaderStream( _mappedData, bufSize ) )
				{
					Modules::log().writeDebugInfo( "Please fix maximum buffer size for binary shaders!" );
					return nullptr;
				}
			}

			_binaryShaderSize = bufSize;
			return _mappedData;
		}
	}

	return Resource::mapStream( elem, elemIdx, stream, read, write );
}


void ShaderResource::unmapStream()
{
	if( _mappedData != nullptr )
	{
		_mappedData = nullptr;
		return;
	}

	Resource::unmapStream();
}

}  // namespace
