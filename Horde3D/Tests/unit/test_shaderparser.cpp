#include "../catch.hpp"
#include <memory>
#include <iostream>
#include <vector>

#include "../../Source/Horde3DEngine/egShaderParser.h"
#include "../../Source/Horde3DEngine/egModules.h"
#include "../../Source/Horde3DEngine/egRenderer.h"
#include "../../Source/Horde3DEngine/egRendererBaseNull.h"

using namespace Horde3D;

#define NULL_RENDER_BACKEND 256

class TestShaderParser : public ShaderParser
{
public:
    
    TestShaderParser( const std::string shaderName ) : ShaderParser( shaderName )
    {
        // Init engine
        _initialized = Modules::init( NULL_RENDER_BACKEND );
        if ( !_initialized )
            std::cout << "---Engine failed to initialize. Test will likely fail/crash!---" << std::endl;
    }
    
    ~TestShaderParser()
    {
        if ( _initialized )
            Modules::release();
    }
    
    bool test_parseBinarySampler( char *data, uint32_t samplerCount )
    {
        return parseBinarySampler( data, samplerCount );
    }
    
    bool test_parseBinaryUniform( char *data, uint32_t variablesCount )
    {
        return parseBinaryUniforms( data, variablesCount );
    }

    bool test_parseBinaryBuffer( char *data, uint32_t bufferCount )
    {
        return parseBinaryBuffer( data, bufferCount );
    }

    bool test_parseBinaryFlags( char *data, uint32_t flagCount )
    {
        return parseBinaryFlags( data, flagCount );
    }

    bool test_parseBinaryContext( char *data, uint32_t contextCount )
    {
        return parseBinaryContexts( data, contextCount );
    }

    bool test_parseBinaryCombination( char *data, uint32_t combinationCount )
    {
        return parseBinaryContextShaderCombs( data, combinationCount );
    }

    std::vector< ShaderSampler > &getSamplers() { return _samplers; }
    std::vector< ShaderUniform > &getUniforms() { return _uniforms; }
    std::vector< ShaderBuffer > &getBuffers() { return _buffers; }
    std::vector< ShaderFlag > &getFlags() { return _flags; }
    std::vector< ShaderContext > &getContexts() { return _contexts; }
    std::vector< ShaderBinaryData> &getCombinations() { return _binaryShaders; }

private:
    
    bool _initialized;
};

enum class SamplerData
{
    Incorrect_Type,
    Incorrect_Id,
    Incorrect_TexID,
    Incorrect_TexUnit,
    Incorrect_Address,
    Incorrect_Filter,
    Incorrect_Aniso,
    Incorrect_Usage,
    Correct
};

enum class UniformData
{
    Incorrect_NoUniforms,
    Incorrect_Type,
    Incorrect_Id,
    Incorrect_DefValue,
    Correct
};

enum class BufferData
{
    Incorrect_NoBuffers,
    Incorrect_Id,
    Correct
};

enum class FlagData
{
    Incorrect_NoFlags,
    Incorrect_Id,
    Correct
};

enum class ContextData
{
    Incorrect_NoContexts,
    Incorrect_Id,
    Incorrect_Applicability,
    Incorrect_ZWriteEnable,
    Incorrect_ZEnable,
    Incorrect_ZFunc,
    Incorrect_Blendmode,
    Incorrect_Cullmode,
    Incorrect_AlphaToCoverage,
    Incorrect_TestPatchVertices,
    Incorrect_Combinations,
    Correct
};

enum class CombinationData
{
    Incorrect_Context,
    Incorrect_CombinationCount,
    Incorrect_CombinationShaderType,
    Correct
};

