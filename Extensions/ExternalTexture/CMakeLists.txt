# External texture extension allows replacing a Horde3D's OpenGL texture with an externally managed one    
option(HORDE3D_BUILD_EXTERNAL_TEXTURE "Build the ExternalTexture extension into Horde3D" OFF)
if(HORDE3D_BUILD_EXTERNAL_TEXTURE)
	add_subdirectory(Bindings)
	add_subdirectory(Source)

	set(HORDE3D_EXTENSION_INSTALLS "${HORDE3D_EXTENSION_INSTALLS}extMan().installExtension( new Horde3DExternalTexture::ExternalTexture() );\n" PARENT_SCOPE )
	set(HORDE3D_EXTENSION_INCLUDES "${HORDE3D_EXTENSION_INCLUDES}\n#include \"Extensions/ExternalTexture/Source/extension.h\"\n" PARENT_SCOPE )
	set(HORDE3D_EXTENSION_LIBS ${HORDE3D_EXTENSION_LIBS} Horde3DExternalTexture PARENT_SCOPE )
endif()
