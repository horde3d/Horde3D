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
        // setting default state for sampler
        ShaderSampler sampler;
        sampler.sampState = SS_FILTER_TRILINEAR | SS_ANISO8 | SS_ADDR_WRAP;
        sampler.usage = TextureUsage::Texture;
        
        // sampler type
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
        
        // sampler id
        uint16 samplerIdSize;
        data = elemcpy_le( &samplerIdSize, (uint16*)( data ), 1 );
        if ( samplerIdSize == 0 || samplerIdSize > 255 )
        {
            return raiseError( "Incorrect sampler id for sampler " + std::to_string( i ) );
        }
        
        char id[ 256 ] = { 0 };
        data = elemcpy_le( id, (char*)( data ), samplerIdSize );
        sampler.id = std::string( id );
        
        // sampler texture
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
        
        // sampler texunit
        short samplerTexUnit;
        data = elemcpy_le( &samplerTexUnit, (short*)( data ), 1 );
        sampler.texUnit = samplerTexUnit; // default -1
        if ( sampler.texUnit < -1 ) return raiseError( "Incorrect texUnit for sampler " + std::to_string( i ) );
        if( sampler.texUnit > ( int ) unitFree.size() - 1 ) return raiseError( "texUnit exceeds limit for sampler " + std::to_string( i ) );
        if( sampler.texUnit >= 0 ) unitFree[ sampler.texUnit ] = false;
        
        // sampler texture options
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
        
        // sampler filtering options
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
        
        // sampler anisotropy options
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
        
        // sampler usage, mainly for compute
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
    if ( !data || variablesCount == 0 ) return false;

    // main uniform parsing function
    auto parseUniform = []( char *&data, ShaderUniform *uni )
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
        if ( uniformDefValuesSize != uni->size ) return false;

        data = elemcpy_le( uni->defValues, (float*)( data ), uniformDefValuesSize );
        uni->id = uniformName;
        
        return true;
    };
    
    // find out uniform count for each type
//     uint16 float4_uniforms;
//     data = elemcpy_le( &float4_uniforms, (uint16*)( data ), 1 );
// 
//     uint16 float_uniforms;
//     data = elemcpy_le( &float_uniforms, (uint16*)( data ), 1 );
// 
//     if ( float4_uniforms + float_uniforms != variablesCount ) 
//         return raiseError( "Incorrect number of uniforms!" );
        
    // parsing for each uniform type
    _uniforms.reserve( variablesCount );
    for( size_t i = 0; i < variablesCount; i++ )
    {
        ShaderUniform uni;
        
        uint16 uniformType;
        data = elemcpy_le( &uniformType, (uint16*)( data ), 1 );

        if ( uniformType == 0 ) uni.size = 1;
        else if ( uniformType == 1 ) uni.size = 4;
        else return raiseError( "Failed to parse uniform type for uniform " + std::to_string( i ) );
  
        if ( !parseUniform( data, &uni ) )
            return raiseError( "Failed to parse uniform " + std::to_string( i ) );
        
        _uniforms.emplace_back( uni );
    }
    
//     for( size_t i = 0; i < float_uniforms; i++ )
//     {
//         ShaderUniform uni;
//         uni.size = 1;
//         
//         if ( !parseUniform( data, &uni ) )
//             return raiseError( "Failed to parse uniform float " + std::to_string( i ) );
//         
//         _uniforms.emplace_back( uni );
//     }
    
    return true;
}


bool ShaderParser::parseBinaryBuffer( char *data, uint32 bufferCount )
{
    // parsing for each buffer
    _buffers.reserve( bufferCount );
    for( size_t i = 0; i < bufferCount; i++ )
    {
        ShaderBuffer buf;
        
        uint16 bufferIdSize;
        data = elemcpy_le( &bufferIdSize, (uint16*)( data ), 1 );
        
        // uniform id cannot be larger than 256 characters
        if ( bufferIdSize == 0 || bufferIdSize > 255 ) 
            return raiseError( "Incorrect buffer id size for buffer " + std::to_string( i ) );
        
        char bufferName[ 256 ] = { 0 };
        data = elemcpy_le( bufferName, ( char * ) (data), bufferIdSize );
        if ( bufferName[ 0 ] == '\0' ) 
            return raiseError( "Incorrect buffer id for buffer " + std::to_string( i ) );
        
        buf.id = bufferName;
        
        _buffers.emplace_back( buf );
    }
    
    return true;
}