static uint8_t *generateBinarySamplerData( SamplerData genType, int iteration )
{
    uint8_t *data = new uint8_t[ 1024 ];
    memset( data, 0, 1024 );
    
    switch( genType )
    {
        case SamplerData::Incorrect_Type:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            
            if ( iteration == 0 ) samplerData[ 0 ] = 0;
            if ( iteration == 1 ) samplerData[ 0 ] = 1; // change in next versions because sampler1D should be usable
            if ( iteration == 2 ) samplerData[ 0 ] = 4; 
            if ( iteration == 3 ) samplerData[ 0 ] = 7; 

            break;
        }
        case SamplerData::Incorrect_Id:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            
            if ( iteration == 0 ) samplerData[ 1 ] = 0;
            if ( iteration == 1 ) 
            {
                samplerData[ 1 ] = 257;
                samplerData[ 2 ] = 'a';
            }
            
            break;
        }
        case SamplerData::Incorrect_TexID:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            samplerData[ 1 ] = 4; // sampler id size
            samplerData[ 2 ] = 'a'; // sampler id
            samplerData[ 3 ] = 'b';
            
            if ( iteration == 0 ) samplerData[ 4 ] = 0; // sampler tex id size
            if ( iteration == 1 ) 
            {
                samplerData[ 4 ] = 257; // sampler tex id size
                samplerData[ 5 ] = 'a'; // sampler tex id
            }
            break;
        }
        case SamplerData::Incorrect_TexUnit:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            samplerData[ 1 ] = 4; // sampler id size
            samplerData[ 2 ] = 'a'; // sampler id
            samplerData[ 3 ] = 'b';
            samplerData[ 4 ] = 4; // sampler tex id size
            samplerData[ 5 ] = 'c'; // sampler tex id
            samplerData[ 6 ] = 'd';
            
            if ( iteration == 0 ) 
            {
                short *d = ( short * ) samplerData;
                d[ 7 ] = -1;
            }
            if ( iteration == 1 )
            {
                short *d = ( short * ) samplerData;
                d[ 7 ] = 16;
            }
            break;
        }
        case SamplerData::Incorrect_Address:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            samplerData[ 1 ] = 4; // sampler id size
            samplerData[ 2 ] = 'a'; // sampler id
            samplerData[ 3 ] = 'b';
            samplerData[ 4 ] = 4; // sampler tex id size
            samplerData[ 5 ] = 'c'; // sampler tex id
            samplerData[ 6 ] = 'd';
            samplerData[ 7 ] = 0; // sampler texunit
            samplerData[ 8 ] = 3; // sampler address
            
            break;
        }
        case SamplerData::Incorrect_Filter:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            samplerData[ 1 ] = 4; // sampler id size
            samplerData[ 2 ] = 'a'; // sampler id
            samplerData[ 3 ] = 'b';
            samplerData[ 4 ] = 4; // sampler tex id size
            samplerData[ 5 ] = 'c'; // sampler tex id
            samplerData[ 6 ] = 'd';
            samplerData[ 7 ] = 0; // sampler texunit
            samplerData[ 8 ] = 0; // sampler address
            samplerData[ 9 ] = 3; // sampler filter
            
            break;
        }
        case SamplerData::Incorrect_Aniso:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            samplerData[ 1 ] = 4; // sampler id size
            samplerData[ 2 ] = 'a'; // sampler id
            samplerData[ 3 ] = 'b';
            samplerData[ 4 ] = 4; // sampler tex id size
            samplerData[ 5 ] = 'c'; // sampler tex id
            samplerData[ 6 ] = 'd';
            samplerData[ 7 ] = 0; // sampler texunit
            samplerData[ 8 ] = 0; // sampler address
            samplerData[ 9 ] = 2; // sampler filter
            
            if ( iteration == 0 ) samplerData[ 10 ] = 0;
            if ( iteration == 1 ) samplerData[ 10 ] = 3;
            if ( iteration == 2 ) samplerData[ 10 ] = 5;
            if ( iteration == 3 ) samplerData[ 10 ] = 6;
            if ( iteration == 4 ) samplerData[ 10 ] = 7;
            if ( iteration == 5 ) samplerData[ 10 ] = 9;
            if ( iteration == 6 ) samplerData[ 10 ] = 10;
            if ( iteration == 7 ) samplerData[ 10 ] = 11;
            if ( iteration == 8 ) samplerData[ 10 ] = 12;
            if ( iteration == 9 ) samplerData[ 10 ] = 13;
            if ( iteration == 10 ) samplerData[ 10 ] = 14;
            if ( iteration == 11 ) samplerData[ 10 ] = 15;
            if ( iteration == 12 ) samplerData[ 10 ] = 17;

            break;
        }
        case SamplerData::Incorrect_Usage:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            samplerData[ 1 ] = 4; // sampler id size
            samplerData[ 2 ] = 'a'; // sampler id
            samplerData[ 3 ] = 'b';
            samplerData[ 4 ] = 4; // sampler tex id size
            samplerData[ 5 ] = 'c'; // sampler tex id
            samplerData[ 6 ] = 'd';
            samplerData[ 7 ] = 0; // sampler texunit
            samplerData[ 8 ] = 0; // sampler address
            samplerData[ 9 ] = 2; // sampler filter
            samplerData[ 10 ] = 1; // sampler anisotropy
            
            if ( iteration == 0 ) samplerData[ 11 ] = 4;
            if ( iteration == 1 ) 
            {
                samplerData[ 0 ] = 3; // sampler type
                samplerData[ 11 ] = 3; // sampler usage
            }
            if ( iteration == 2 )
            {
                samplerData[ 11 ] = 2; // samplerUsage
            }
            break;
        }
        case SamplerData::Correct:
        {
            uint16_t *samplerData = ( uint16_t * ) data;
            samplerData[ 0 ] = 2; // sampler2D
            samplerData[ 1 ] = 4; // sampler id size
            samplerData[ 2 ] = 'a'; // sampler id
            samplerData[ 3 ] = 'b';
            samplerData[ 4 ] = 4; // sampler tex id size
            samplerData[ 5 ] = 'c'; // sampler tex id
            samplerData[ 6 ] = 'd';
            samplerData[ 7 ] = 0; // sampler texunit
            samplerData[ 8 ] = 0; // sampler address
            samplerData[ 9 ] = 2; // sampler filter
            samplerData[ 10 ] = 8; // sampler anisotropy
            samplerData[ 11 ] = 0; // sampler usage
            break;
        }
        default:
            break;
    }
    
    return data;
}

