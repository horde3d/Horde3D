#include "../catch.hpp"
#include <memory>
#include <iostream>

#include "../../Source/Horde3DEngine/egShaderParser.h"
#include "../../Source/Horde3DEngine/egModules.h"
#include "../../Source/Horde3DEngine/egRenderer.h"

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
    
private:
    
    bool _initialized;
};

enum class SamplerData
{
    Incorrect_Type,
    Incorrect_Id,
    Incorrect_TexID,
    Correct
};

uint8_t *generateBinarySamplerData( SamplerData genType, int iteration )
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
            
            if ( iteration == 0 ) samplerData[ 4 ] = 0;
            if ( iteration == 1 ) 
            {
                samplerData[ 4 ] = 257;
                samplerData[ 5 ] = 'a';
            }
            break;
        }
        case SamplerData::Correct:
        {
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
    
//     SECTION( "incorrect sampler texunit" )
//     {
//         samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_TexID, 0 ) );
//         REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
//         
//         samplerData.reset( generateBinarySamplerData( SamplerData::Incorrect_TexID, 1 ) );
//         REQUIRE_FALSE( p.test_parseBinarySampler( (char *) samplerData.get(), 1 ) );
//     }
}