bool ShaderParser::parseBinaryFlags( char *data, uint32 flagCount )
{
    // parsing for each flag - currently the same as parsing buffers
    _flags.reserve( flagCount );
    for( size_t i = 0; i < flagCount; i++ )
    {
        ShaderFlag flag;
        
        uint16 flagIdSize;
        data = elemcpy_le( &flagIdSize, (uint16*)( data ), 1 );
        
        // uniform id cannot be larger than 256 characters
        if ( flagIdSize == 0 || flagIdSize > 255 ) 
            return raiseError( "Incorrect flag id size for flag " + std::to_string( i ) );
        
        char flagName[ 256 ] = { 0 };
        data = elemcpy_le( flagName, ( char * ) (data), flagIdSize );
        if ( flagName[ 0 ] == '\0' ) 
            return raiseError( "Incorrect flag id for flag " + std::to_string( i ) );
        
        // save parsed value
        flag.id = flagName;
        
        _flags.emplace_back( flag );
    }
    
    return true;
}


bool ShaderParser::parseBinaryContexts( char *data, uint32 contextCount )
{
    _contexts.reserve( contextCount );
    for( size_t i = 0; i < contextCount; ++i )
    {
        ShaderContext ctx;
        
        uint16 contextIdSize;
        data = elemcpy_le( &contextIdSize, (uint16*)( data ), 1 );
        
        // context id cannot be larger than 256 characters
        if ( contextIdSize == 0 || contextIdSize > 255 ) 
            return raiseError( "Incorrect context id size for context " + std::to_string( i ) );;
        
        char contextName[ 256 ] = { 0 };
        data = elemcpy_le( contextName, ( char * ) (data), contextIdSize );
        if ( contextName[ 0 ] == '\0' ) 
            return raiseError( "Incorrect context id for context " + std::to_string( i ) );
        
        ctx.id = contextName;
        
        // what render interfaces can use this context 
        uint16 contextApplicability;
        data = elemcpy_le( &contextApplicability, (uint16*)( data ), 1 );
        
        switch( contextApplicability )
        {
//            case RenderBackendType::OpenGL2 :
            case RenderBackendType::OpenGL4 :
            case RenderBackendType::OpenGLES3 :
                break;
            default:
                return raiseError( "Incorrect render backend specified for context " + std::to_string( i ) );
        }
        
        ctx.renderInterfaces = contextApplicability;
        
        //
        // context options
        //
        
        // ZWriteEnable
        uint16 contextZWriteEnable;
        data = elemcpy_le( &contextZWriteEnable, (uint16*)( data ), 1 );
        
        if ( contextZWriteEnable == 0 || contextZWriteEnable == 1 ) ctx.writeDepth = contextZWriteEnable;
        else return raiseError( "Incorrect ZWriteEnable value for context " + std::to_string( i ) );
        
        // ZEnable
        uint16 contextZEnable;
        data = elemcpy_le( &contextZEnable, (uint16*)( data ), 1 );
        
        if ( contextZEnable == 0 || contextZEnable == 1 ) ctx.depthTest = contextZEnable;
        else return raiseError( "Incorrect ZEnable value for context " + std::to_string( i ) );
        
        // ZFunc
        uint16 contextZFunc;
        data = elemcpy_le( &contextZFunc, (uint16*)( data ), 1 );
        
        switch( contextZFunc )
        {
            case 0: // Always
                ctx.depthFunc = TestModes::Always;
                break;
            case 1: // Equal
                ctx.depthFunc = TestModes::Equal;
                break;
            case 2: // Less
                ctx.depthFunc = TestModes::Less;
                break;
            case 3: // LessEqual - default
                ctx.depthFunc = TestModes::LessEqual;
                break;
            case 4: // Greater
                ctx.depthFunc = TestModes::Greater;
                break;
            case 5: // GreaterEqual
                ctx.depthFunc = TestModes::GreaterEqual;
                break;
            default:
                return raiseError( "Incorrect ZFunc value for context " + std::to_string( i ) );
        }
        
        // BlendMode
        uint16 contextBlendMode, contextBlendMode2;
        data = elemcpy_le( &contextBlendMode, (uint16*)( data ), 1 );
        data = elemcpy_le( &contextBlendMode2, (uint16*)( data ), 1 );
                
        if ( contextBlendMode2 == 0 )
        {
            // Blend mode used the old way - Replace, Blend, Add, Mult
            switch( contextBlendMode )
            {
                case 0: // Replace
                    ctx.blendStateSrc = ctx.blendStateDst = BlendModes::Zero;
                    ctx.blendingEnabled = false;
                case 1: // Blend
                    ctx.blendingEnabled = true; 
                    ctx.blendStateSrc = BlendModes::SrcAlpha;
                    ctx.blendStateDst = BlendModes::OneMinusSrcAlpha;
                    break;
                case 2: // Add
                    ctx.blendingEnabled = true; 
                    ctx.blendStateSrc = BlendModes::One;
                    ctx.blendStateDst = BlendModes::One;
                    break;
                case 3: // AddBlended
                    ctx.blendingEnabled = true; 
                    ctx.blendStateSrc = BlendModes::SrcAlpha;
                    ctx.blendStateDst = BlendModes::One;
                    break;
                case 4: // Mult
                    ctx.blendingEnabled = true; 
                    ctx.blendStateSrc = BlendModes::DestColor;
                    ctx.blendStateDst = BlendModes::Zero;
                    break;
                        
                default:
                    break;
            }
        }
        else
        {
            // Blend mode used the new way - separate definitions for source and destination
            for( int blendOp = 0; blendOp < 2; ++blendOp )
            {
                uint16 blendData = blendOp == 0 ? contextBlendMode : contextBlendMode2;
                BlendModes::List &target = blendOp == 0 ? ctx.blendStateSrc : ctx.blendStateDst;
                
                if ( blendData == 10 || /* Zero */
                     blendData == 11 || /* One */
                     blendData == 12 || /* SrcAlpha */
                     blendData == 13 || /* OneMinusSrcAlpha */
                     blendData == 14 || /* DestAlpha */
                     blendData == 15 || /* OneMinusDestAlpha */
                     blendData == 16 || /* DestColor */
                     blendData == 17 || /* SrcColor */
                     blendData == 18 || /* OneMinusDestColor */
                     blendData == 19    /* OneMinusSrcColor */ )
                    
                    target = ( BlendModes::List) blendData;
                else
                    return raiseError( "Incorrect blend mode for " + 
                                       (blendOp == 0 ? std::string( "source" ) : std::string( "destination" )) + 
                                       std::string( " in context " ) + std::to_string( i ) );
            }
            
            // Set blending status
            if ( ctx.blendStateSrc == BlendModes::Zero && ctx.blendStateDst == BlendModes::Zero ) ctx.blendingEnabled = false;
            else ctx.blendingEnabled = true;
        }
        
        // Cull mode
        uint16 contextCullMode;
        data = elemcpy_le( &contextCullMode, (uint16*)( data ), 1 );
        switch( contextCullMode )
        {
            case 0: /* Back */
                ctx.cullMode = CullModes::Back;
                break;
            case 1: /* Front */
                ctx.cullMode = CullModes::Front;
                break;
            case 2: /* None */
                ctx.cullMode = CullModes::None;
                break;
            default:
                return raiseError( "Incorrect cull mode for context " + std::to_string( i ) );
        }
        
        // AlphaToCoverage
        uint16 contextAlphaToCoverage;
        data = elemcpy_le( &contextAlphaToCoverage, (uint16*)( data ), 1 );
        if ( contextAlphaToCoverage == 0 || /* false */
             contextAlphaToCoverage == 1    /* true */ )
            ctx.alphaToCoverage = contextAlphaToCoverage;
        else
            return raiseError( "Incorrect alpha to coverage value for context " + std::to_string( i ) );
        
        // TessPatchVertices
        uint16 contextTessPatchVertices;
        data = elemcpy_le( &contextTessPatchVertices, (uint16*)( data ), 1 );
        if ( contextTessPatchVertices >= 1 && contextTessPatchVertices <= 32 )
            ctx.tessVerticesInPatchCount = contextTessPatchVertices;
        else
            return raiseError( "Incorrect tessellation patch vertices value for context " + std::to_string( i ) );
        
        //
        // context shaders
        //
        
        // used flags
        uint32 contextFlagMask;
        data = elemcpy_le( &contextFlagMask, (uint32*)( data ), 1 );
        
        ctx.flagMask = contextFlagMask;
        
        // shader combinations
        uint16 contextCombinations;
        data = elemcpy_le( &contextCombinations, (uint16*)( data ), 1 );
        
        if ( contextCombinations > 255 )
            return raiseError( "Incorrect number of shader combinations for context " + std::to_string( i ) );
        
        ctx.shaderCombs.resize( contextCombinations );
        
        // shader code sections indices
 /*       data = elemcpy_le( &ctx.vertCodeIdx, (int*)( data ), 1 );
        data = elemcpy_le( &ctx.fragCodeIdx, (int*)( data ), 1 );
        data = elemcpy_le( &ctx.geomCodeIdx, (int*)( data ), 1 );
        data = elemcpy_le( &ctx.tessEvalCodeIdx, (int*)( data ), 1 );
        data = elemcpy_le( &ctx.tessCtlCodeIdx, (int*)( data ), 1 );
        data = elemcpy_le( &ctx.computeCodeIdx, (int*)( data ), 1 );
        
        // sanity check
        if ( ctx.computeCodeIdx != -1 && 
           ( ctx.vertCodeIdx != -1 || ctx.fragCodeIdx != -1 || ctx.geomCodeIdx != -1 || ctx.tessEvalCodeIdx != -1 || ctx.tessCtlCodeIdx != -1 ) )
            return raiseError( "Using compute shader and other types of shaders in one context is not supported. Context " + std::to_string( i ) );
            
        if ( ctx.computeCodeIdx == -1 ) 
        {
            // Compute shader is a standalone type of shader and is not directly attached to any geometry object, hence it is not a part of VS-GS-FS shader pipeline
            if ( ctx.vertCodeIdx < 0 )
                return raiseError( "Vertex shader referenced by context '" + ctx.id + "' not found" );
            if ( ctx.fragCodeIdx < 0 )
                return raiseError( "Pixel shader referenced by context '" + ctx.id + "' not found" );
            if ( ctx.geomCodeIdx != -1 )
            {
                if ( !Modules::renderer().getRenderDevice()->getCaps().geometryShaders )
                    return raiseError( "Geometry shaders referenced by context '" + ctx.id + "' are not supported on this device" );
                else if ( ctx.geomCodeIdx < 0 )
                    return raiseError( "Geometry shader referenced by context '" + ctx.id + "' not found" );
            }
            if ( ctx.tessCtlCodeIdx != -1 )
            {
                if ( !Modules::renderer().getRenderDevice()->getCaps().tesselation )
                    return raiseError( "Tessellation shaders referenced by context '" + ctx.id + "' are not supported on this device" );
                else if ( ctx.tessCtlCodeIdx < 0 )
                    return raiseError( "Tessellation control shader referenced by context '" + ctx.id + "' not found" );
            }
            if ( ctx.tessEvalCodeIdx != -1 )
            {
                if ( !Modules::renderer().getRenderDevice()->getCaps().tesselation )
                    return raiseError( "Tessellation shaders referenced by context '" + ctx.id + "' are not supported on this device" );
                else if ( ctx.tessEvalCodeIdx < 0 )
                    return raiseError( "Tessellation evaluation shader referenced by context '" + ctx.id + "' not found" );
            }
        }
        else
        {
            if ( !Modules::renderer().getRenderDevice()->getCaps().computeShaders )
                return raiseError( "Compute shaders referenced by context '" + ctx.id + "' are not supported on this device" );
            else if ( ctx.computeCodeIdx < 0 )
                return raiseError( "Compute shader referenced by context '" + ctx.id + "' not found" );
        }
        */
 
        // save context for future use
        _contexts.emplace_back( ctx );
    }
    
    return true;
}