static uint8_t *generateBinaryUniformData( UniformData genType, int iteration )
{
    uint8_t *data = new uint8_t[ 1024 ];
    memset( data, 0, 1024 );
    
    switch( genType )
    {
        case UniformData::Incorrect_NoUniforms:
        {
            uint16_t *uniData = ( uint16_t * ) data;
            
            if ( iteration == 0 ) uniData[ 0 ] = 0;

            break;
        }
        case UniformData::Incorrect_Type:
        {
            uint16_t *uniData = ( uint16_t * ) data;

            if ( iteration == 0 ) uniData[ 0 ] = 3;
            if ( iteration == 1 ) uniData[ 0 ] = 10;
            if ( iteration == 2 ) uniData[ 0 ] = 2;
            if ( iteration == 3 ) uniData[ 0 ] = 3;

            break;
        }
        case UniformData::Incorrect_Id:
        {
            uint16_t *uniData = ( uint16_t * ) data;
            uniData[ 0 ] = 0; // float
            
            if ( iteration == 0 ) uniData[ 1 ] = 0;
            if ( iteration == 1 ) 
            {
                uniData[ 1 ] = 257;
                uniData[ 2 ] = 'a';
            }
            if ( iteration == 2 )
            {
                uniData[ 1 ] = 15;
                uniData[ 2 ] = '\0';
            }

            break;
        }
        case UniformData::Incorrect_DefValue:
        {
            uint16_t *uniData = ( uint16_t * ) data;
            uniData[ 0 ] = 0; // float
            uniData[ 1 ] = 2; // id size
            uniData[ 2 ] = 'a'; // id

            if ( iteration == 0 ) uniData[ 3 ] = 2;
            if ( iteration == 1 ) uniData[ 3 ] = 4;
            if ( iteration == 2 )
            {
                uniData[ 0 ] = 1; // float4
                uniData[ 3 ] = 3;
            }
            if ( iteration == 2 )
            {
                uniData[ 0 ] = 1; // float4
                uniData[ 3 ] = 1;
            }

            break;
        }
        case UniformData::Correct:
        {
            if ( iteration == 0 )
            {
                uint16_t *uniData = ( uint16_t * ) data;
                uniData[ 0 ] = 0; // float
                uniData[ 1 ] = 2; // id size
                uniData[ 2 ] = 'a'; // id
                uniData[ 3 ] = 1;

                float *defValueData = ( float * )( data + sizeof( uint16_t ) * 4);
                defValueData[ 0 ] = 15.5f;
            }
            if ( iteration == 1 )
            {
                uint16_t *uniData = ( uint16_t * ) data;
                uniData[ 0 ] = 1; // float4
                uniData[ 1 ] = 2; // id size
                uniData[ 2 ] = 'a'; // id
                uniData[ 3 ] = 4;

                float *defValueData = ( float * )( data + sizeof( uint16_t ) * 4);
                defValueData[ 0 ] = 15.5f;
                defValueData[ 1 ] = 25.5f;
                defValueData[ 2 ] = 35.5f;
                defValueData[ 3 ] = 45.5f;
            }
            if ( iteration == 2 )
            {
                uint16_t *uniData = ( uint16_t * ) data;
                uniData[ 0 ] = 1; // float4
                uniData[ 1 ] = 2; // id size
                uniData[ 2 ] = 'a'; // id
                uniData[ 3 ] = 4;

                float *defValueData = ( float * )( data + sizeof( uint16_t ) * 4);
                defValueData[ 0 ] = 15.5f;
                defValueData[ 1 ] = 25.5f;
                defValueData[ 2 ] = 35.5f;
                defValueData[ 3 ] = 45.5f;

                // skip 16 bytes, 8 uint16_t
                uniData[ 12 ] = 1; // float4
                uniData[ 13 ] = 2; // id size
                uniData[ 14 ] = 'a'; // id
                uniData[ 15 ] = 4;

                defValueData[ 6 ] = 15.5f;
                defValueData[ 7 ] = 25.5f;
                defValueData[ 8 ] = 35.5f;
                defValueData[ 9 ] = 45.5f;
            }

            break;
        }
        default:
            break;
    }
    
    return data;
}

