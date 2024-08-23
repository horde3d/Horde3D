include_directories(../Bindings/C++)
include_directories(../../Extensions/Overlays/Bindings/C++)

add_subdirectory(Framework)
add_subdirectory(Chicago)
add_subdirectory(Knight)

if( NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten" )
    add_subdirectory(ParticleVortex) # Compute shaders are not supported on Emscripten, which is what this example uses
    add_subdirectory(Tessellator)    # requires OpenGLES 3.2, but WebGL only supports OpenGLES 3.0
endif()

IF(HORDE3D_USE_GLFW)
    include_directories(${GLFW_INCLUDE_DIR})
ENDIF(HORDE3D_USE_GLFW)

IF (HORDE3D_USE_SDL)
    include_directories(${SDL2_INCLUDE_DIR})
ENDIF(HORDE3D_USE_SDL)