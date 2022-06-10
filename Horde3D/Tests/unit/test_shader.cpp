#include "../catch.hpp"
#include <memory>
#include <iostream>
#include <vector>

#include "../../Source/Horde3DEngine/egShader.h"
#include "../../Source/Horde3DEngine/egModules.h"
#include "../../Source/Horde3DEngine/egRenderer.h"
#include "../../Source/Horde3DEngine/egRendererBaseNull.h"


using namespace Horde3D;

#define NULL_RENDER_BACKEND 256

class TestShaderResource : public ShaderResource
{
public:

    TestShaderResource( const std::string shaderName, int flags ) : ShaderResource( shaderName, flags )
    {
        // Init engine
        _initialized = Modules::init( NULL_RENDER_BACKEND );
        if ( !_initialized )
            std::cout << "---Engine failed to initialize. Test will likely fail/crash!---" << std::endl;
    }

    ~TestShaderResource()
    {
        if ( _initialized )
            Modules::release();
    }

    bool test_load( const char *data, int size )
    {
        return load( data, size );
    }

private:

    bool _initialized;
};

TEST_CASE( "load binary shader", "[unit-shader]" )
{

}