static uint8_t *generateBinaryBufferData( BufferData genType, int iteration )
{
    uint8_t *data = new uint8_t[ 1024 ];
    memset( data, 0, 1024 );

    switch( genType )
    {
        case BufferData::Incorrect_NoBuffers:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            if ( iteration == 0 ) bufData[ 0 ] = 0;

            break;
        }
        case BufferData::Incorrect_Id:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            if ( iteration == 0 ) bufData[ 1 ] = 0;
            if ( iteration == 1 )
            {
                bufData[ 0 ] = 257;
                bufData[ 1 ] = 'a';
            }
            if ( iteration == 2 )
            {
                bufData[ 0 ] = 15;
                bufData[ 1 ] = '\0';
            }

            break;
        }
        case BufferData::Correct:
        {
            if ( iteration == 0 )
            {
                uint16_t *bufData = ( uint16_t * ) data;
                bufData[ 0 ] = 2; // id size
                bufData[ 1 ] = 'a'; // id
            }
            if ( iteration == 1 )
            {
                uint16_t *bufData = ( uint16_t * ) data;
                bufData[ 0 ] = 2; // id size
                bufData[ 1 ] = 'a'; // id
                bufData[ 2 ] = 2; // id size
                bufData[ 3 ] = 'b'; // id
            }

            break;
        }
        default:
            break;
    }

    return data;
}

static uint8_t *generateBinaryFlagData( FlagData genType, int iteration )
{
    uint8_t *data = new uint8_t[ 1024 ];
    memset( data, 0, 1024 );

    switch ( genType )
    {
        case FlagData::Incorrect_NoFlags:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            if ( iteration == 0 ) bufData[ 0 ] = 0;

            break;
        }
        case FlagData::Incorrect_Id:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            if ( iteration == 0 ) bufData[ 1 ] = 0;
            if ( iteration == 1 )
            {
                bufData[ 0 ] = 257;
                bufData[ 1 ] = 'a';
            }
            if ( iteration == 2 )
            {
                bufData[ 0 ] = 15;
                bufData[ 1 ] = '\0';
            }

            break;
        }
        case FlagData::Correct:
        {
            if ( iteration == 0 )
            {
                uint16_t *bufData = ( uint16_t * ) data;
                bufData[ 0 ] = 2; // id size
                bufData[ 1 ] = 'a'; // id
            }
            if ( iteration == 1 )
            {
                uint16_t *bufData = ( uint16_t * ) data;
                bufData[ 0 ] = 2; // id size
                bufData[ 1 ] = 'a'; // id
                bufData[ 2 ] = 2; // id size
                bufData[ 3 ] = 'b'; // id
            }

            break;
        }
        default:
            break;
    }

    return data;
}

