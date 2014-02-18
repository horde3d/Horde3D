add_subdirectory(Bindings)
add_subdirectory(Source)

set(HORDE3D_EXTENSION_INSTALLS
	${HORDE3D_EXTENSION_INSTALLS}
	"extMan().installExtension( new Horde3DTerrain::ExtTerrain() );\n"
    PARENT_SCOPE	
)
set(HORDE3D_EXTENSION_INCLUDES
	${HORDE3D_EXTENSION_INCLUDES}
	"\n#include \"Extensions/Terrain/Source/extension.h\"\n"
    PARENT_SCOPE
)
set(HORDE3D_EXTENSION_LIBS
	${HORDE3D_EXTENSION_LIBS}
	Horde3DTerrain
    PARENT_SCOPE	
)

IF(GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})
    add_subdirectory(Sample)
ENDIF(GLFW_FOUND)
