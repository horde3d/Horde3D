#include "../catch.hpp"
#include <memory>

#include "../../Source/Horde3DEngine/egShaderParser.h"

using namespace Horde3D;

class TestShaderParser : public ShaderParser
{
public:
    
    TestShaderParser( const std::string shaderName ) : ShaderParser( shaderName )
    {
        
    }
    
    bool test_parseBinarySampler( char *data, uint32_t samplerCount )
    {
        return parseBinarySampler( data, samplerCount );
    }
    
    bool test_parseBinaryUniform( char *data, uint32_t variablesCount )
    {
        return parseBinaryUniforms( data, variablesCount );
    }
};

enum class SamplerData
{
    Incorrect_Type,
    Correct
};

uint8_t *generateBinarySamplerData( SamplerData genType, int iteration )
{
    uint8_t *data = new uint8_t[ 1024 ];
    
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
}