static uint8_t *generateBinaryContextData( ContextData genType, int iteration )
{
    uint8_t *data = new uint8_t[ 1024 ];
    memset( data, 0, 1024 );

    switch ( genType )
    {
        case ContextData::Incorrect_NoContexts:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            if ( iteration == 0 ) bufData[ 0 ] = 0;

            break;
        }
        case ContextData::Incorrect_Id:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            if ( iteration == 0 ) bufData[ 1 ] = 0;
            if ( iteration == 1 )
            {
                bufData[ 0 ] = 257;
                bufData[ 1 ] = 'a';
            }
            if ( iteration == 2 )
            {
                bufData[ 0 ] = 15;
                bufData[ 1 ] = '\0';
            }

            break;
        }
        case ContextData::Incorrect_Applicability:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id

            if ( iteration == 0 ) bufData[ 2 ] = 1;
            if ( iteration == 1 ) bufData[ 2 ] = 3;
            if ( iteration == 2 ) bufData[ 2 ] = 5;
            if ( iteration == 3 ) bufData[ 2 ] = 6;
            if ( iteration == 4 ) bufData[ 2 ] = 10;

            break;
        }
        case ContextData::Incorrect_ZWriteEnable:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl

            if ( iteration == 0 ) bufData[ 3 ] = 2;
            break;
        }
        case ContextData::Incorrect_ZEnable:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable

            if ( iteration == 0 ) bufData[ 4 ] = 2;
            break;
        }
        case ContextData::Incorrect_ZFunc:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable
            bufData[ 4 ] = 1; // zenable

            if ( iteration == 0 ) bufData[ 5 ] = 6;
            break;
        }
        case ContextData::Incorrect_Blendmode:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable
            bufData[ 4 ] = 1; // zenable
            bufData[ 5 ] = 5; // zfunc

            if ( iteration == 0 )
            {
                bufData[ 6 ] = 5;
                bufData[ 7 ] = 0;
            }
            if ( iteration == 1 )
            {
                bufData[ 6 ] = 10;
                bufData[ 7 ] = 0;
            }
            if ( iteration == 2 )
            {
                bufData[ 6 ] = 0;
                bufData[ 7 ] = 10;
            }
            if ( iteration == 3 )
            {
                bufData[ 6 ] = 10;
                bufData[ 7 ] = 1;
            }
            if ( iteration == 4 )
            {
                bufData[ 6 ] = 20;
                bufData[ 7 ] = 20;
            }

            break;
        }
        case ContextData::Incorrect_Cullmode:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable
            bufData[ 4 ] = 1; // zenable
            bufData[ 5 ] = 5; // zfunc
            bufData[ 6 ] = 10;
            bufData[ 7 ] = 10;

            if ( iteration == 0 ) bufData[ 8 ] = 3;
            break;
        }
        case ContextData::Incorrect_AlphaToCoverage:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable
            bufData[ 4 ] = 1; // zenable
            bufData[ 5 ] = 5; // zfunc
            bufData[ 6 ] = 10;
            bufData[ 7 ] = 10;
            bufData[ 8 ] = 2;

            if ( iteration == 0 ) bufData[ 9 ] = 2;
            break;
        }
        case ContextData::Incorrect_TestPatchVertices:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable
            bufData[ 4 ] = 1; // zenable
            bufData[ 5 ] = 5; // zfunc
            bufData[ 6 ] = 10;
            bufData[ 7 ] = 10;
            bufData[ 8 ] = 2;
            bufData[ 9 ] = 1;

            if ( iteration == 0 ) bufData[ 10 ] = 0;
            if ( iteration == 1 ) bufData[ 10 ] = 33;

            break;
        }
        case ContextData::Incorrect_Combinations:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable
            bufData[ 4 ] = 1; // zenable
            bufData[ 5 ] = 5; // zfunc
            bufData[ 6 ] = 10; // blendmode 1
            bufData[ 7 ] = 10; // blendmode 2
            bufData[ 8 ] = 2; // cull mode
            bufData[ 9 ] = 1; // alpha_to_coverage
            bufData[ 10 ] = 1; // tess patch vertices
            bufData[ 11 ] = 1; // flag mask
            bufData[ 12 ] = 1; // flag mask

            if ( iteration == 0 ) bufData[ 13 ] = 256;

            break;
        }
        case ContextData::Correct:
        {
           uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 2;
            bufData[ 1 ] = 'a'; // id
            bufData[ 2 ] = 4; // opengl
            bufData[ 3 ] = 1; // zwriteenable
            bufData[ 4 ] = 1; // zenable
            bufData[ 5 ] = 5; // zfunc
            bufData[ 6 ] = 10; // blendmode 1
            bufData[ 7 ] = 10; // blendmode 2
            bufData[ 8 ] = 2; // cull mode
            bufData[ 9 ] = 1; // alpha_to_coverage
            bufData[ 10 ] = 1; // tess patch vertices
            bufData[ 11 ] = 1; // flag mask
            bufData[ 12 ] = 1; // flag mask
            bufData[ 13 ] = 1; // combinations
            break;
        }
        default:
            break;
    }

    return data;
}

