#include "../catch.hpp"
#include <memory>
#include <iostream>
#include <cstdio>
#include <vector>

#include "../../Source/Horde3DEngine/egShader.h"
#include "../../Source/Horde3DEngine/egModules.h"
#include "../../Source/Horde3DEngine/egRenderer.h"
#include "../../Source/Horde3DEngine/egRendererBaseNull.h"

using namespace Horde3D;

#define NULL_RENDER_BACKEND 256

class Tester
{
public:

    Tester()
    {
        // Init engine
        _initialized = Modules::init( NULL_RENDER_BACKEND );
        if ( !_initialized )
        {
            std::cout << "---Engine failed to initialize. Test will likely fail/crash!---" << std::endl;

            return;
        }
    }

    ~Tester()
    {
        if ( _initialized )
        {
            Modules::release();
        }
    }

private:

    bool _initialized;
};

class TestShaderResource : public ShaderResource
{
public:

    TestShaderResource( const std::string shaderName, int flags ) : ShaderResource( shaderName, flags )
    {

    }

    ~TestShaderResource()
    {

    }

    bool test_load( const char *data, int size )
    {
        return load( data, size );
    }

    bool test_generateBinaryShader( uint8_t *&data, uint32_t &dataSize )
    {
        return createBinaryShaderStream( data, dataSize );
    }

    void addDummyContext()
    {
        ShaderContext ctx;
        ctx.id = "dummy";
        ctx.alphaToCoverage = true;
        ctx.blendingEnabled = true;
        ctx.blendStateSrc = BlendModes::One;
        ctx.blendStateDst = BlendModes::OneMinusSrcAlpha;
        ctx.cullMode = CullModes::Back;
        ctx.depthTest = true;
        ctx.depthFunc = TestModes::Greater;
        ctx.flagMask = 0;
        ctx.writeDepth = false;
        ctx.renderInterfaces = RenderBackendType::OpenGL4;

        _contexts.push_back( ctx );
    }

    void addDummyShaderComb()
    {
        ShaderCombination comb;
        Modules::renderer().createShaderComb( comb, "dummy", "dummy", nullptr, nullptr, nullptr, nullptr );

        _contexts[ 0 ].shaderCombs.push_back( comb );
    }

    bool checkBinaryShader() { return _binaryShader; }

    bool checkContexts() { return !_contexts.empty(); }

    bool checkSamplers() { return !_samplers.empty(); }

    bool checkUniforms() { return !_uniforms.empty(); }

    bool checkBuffers() { return !_buffers.empty(); }

    bool checkBinarySections() { return !_binarySections.empty(); }

    auto &getSamplers() { return _samplers; }

    auto &getContexts() { return _contexts; }

    auto &getUniforms() { return _uniforms; }

    auto &getBinarySections() { return _binarySections; }

private:

};

enum class ShaderGenerator
{
    HeaderOnly,
    CorrectBinaryShader
};

static uint8 *generateBinaryShader( ShaderGenerator gen, uint32 &size )
{
    uint8_t *data = new uint8_t[ 8192 ];
    memset( data, 0, 8192 );

    switch( gen )
    {
        case ShaderGenerator::HeaderOnly:
        {
            data[ 0 ] = 'H';
            data[ 1 ] = '3';
            data[ 2 ] = 'D';
            data[ 3 ] = 'S';
            data[ 4 ] = 'B';

            size = 5;
            break;
        }
        case ShaderGenerator::CorrectBinaryShader:
        {
            TestShaderResource tr( "model", 0 );

            FILE *f = fopen( "../../Content/shaders/model.shader", "rb" );
            if ( !f ) return nullptr;

            fseek( f, 0L, SEEK_END );
            auto fsize = ftell( f );
            fseek( f, 0L, SEEK_SET );

            uint8 *tmp = new uint8[ fsize + 1 ];
            auto read = fread( tmp, sizeof( char ), fsize, f );
            fclose( f );

            if ( !tr.load( (const char *) tmp, fsize ) ) return nullptr;

            // shader does not have contexts as we are using null render backend and not opengl one
            // contexts are parsed relative to backend
            // create contexts with dummy data
            tr.addDummyContext();
            tr.addDummyShaderComb();

            uint32 binSize = 8192;
            if ( !tr.test_generateBinaryShader( data, binSize ) ) return nullptr;

            size = binSize;

            break;
        }
    }

    return data;
}

TEST_CASE( "load binary shader", "[unit-shader]" )
{
    Tester t; // init and deinit engine, should always be first and last to destroy

    TestShaderResource sr( "test", 0 );
    std::unique_ptr< uint8_t > shData = nullptr;

    SECTION( "check header" )
    {
        uint32 dataSize = 0;
        shData.reset( generateBinaryShader( ShaderGenerator::HeaderOnly, dataSize ) );

        REQUIRE( sr.test_load( (const char *) shData.get(), dataSize ) == false );

        REQUIRE( sr.checkBinaryShader() == true );
    }

    SECTION( "check load successful" )
    {
        uint32 dataSize = 0;
        shData.reset( generateBinaryShader( ShaderGenerator::CorrectBinaryShader, dataSize ) );

        REQUIRE( sr.test_load( (const char *) shData.get(), dataSize ) == true );
        REQUIRE( sr.checkContexts() == true );
        REQUIRE( sr.checkSamplers() == true );
//        REQUIRE( sr.checkBuffers() == true );
        REQUIRE( sr.checkUniforms() == true );
        REQUIRE( sr.checkBinarySections() == true );

        // check additional details
        auto &contexts = sr.getContexts();
        REQUIRE( contexts.size() == 1 );
        REQUIRE( contexts[ 0 ].alphaToCoverage == true );
        REQUIRE( contexts[ 0 ].id == "dummy" );
        REQUIRE( contexts[ 0 ].blendingEnabled == true );
        REQUIRE( contexts[ 0 ].blendStateSrc == BlendModes::One );
        REQUIRE( contexts[ 0 ].blendStateDst == BlendModes::OneMinusSrcAlpha );
        REQUIRE( contexts[ 0 ].cullMode == CullModes::Back );
        REQUIRE( contexts[ 0 ].depthTest == true );
        REQUIRE( contexts[ 0 ].depthFunc == TestModes::Greater );
        REQUIRE( contexts[ 0 ].writeDepth == false );
//         REQUIRE( contexts[ 0 ].renderInterfaces == RenderBackendType::OpenGL4 );

        auto &unis = sr.getUniforms();
        REQUIRE( unis.size() == 2 );
        REQUIRE( unis[ 0 ].size == 4 );
        REQUIRE( unis[ 1 ].size == 4 );

        auto &samplers = sr.getSamplers();
        REQUIRE( samplers.size() == 4 );
        REQUIRE( samplers[ 2 ].type == Horde3D::TextureTypes::TexCube );
        REQUIRE( samplers[ 3 ].type == Horde3D::TextureTypes::TexCube );

        auto &bin = sr.getBinarySections();
        REQUIRE( bin.size() == 1 );
        REQUIRE( bin[ 0 ].shaderType == ShaderType::Program );
        REQUIRE( bin[ 0 ].dataFormat == 1000 );
        REQUIRE( bin[ 0 ].dataSize == 8 );

        unsigned char data[] = "deadbeef";
        REQUIRE( memcmp( bin[ 0 ].data, data, 8 ) == 0 );
    }
}

