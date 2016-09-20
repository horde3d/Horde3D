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

#include "egShader.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include <fstream>
#include <cstring>

#include "utDebug.h"


namespace Horde3D {

using namespace std;

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
string ShaderResource::_tessPreamble = "";
string ShaderResource::_computePreamble = "";
bool ShaderResource::_defaultPreambleSet = false;

string ShaderResource::_tmpCodeVS = "";
string ShaderResource::_tmpCodeFS = "";
string ShaderResource::_tmpCodeGS = "";
string ShaderResource::_tmpCodeCS = "";
string ShaderResource::_tmpCodeTSCtl = "";
string ShaderResource::_tmpCodeTSEval = "";


ShaderResource::ShaderResource( const string &name, int flags ) :
	Resource( ResourceTypes::Shader, name, flags )
{
	initDefault();
}


ShaderResource::~ShaderResource()
{
	release();
}


void ShaderResource::initDefault()
{
	if ( !_defaultPreambleSet )
	{
		// specify default version preamble for shaders
		if ( Modules::renderer().getRenderDeviceType() == RenderBackendType::OpenGL4 )
		{
			_vertPreamble = "#version 330\r\n";
			_fragPreamble = "#version 330\r\n";
			_geomPreamble = "#version 330\r\n";
			_tessPreamble = "#version 410\r\n";
			_computePreamble = "#version 430\r\n";
		}

		_defaultPreambleSet = true;
	}
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
	
	// Parsing
	const char *identifier = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	const char *intnum = "+-0123456789";
	const char *floatnum = "+-0123456789.eE";

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
				uniform.defValues[0] = (float)atof( tok.getToken( floatnum ) );
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
				uniform.defValues[0] = (float)atof( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[1] = (float)atof( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[2] = (float)atof( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[3] = (float)atof( tok.getToken( floatnum ) );
				if( !tok.checkToken( "}" ) ) return raiseError( "FX: expected '}'", tok.getLine() );
			}
			if( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );

			_uniforms.push_back( uniform );
		}
		else if( tok.checkToken( "sampler2D", true ) || tok.checkToken( "samplerCube", true ) ||
				 tok.checkToken( "sampler3D", true ) || tok.checkToken( "samplerBuffer", true ) )
		{
			ShaderSampler sampler;
			sampler.sampState = SS_FILTER_TRILINEAR | SS_ANISO8 | SS_ADDR_WRAP;

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
			else if ( tok.checkToken( "samplerBuffer" ) )
			{
				sampler.type = TextureTypes::Tex2D;
				sampler.defTex = ( TextureResource * ) Modules::resMan().findResource( ResourceTypes::Texture, "$Tex2D" );
				sampler.sampState = SS_FILTER_POINT | SS_ANISO1 | SS_ADDR_CLAMP;
			}

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
	
	const char *intnum = "+-0123456789";
	const char *floatnum = "+-0123456789.eE";

	bool geometryShaderAvailable, computeShaderAvailable, tessControlShaderAvailable, tessEvalShaderAvailable;
	geometryShaderAvailable = computeShaderAvailable = tessControlShaderAvailable = tessEvalShaderAvailable = false;

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
			if ( tok.checkToken( "Replace" ) ) context.blendMode = BlendModes::Replace;
			else if ( tok.checkToken( "Blend" ) ) context.blendMode = BlendModes::Blend;
			else if ( tok.checkToken( "Add" ) ) context.blendMode = BlendModes::Add;
			else if ( tok.checkToken( "AddBlended" ) ) context.blendMode = BlendModes::AddBlended;
			else if ( tok.checkToken( "Mult" ) ) context.blendMode = BlendModes::Mult;
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

	if ( !computeShaderAvailable ) 
	{
		// compute shader is a standalone type of shader and is not directly attached to any geometry object, hence it is not a part of VS-GS-FS shader pipeline
		if ( context.vertCodeIdx < 0 )
			return raiseError( "FX: Vertex shader referenced by context '" + context.id + "' not found" );
		if ( context.fragCodeIdx < 0 )
			return raiseError( "FX: Pixel shader referenced by context '" + context.id + "' not found" );
		if ( geometryShaderAvailable && context.geomCodeIdx < 0 )
			return raiseError( "FX: Geometry shader referenced by context '" + context.id + "' not found" );
		if ( tessControlShaderAvailable && context.tessCtlCodeIdx < 0 )
			return raiseError( "FX: Tessellation control shader referenced by context '" + context.id + "' not found" );
		if ( tessEvalShaderAvailable && context.tessEvalCodeIdx < 0 )
			return raiseError( "FX: Tessellation evaluation shader referenced by context '" + context.id + "' not found" );
	}
	else
	{
		if ( context.computeCodeIdx < 0 )
			return raiseError( "FX: Compute shader referenced by context '" + context.id + "' not found" );
	}

	// skip contexts that are intended for other render interfaces
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
				if( fxCode != 0x0 ) return raiseError( "More than one FX section" );
				fxCode = new char[sectionContentEnd - sectionContentStart + 1];
				memcpy( fxCode, sectionContentStart, sectionContentEnd - sectionContentStart );
				fxCode[sectionContentEnd - sectionContentStart] = '\0';
			}
			else
			{
				// Add section as private code resource which is not managed by resource manager
				_tmpCodeVS.assign( sectionNameStart, sectionNameEnd );
				_codeSections.push_back( CodeResource( _tmpCodeVS, 0 ) );
				_tmpCodeVS.assign( sectionContentStart, sectionContentEnd );
				_codeSections.back().load( _tmpCodeVS.c_str(), (uint32)_tmpCodeVS.length() );
			}
		}
		else
			++pData;
	}