static uint8_t *generateBinaryCombinationData( CombinationData genType, int iteration )
{
    uint8_t *data = new uint8_t[ 1024 ];
    memset( data, 0, 1024 );

    switch ( genType )
    {
        case CombinationData::Incorrect_Context:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            if ( iteration == 0 ) bufData[ 0 ] = 1;

            break;
        }
        case CombinationData::Incorrect_CombinationCount:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 0;
            bufData[ 1 ] = 0; // mask
            bufData[ 2 ] = 0; // mask

            if ( iteration == 0 ) bufData[ 3 ] = 0;

            break;
        }
        case CombinationData::Incorrect_CombinationShaderType:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 0;
            bufData[ 1 ] = 0;
            bufData[ 2 ] = 0;
            bufData[ 3 ] = 1;

            if ( iteration == 0 ) bufData[ 4 ] = 7;

            break;
        }
        case CombinationData::Correct:
        {
            uint16_t *bufData = ( uint16_t * ) data;

            bufData[ 0 ] = 0;
            bufData[ 1 ] = 0;
            bufData[ 2 ] = 0;
            bufData[ 3 ] = 1;
            bufData[ 4 ] = 0;
            bufData[ 5 ] = 4; // size
            bufData[ 6 ] = 0; // size
            bufData[ 7 ] = 10; // format
            bufData[ 8 ] = 0; // format
            bufData[ 9 ] = 50; // data
            bufData[ 10 ] = 50; // data

            break;
        }
        default:
            break;
    }

    return data;
}

TEST_CASE( "create shader parser", "[unit-shader]" )
{
    ShaderParser p( "test_shader" );
    
    REQUIRE( p.getShaderName() == "test_shader" );
}

TEST_CASE( "parse binary sampler", "[unit-shader]" )
{
    TestShaderParser p( "test_shader" );
    
    std::unique_ptr< uint8_t > samplerData = nullptr;
    
    SECTION( "no data" )
    {
        REQUIRE_FALSE( p.test_parseBinarySampler( nullptr, 0 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Type, 0 ) );
        
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 0 ) );
    }
    
    SECTION( "incorrect sampler type" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Type, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Type, 1 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Type, 2 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Type, 3 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
    }
    
    SECTION( "incorrect sampler id" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Id, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Id, 1 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
    }
    
    SECTION( "incorrect sampler texture id" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_TexID, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_TexID, 1 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
    }
    
    SECTION( "incorrect sampler texunit" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_TexUnit, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );        

        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_TexUnit, 1 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) ); 
    }
    
    SECTION( "incorrect sampler address" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Address, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
    }

    SECTION( "incorrect sampler filter" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Filter, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
    }
    
    SECTION( "incorrect sampler anisotropy" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );        

        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 1 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 2 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 3 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 4 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 5 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 6 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 7 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 8 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 9 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 10 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 11 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Aniso, 12 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
    }
    
    SECTION( "incorrect sampler usage" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Usage, 0 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Usage, 1 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
        // modify render device caps to handle case when computer shaders are not available
        RDI_Null::RenderDeviceNull *rdi = ( RDI_Null::RenderDeviceNull * ) Modules::renderer().getRenderDevice();
        DeviceCaps caps = rdi->getCaps();
        caps.computeShaders = false;
        rdi->setCaps( caps );
            
        samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_Usage, 2 ) );
        REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
        
    }
    
    SECTION( "correct usage" )
    {
        samplerData.reset( generateBinarySamplerData( SamplerData::Correct, 0 ) );
        REQUIRE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) == true );        

        REQUIRE( p.getSamplers().size() == 1 );
    }
}

