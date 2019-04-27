include_directories(../Bindings/C++)
include_directories(../../Extensions/Overlays/Bindings/C++)

add_subdirectory(Framework)
add_subdirectory(Chicago)
add_subdirectory(Knight)
add_subdirectory(ParticleVortex)
add_subdirectory(Tessellator)

IF(HORDE3D_USE_GLFW)
    include_directories(${GLFW_INCLUDE_DIR})
ENDIF(HORDE3D_USE_GLFW)

IF (HORDE3D_USE_SDL)
    include_directories(${SDL2_INCLUDE_DIR})
ENDIF(HORDE3D_USE_SDL)