	if( fxCode == 0x0 ) return raiseError( "Missing FX section" );
	bool result = parseFXSection( fxCode );
	delete[] fxCode; fxCode = 0x0;
	if( !result ) return false;

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
	_tmpCodeTSCtl = _tmpCodeTSEval = _tessPreamble;

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
														  vsAvailable ? _tmpCodeVS.c_str() : nullptr,
														  fsAvailable ? _tmpCodeFS.c_str() : nullptr,  
														  gsAvailable ? _tmpCodeGS.c_str() : nullptr,
														  tscAvailable ? _tmpCodeTSCtl.c_str() : nullptr,
														  tseAvailable ? _tmpCodeTSEval.c_str() : nullptr,
														  csAvailable ? _tmpCodeCS.c_str() : nullptr
														  );
	if( !compiled )
	{
		Modules::log().writeError( "Shader resource '%s': Failed to compile shader context '%s' (comb %i)",
			_name.c_str(), context.id.c_str(), sc.combMask );

		if( Modules::config().dumpFailedShaders )
		{
			std::ofstream out0( "shdDumpVS.txt", ios::binary ), out1( "shdDumpFS.txt", ios::binary );
			if( out0.good() ) out0 << _tmpCodeVS;
			if( out1.good() ) out1 << _tmpCodeFS;
			out0.close();
			out1.close();
		}
	}
	else
	{
		rdi->bindShader( sc.shaderObj );

		// Find samplers in compiled shader
		sc.customSamplers.reserve( _samplers.size() );
		for( uint32 i = 0; i < _samplers.size(); ++i )
		{
			int samplerLoc = rdi->getShaderSamplerLoc( sc.shaderObj, _samplers[i].id.c_str() );
			sc.customSamplers.push_back( samplerLoc );
			
			// Set texture unit
			if( samplerLoc >= 0 )
				rdi->setShaderSampler( samplerLoc, _samplers[i].texUnit );
		}
		
		// Find uniforms in compiled shader
		sc.customUniforms.reserve( _uniforms.size() );
		for( uint32 i = 0; i < _uniforms.size(); ++i )
		{
			sc.customUniforms.push_back(
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
			combinationsCompileStatus &= compileCombination( context, context.shaderCombs[j] );
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

}  // namespace
