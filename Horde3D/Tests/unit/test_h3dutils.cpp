#include "../catch.hpp"
#include <memory>
#include <iostream>
#include <vector>

#include "Horde3D.h"
#include "Horde3DUtils.h"

#define NULL_RENDER_BACKEND 256

class Tester
{
public:

    Tester()
    {
        if ( !h3dInit( (H3DRenderDevice::List) NULL_RENDER_BACKEND ) )
        {
            std::cout << "Failed to initialize engine!" << std::endl;
        }
    }

    ~Tester()
    {
        h3dRelease();
    }
};

TEST_CASE( "generate binary shader from existing shader", "[unit-utils]" )
{
    Tester t;

    H3DRes shader = h3dAddResource( H3DResTypes::Shader, "shaders/overlay.shader", 0 );
    REQUIRE( h3dutLoadResourcesFromDisk( "../../Content/" ) == true );

    REQUIRE( h3dutCreateBinaryShader( shader, "test.h3dsb" ) == true );
}
