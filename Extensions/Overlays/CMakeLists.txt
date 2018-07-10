# Overlays extension allows displaying various information, like text, logos and performance info above the rendered image     
option(HORDE3D_BUILD_OVERLAYS "Build the Overlays extension into Horde3D" ON)
if(HORDE3D_BUILD_OVERLAYS)
    add_subdirectory(Bindings)
    add_subdirectory(Source)

    set(HORDE3D_EXTENSION_INSTALLS "${HORDE3D_EXTENSION_INSTALLS}extMan().installExtension( new Horde3DOverlays::ExtOverlays() );\n" PARENT_SCOPE )
    set(HORDE3D_EXTENSION_INCLUDES "${HORDE3D_EXTENSION_INCLUDES}\n#include \"Extensions/Overlays/Source/extension.h\"\n" PARENT_SCOPE )
    set(HORDE3D_EXTENSION_LIBS ${HORDE3D_EXTENSION_LIBS} Horde3DOverlays PARENT_SCOPE )
endif()