TEST_CASE( "parse binary uniform", "[unit-shader]" )
{
    TestShaderParser p( "test_shader" );

    std::unique_ptr< uint8_t > uniformData = nullptr;

    SECTION( "no data" )
    {
        REQUIRE_FALSE( p.test_parseBinaryUniform( nullptr, 0 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_NoUniforms, 0 ) );

        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 0 ) );
    }

    SECTION( "incorrect type" )
    {
        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_Type, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_Type, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_Type, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_Type, 3 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );
    }

    SECTION( "incorrect id" )
    {
        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_Id, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_Id, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_Id, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );
    }

    SECTION( "incorrect def value" )
    {
        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_DefValue, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_DefValue, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_DefValue, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );

        uniformData.reset( generateBinaryUniformData( UniformData::Incorrect_DefValue, 3 ) );
        REQUIRE_FALSE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) );
    }

    SECTION( "correct usage" )
    {
        uniformData.reset( generateBinaryUniformData( UniformData::Correct, 0 ) );
        REQUIRE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) == true );

        REQUIRE( p.getUniforms().size() == 1 );

        uniformData.reset( generateBinaryUniformData( UniformData::Correct, 1 ) );
        REQUIRE( p.test_parseBinaryUniform( (char *) uniformData.get(), 1 ) == true );

        REQUIRE( p.getUniforms().size() == 2 ); // we are not cleaning previous addition

        uniformData.reset( generateBinaryUniformData( UniformData::Correct, 2 ) );
        REQUIRE( p.test_parseBinaryUniform( (char *) uniformData.get(), 2 ) == true );

        REQUIRE( p.getUniforms().size() == 4 ); // we are not cleaning previous addition

    }
}

