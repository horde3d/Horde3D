#include "egShaderParser.h"
#include "egPrerequisites.h"
#include "egModules.h"
#include <utEndian.h>
#include "egCom.h"
#include "egRenderer.h"

namespace Horde3D {
    
ShaderParser::ShaderParser( const std::string &shaderName ) : _shaderName( shaderName )
{
    
}

ShaderParser::~ShaderParser()
{
    
}

bool ShaderParser::raiseError( const std::string& msg, int line )
{
    if( line < 0 )
		Modules::log().writeError( "Shader resource '%s': %s", _shaderName.c_str(), msg.c_str() );
	else
		Modules::log().writeError( "Shader resource '%s': %s (line %i)", _shaderName.c_str(), msg.c_str(), line );
	
	return false;
}

bool ShaderParser::parseBinarySampler( char *data, uint32 samplerCount )
{
    if ( !data || samplerCount == 0 ) return false;
    
    std::vector< uint8 > unitFree( Modules::renderer().getRenderDevice()->getCaps().maxTexUnitCount - 4, true ); // I don't understand why marciano excluded 4 texunits, but currently I'll leave it this way  

    _samplers.reserve( samplerCount );
    for( size_t i = 0; i < samplerCount; ++i )
    {
        ShaderSampler sampler;
        sampler.sampState = SS_FILTER_TRILINEAR | SS_ANISO8 | SS_ADDR_WRAP;
        sampler.usage = TextureUsage::Texture;
        
        uint16 samplersType;
        data = elemcpy_le( &samplersType, (uint16*)( data ), 1 );
        switch( samplersType )
        {
            case 2: // sampler2D
                sampler.type = TextureTypes::Tex2D; 
                sampler.defTex = (TextureResource *)Modules::resMan().findResource( ResourceTypes::Texture, "$Tex2D" );
                break;
            case 3: // sampler3D
                sampler.type = TextureTypes::Tex3D;
                sampler.defTex = (TextureResource *)Modules::resMan().findResource( ResourceTypes::Texture, "$Tex3D" );
                break;
            case 6: // samplerCube
                sampler.type = TextureTypes::TexCube;
                sampler.defTex = (TextureResource *)Modules::resMan().findResource( ResourceTypes::Texture, "$TexCube" );
                break;
            default:
                return raiseError( "Unknown sampler type for sampler " + std::to_string( i ) );
        }
        
        uint16 samplerIdSize;
        data = elemcpy_le( &samplerIdSize, (uint16*)( data ), 1 );
        if ( samplerIdSize == 0 || samplerIdSize > 255 )
        {
            return raiseError( "Incorrect sampler id for sampler " + std::to_string( i ) );
        }
        
        char id[ 256 ] = { 0 };
        data = elemcpy_le( id, (char*)( data ), samplerIdSize );
        sampler.id = std::string( id );
        
        uint16 samplerTexNameSize;
        data = elemcpy_le( &samplerTexNameSize, (uint16*)( data ), 1 );
        if ( samplerTexNameSize > 255 )
        {
            return raiseError( "Sampler texture name larger than 255 chars. Sampler " + std::to_string( i ) );
        }
        
        if ( samplerTexNameSize > 0 )
        {
            char idtex[ 256 ] = { 0 };
            data = elemcpy_le( idtex, (char*)( data ), samplerTexNameSize );
            
            ResHandle texMap =  Modules::resMan().addResource( ResourceTypes::Texture, idtex, 0, false );
            sampler.defTex = (TextureResource *)Modules::resMan().resolveResHandle( texMap );
        }
        
        short samplerTexUnit;
        data = elemcpy_le( &samplerTexUnit, (short*)( data ), 1 );
        sampler.texUnit = samplerTexUnit; // default -1
        if ( sampler.texUnit < -1 ) return raiseError( "Incorrect texUnit for sampler " + std::to_string( i ) );
        if( sampler.texUnit > ( int ) unitFree.size() - 1 ) return raiseError( "texUnit exceeds limit for sampler " + std::to_string( i ) );
        if( sampler.texUnit >= 0 ) unitFree[ sampler.texUnit ] = false;
        
        uint16 samplerAddress;
        data = elemcpy_le( &samplerAddress, (uint16*)( data ), 1 );
        sampler.sampState &= ~SS_ADDR_MASK;
        switch( samplerAddress )
        {
            case 0: // Wrap - default
                sampler.sampState |= SS_ADDR_WRAP;
                break;
            case 1: // Clamp
                sampler.sampState |= SS_ADDR_CLAMP;
                break;
            case 2: // Border
                sampler.sampState |= SS_ADDR_CLAMPCOL;
                break;
            default:
                return raiseError( "Unsupported sampler address value for sampler " + std::to_string( i ) );
        }
        
        uint16 samplerFilter;
        data = elemcpy_le( &samplerFilter, (uint16*)( data ), 1 );
        switch( samplerFilter )
        {
            case 0: // None
                sampler.sampState |= SS_FILTER_POINT;
                break;
            case 1: // Bilinear
                sampler.sampState |= SS_FILTER_BILINEAR;
                break;
            case 2: // Trilinear - default
                sampler.sampState |= SS_FILTER_TRILINEAR;
                break;
            default:
                return raiseError( "Unsupported sampler filter value for sampler " + std::to_string( i ) );
        }
        
        uint16 samplerMaxAnisotropy;
        data = elemcpy_le( &samplerMaxAnisotropy, (uint16*)( data ), 1 );
        switch( samplerMaxAnisotropy )
        {
            case 1:
                sampler.sampState |= SS_ANISO1;
                break;
            case 2:
                sampler.sampState |= SS_ANISO2;
                break;
            case 4:
                sampler.sampState |= SS_ANISO4;
                break;
            case 8: // default
                sampler.sampState |= SS_ANISO8;
                break;
            case 16:
                sampler.sampState |= SS_ANISO16;
                break;
            default:
                return raiseError( "Unsupported max anisotropy value for sampler " + std::to_string( i ) );
        }
        
        uint16 samplerUsage;
        data = elemcpy_le( &samplerUsage, (uint16*)( data ), 1 );

        switch( samplerUsage )
        {
            case 0: // use as texture - default
                sampler.usage = TextureUsage::Texture;
                break;
            case 1: // use as read-only compute image
                sampler.usage = TextureUsage::ComputeImageRO;
                break;
            case 2: // use as write-only compute image
                sampler.usage = TextureUsage::ComputeImageWO;
                break;
            case 3: // use as read-write compute image
                sampler.usage = TextureUsage::ComputeImageRW;
                break;
            default:
                return raiseError( "Unsupported texture usage, sampler " + std::to_string( i ) );
        }
        
        if ( samplerUsage != 0 && sampler.type != TextureTypes::Tex2D )
            return raiseError( "Invalid sampler type is used as compute image, only sampler2D is supported, sampler " 
                                + std::to_string( i ) );
    
        if ( samplerUsage != 0 && !Modules::renderer().getRenderDevice()->getCaps().computeShaders )
            return raiseError( "Using texture as compute image is not supported on this version of render interface, "
                               "sampler " + std::to_string( i ) );
    }
    
    return true;
}

bool ShaderParser::parseBinaryUniforms( char *data, uint32 variablesCount )
{
    // main uniform parsing function
    auto parseUniform = []( char *data, ShaderUniform *uni )
    {
        uint16 uniformIdSize;
        data = elemcpy_le( &uniformIdSize, (uint16*)( data ), 1 );
        
        // uniform id cannot be larger than 256 characters
        if ( uniformIdSize == 0 || uniformIdSize > 255 ) return false;
        
        char uniformName[ 256 ] = { 0 };
        data = elemcpy_le( uniformName, ( char * ) (data), uniformIdSize );
        if ( uniformName[ 0 ] == '\0' ) return false;
        
        uint16 uniformDefValuesSize;
        data = elemcpy_le( &uniformDefValuesSize, (uint16*)( data ), 1 );
        
        data = elemcpy_le( uni->defValues, (float*)( data ), uniformDefValuesSize );
        uni->id = uniformName;
        
        return true;
    };
    
    // find out uniform count for each type
    uint16 float4_uniforms;
    data = elemcpy_le( &float4_uniforms, (uint16*)( data ), 1 );

    uint16 float_uniforms;
    data = elemcpy_le( &float_uniforms, (uint16*)( data ), 1 );

    if ( float4_uniforms + float_uniforms != variablesCount ) 
        return raiseError( "Incorrect number of uniforms!" );
        
    // parsing for each uniform type
    _uniforms.reserve( variablesCount );
    for( size_t i = 0; i < float4_uniforms; i++ )
    {
        ShaderUniform uni;
        uni.size = 4;
        
        if ( !parseUniform( data, &uni ) )
            return raiseError( "Failed to parse uniform float4 " + std::to_string( i ) );
        
        _uniforms.emplace_back( uni );
    }
    
    for( size_t i = 0; i < float_uniforms; i++ )
    {
        ShaderUniform uni;
        uni.size = 1;
        
        if ( !parseUniform( data, &uni ) )
            return raiseError( "Failed to parse uniform float " + std::to_string( i ) );
        
        _uniforms.emplace_back( uni );
    }
    
    return true;
}

bool ShaderParser::parseBinaryShader( char *data, uint32 size )
{
    uint16 version;
    data = elemcpy_le( &version, (uint16*)( data ), 1 );
    if( version != 1 ) return raiseError( "Unsupported version of binary shader file" );
    
    uint16 renderBackendType;
    data = elemcpy_le( &renderBackendType, (uint16*)( data ), 1 );
    if( renderBackendType != RenderBackendType::OpenGL4 && renderBackendType != RenderBackendType::OpenGLES3 && 
        renderBackendType != 256 /* Null backend */ ) 
        return raiseError( "Unsupported render backend for binary shader file" );
    
    // check generator/driver name and version
    char generatorName[ 32 ] = { 0 };
    data = elemcpy_le( generatorName, ( char * ) (data), 32 );
    if ( generatorName[ 0 ] == '\0' ) raiseError( "Corrupt binary shader file" );
    
    char generatorVersion[ 16 ] = { 0 };
    data = elemcpy_le( generatorVersion, ( char * ) (data), 16 );
    if ( generatorVersion[ 0 ] == '\0' ) raiseError( "Corrupt binary shader file" );
    
    if ( strncmp( generatorName, Modules::renderer().getRenderDevice()->getRendererName().c_str(), 32 ) != 0 )
        return raiseError( "Binary shader was created with different driver/generator. Skipping." );
        
    if ( strncmp( generatorVersion, Modules::renderer().getRenderDevice()->getRendererVersion().c_str(), 16 ) != 0 )
        Modules::log().writeWarning( "Binary shader was created with different version of driver/generator." 
                                     "Shader might not work as expected." );
    
    // get fx section data from binary shader
    // order is the following:
    // - samplers
    // - uniforms
    // - buffers
    // - contexts
    // - flags
    // - shader combinations for each context
    uint16 samplersCount;
    data = elemcpy_le( &samplersCount, (uint16*)( data ), 1 );

    if ( samplersCount )
    {
        if ( !parseBinarySampler( data, samplersCount ) ) return false;
    }
    
    uint16 variablesCount;
    data = elemcpy_le( &variablesCount, (uint16*)( data ), 1 );
    
    if ( variablesCount )
    {
        if ( !parseBinaryUniforms( data, variablesCount ) ) return false;
    }
    
    return true;
}

} // namespace
