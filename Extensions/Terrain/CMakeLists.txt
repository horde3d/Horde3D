# Terrain extension.
option(HORDE3D_BUILD_TERRAIN "Build the terrain extension into Horde3D" ON)

if(HORDE3D_BUILD_TERRAIN)

	add_subdirectory(Bindings)
	add_subdirectory(Source)

	set( HORDE3D_EXTENSION_INSTALLS "${HORDE3D_EXTENSION_INSTALLS}extMan().installExtension( new Horde3DTerrain::ExtTerrain() );\n" PARENT_SCOPE )
	set( HORDE3D_EXTENSION_INCLUDES "${HORDE3D_EXTENSION_INCLUDES}#include \"Extensions/Terrain/Source/extension.h\"\n" PARENT_SCOPE )
	set( HORDE3D_EXTENSION_LIBS ${HORDE3D_EXTENSION_LIBS} Horde3DTerrain PARENT_SCOPE )

	if(HORDE3D_BUILD_EXAMPLES)
		add_subdirectory(Sample)
	endif(HORDE3D_BUILD_EXAMPLES)

	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

endif(HORDE3D_BUILD_TERRAIN)