bool ShaderParser::parseBinaryContextShaderCombs( char *data, uint32 shaderCombs )
{
    for( size_t i = 0; i < shaderCombs; ++i )
    {
        // id of the context used by this combination - starts from 0
        uint16 combinationContextId;
        data = elemcpy_le( &combinationContextId, (uint16*)( data ), 1 );
        
        if ( combinationContextId > _contexts.size() - 1 )
            return raiseError( "Incorrect context id for shader combination " + std::to_string( i ) );
        
        // combination mask
        uint32 combinationMask;
        data = elemcpy_le( &combinationMask, (uint32*)( data ), 1 );
        
        // total shader count in the combination
        uint16 combinationShaderCount;
        data = elemcpy_le( &combinationShaderCount, (uint16*)( data ), 1 );
        
        for( size_t combShader = 0; combShader < combinationShaderCount; ++combShader )
        {
            ShaderBinaryData bin;
            
            // binary shader type 
            uint16 combinationShaderType;
            data = elemcpy_le( &combinationShaderType, (uint16*)( data ), 1 );
            
            if ( combinationShaderType >= ShaderType::LastElement ) 
                return raiseError( "Incorrect shader type for shader " + std::to_string( combShader ) + ", shader combination id " + 
                                    std::to_string( i ) + ", context id " + std::to_string( combinationContextId ) );
                
            // binary shader size 
            uint32 combinationShaderSize;
            data = elemcpy_le( &combinationShaderSize, (uint32*)( data ), 1 );
            
            // shader data
            uint8_t *combinationShaderData = new uint8_t[ combinationShaderSize ];
            data = elemcpy_le( combinationShaderData, (uint8*)( data ), combinationShaderSize );
            
            bin.combinationId = i;
            bin.contextId = combinationContextId;
            bin.shaderType = combinationShaderType;
            bin.dataSize = combinationShaderSize;
            bin.data = combinationShaderData;
            
            _binaryShaders.emplace_back( bin );
        }

        // further processing is done in the egShader, parsing is complete
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
    
    // shader type
    uint16 binaryType;
    data = elemcpy_le( &binaryType, (uint16*)( data ), 1 );
    if( binaryType == ShaderForm::BinaryDeviceDependent || binaryType == ShaderForm::BinarySPIRV ) 
        _shaderType = binaryType;
    else
        return raiseError( "Unsupported type of binary shader file" );
    
    // get fx section data from binary shader
    // order is the following:
    // - samplers
    // - uniforms
    // - buffers
    // - flags
    // - contexts
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
    
    uint16 buffersCount;
    data = elemcpy_le( &buffersCount, (uint16*)( data ), 1 );
    
    if ( buffersCount )
    {
        if ( !parseBinaryBuffer( data, buffersCount ) ) return false;
    }
    
    uint16 flagsCount;
    data = elemcpy_le( &flagsCount, (uint16*)( data ), 1 );
    if ( flagsCount )
    {
        if ( !parseBinaryFlags( data, flagsCount ) ) return false;
    }
    
    uint16 contextCount;
    data = elemcpy_le( &contextCount, (uint16*)( data ), 1 );
    if ( contextCount )
    {
        if ( !parseBinaryContexts( data, contextCount ) ) return false;
    }
    
    uint16 shaderCombsCount;
    data = elemcpy_le( &shaderCombsCount, (uint16*)( data ), 1 );
    if ( shaderCombsCount )
    {
        if ( !parseBinaryContextShaderCombs( data, shaderCombsCount ) ) return false;
    }
    
    return true;
}

} // namespace