TEST_CASE( "parse binary buffer", "[unit-shader]" )
{
    TestShaderParser p( "test_shader" );

    std::unique_ptr< uint8_t > bufferData = nullptr;

    SECTION( "no data" )
    {
        REQUIRE_FALSE( p.test_parseBinaryBuffer( nullptr, 0 ) );

        bufferData.reset( generateBinaryBufferData( BufferData::Incorrect_NoBuffers, 0 ) );

        REQUIRE_FALSE( p.test_parseBinaryBuffer( (char *) bufferData.get(), 0 ) );
    }

    SECTION( "incorrect id" )
    {
        bufferData.reset( generateBinaryBufferData( BufferData::Incorrect_Id, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryBuffer( (char *) bufferData.get(), 1 ) );

        bufferData.reset( generateBinaryBufferData( BufferData::Incorrect_Id, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryBuffer( (char *) bufferData.get(), 1 ) );

        bufferData.reset( generateBinaryBufferData( BufferData::Incorrect_Id, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryBuffer( (char *) bufferData.get(), 1 ) );
    }

    SECTION( "correct" )
    {
        bufferData.reset( generateBinaryBufferData( BufferData::Correct, 0 ) );
        REQUIRE( p.test_parseBinaryBuffer( (char *) bufferData.get(), 1 ) == true );

        REQUIRE( p.getBuffers().size() == 1 );

        bufferData.reset( generateBinaryBufferData( BufferData::Correct, 1 ) );
        REQUIRE( p.test_parseBinaryBuffer( (char *) bufferData.get(), 2 ) == true );

        REQUIRE( p.getBuffers().size() == 3 ); // we are not cleaning previous addition
    }
}

TEST_CASE( "parse binary flags", "[unit-shader]" )
{
    TestShaderParser p( "test_shader" );

    std::unique_ptr< uint8_t > flagData = nullptr;

    SECTION( "incorrect id" )
    {
        flagData.reset( generateBinaryFlagData( FlagData::Incorrect_Id, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryFlags( (char *) flagData.get(), 1 ) );

        flagData.reset( generateBinaryFlagData( FlagData::Incorrect_Id, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryFlags( (char *) flagData.get(), 1 ) );

        flagData.reset( generateBinaryFlagData( FlagData::Incorrect_Id, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryFlags( (char *) flagData.get(), 1 ) );
    }

    SECTION( "correct" )
    {
        flagData.reset( generateBinaryFlagData( FlagData::Correct, 0 ) );
        REQUIRE( p.test_parseBinaryFlags( (char *) flagData.get(), 1 ) == true );

        REQUIRE( p.getFlags().size() == 1 );

        flagData.reset( generateBinaryFlagData( FlagData::Correct, 1 ) );
        REQUIRE( p.test_parseBinaryFlags( (char *) flagData.get(), 2 ) == true );

        REQUIRE( p.getFlags().size() == 3 ); // we are not cleaning previous addition
    }
}

TEST_CASE( "parse binary context", "[unit-shader]" )
{
    TestShaderParser p( "test_shader" );

    std::unique_ptr< uint8_t > contextData = nullptr;

    SECTION( "no data" )
    {
        REQUIRE_FALSE( p.test_parseBinaryContext( nullptr, 0 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_NoContexts, 0 ) );

        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 0 ) );
    }
    SECTION( "incorrect id" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Id, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Id, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Id, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect applicability" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Applicability, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Applicability, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Applicability, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Applicability, 3 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Applicability, 4 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect zwriteenable" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_ZWriteEnable, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect zenable" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_ZEnable, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect zfunc" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_ZFunc, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect blendmode" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Blendmode, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Blendmode, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Blendmode, 2 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Blendmode, 3 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Blendmode, 4 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect cullmode" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Cullmode, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect alpha_to_coverage" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_AlphaToCoverage, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect tess patch vertices" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_TestPatchVertices, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );

        contextData.reset( generateBinaryContextData( ContextData::Incorrect_TestPatchVertices, 1 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "incorrect combination count" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Incorrect_Combinations, 0 ) );
        REQUIRE_FALSE( p.test_parseBinaryContext( (char *) contextData.get(), 1 ) );
    }
    SECTION( "correct" )
    {
        contextData.reset( generateBinaryContextData( ContextData::Correct, 0 ) );
        REQUIRE( p.test_parseBinaryContext( (char *) contextData.get(), 1 )  == true );

        REQUIRE( p.getContexts().size() == 1 );
    }
}

TEST_CASE( "parse shader binary combination", "[unit-shader]" )
{
    TestShaderParser p( "test_shader" );

    std::unique_ptr< uint8_t > combData = nullptr;

    SECTION( "incorrect combination context" )
    {
        combData.reset( generateBinaryCombinationData( CombinationData::Incorrect_Context, 0 ) );
        p.getContexts().push_back( ShaderContext() );

        REQUIRE_FALSE( p.test_parseBinaryCombination( (char *) combData.get(), 1 ) );
    }
    SECTION( "incorrect combination count" )
    {
        combData.reset( generateBinaryCombinationData( CombinationData::Incorrect_CombinationCount, 0 ) );

        p.test_parseBinaryContext( (char *) generateBinaryContextData( ContextData::Correct, 0 ), 1 );

        REQUIRE_FALSE( p.test_parseBinaryCombination( (char *) combData.get(), 1 ) );
    }
    SECTION( "incorrect combination type" )
    {
        combData.reset( generateBinaryCombinationData( CombinationData::Incorrect_CombinationShaderType, 0 ) );

        p.test_parseBinaryContext( (char *) generateBinaryContextData( ContextData::Correct, 0 ), 1 );

        REQUIRE_FALSE( p.test_parseBinaryCombination( (char *) combData.get(), 1 ) );
    }
    SECTION( "incorrect combination type" )
    {
        combData.reset( generateBinaryCombinationData( CombinationData::Correct, 0 ) );

        p.test_parseBinaryContext( (char *) generateBinaryContextData( ContextData::Correct, 0 ), 1 );

        REQUIRE( p.test_parseBinaryCombination( (char *) combData.get(), 1 ) == true );

        auto combs = p.getCombinations();
        REQUIRE( combs[ 0 ].combinationId == 0 );
        REQUIRE( combs[ 0 ].combinationShadersLeft == 0 );
        REQUIRE( combs[ 0 ].contextId == 0 );
        REQUIRE( combs[ 0 ].dataFormat == 10 );
        REQUIRE( combs[ 0 ].dataSize == 4 );
        REQUIRE( combs[ 0 ].shaderType == 0 );

        uint16 data[ 2 ];
        memcpy( data, combs[ 0 ].data, 4 );

        REQUIRE( data[ 0 ] == 50 );
        REQUIRE( data[ 1 ] == 50 );
    }
}
