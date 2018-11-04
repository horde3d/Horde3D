
# CMAKE_BINARY_DIR is used for inclusion of automatically generated files
include_directories(. ../Shared ../../Bindings/C++ ../../../ ${CMAKE_BINARY_DIR} ${HORDE3D_EXTENSION_INCLUDE_DIRS})


set(HORDE3D_SOURCES 
	egAnimatables.cpp
	egAnimation.cpp
	egCamera.cpp
	egCom.cpp
	egComputeNode.cpp
	egComputeBuffer.cpp
	egExtensions.cpp
	egGeometry.cpp
	egLight.cpp
	egMain.cpp
	egMaterial.cpp
	egModel.cpp
	egModules.cpp
	egParticle.cpp
	egPipeline.cpp
	egPrimitives.cpp
	egRendererBaseGL2.cpp
	egRendererBaseGL4.cpp
	egRendererBaseGLES3.cpp
	egRenderer.cpp
	egResource.cpp
	egScene.cpp
	egSceneGraphRes.cpp
	egShader.cpp
	egTexture.cpp
	utImage.cpp
	utOpenGL.cpp
	utOpenGLES3.cpp
	config.h
	egAnimatables.h
	egAnimation.h
	egCamera.h
	egCom.h
	egComputeNode.h
	egComputeBuffer.h
	egExtensions.h
	egGeometry.h
	egLight.h
	egMaterial.h
	egModel.h
	egModules.h
	egParticle.h
	egPipeline.h
	egPrerequisites.h
	egPrimitives.h
	egRenderer.h
	egRendererBase.h
	egRendererBaseGL2.h
	egRendererBaseGL4.h
	egRendererBaseGLES3.h
	egResource.h
	egScene.h
	egSceneGraphRes.h
	egShader.h
	egTexture.h
	utImage.h
	utTimer.h
	utOpenGL.h
	utOpenGLES3.h
    ../Shared/utPlatform.h
	../../Bindings/C++/Horde3D.h
	${HORDE3D_EXTENSION_SOURCES}
	)

add_definitions(-DCMAKE )

if (FORCE_GLES)
	add_definitions(-DH3D_FORCE_GLES)
	
	# remove other opengl render backends from build
	list(REMOVE_ITEM HORDE3D_SOURCES "egRendererBaseGL2.h")
	list(REMOVE_ITEM HORDE3D_SOURCES "egRendererBaseGL2.cpp")
	list(REMOVE_ITEM HORDE3D_SOURCES "egRendererBaseGL4.h")
	list(REMOVE_ITEM HORDE3D_SOURCES "egRendererBaseGL4.cpp")
	list(REMOVE_ITEM HORDE3D_SOURCES "utOpenGL.h")
	list(REMOVE_ITEM HORDE3D_SOURCES "utOpenGL.cpp")

endif (FORCE_GLES)

add_library(Horde3D SHARED
	${HORDE3D_SOURCES}
	)


if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	IF(MSVC)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSECURE_SCL=0")		
	ENDIF(MSVC)
	
	if (FORCE_GLES)
		FIND_PACKAGE(OpenGLESv2 REQUIRED)
		# add additional include paths for egl and gles
		include_directories(${EGL_INCLUDE_DIR} ${OPENGLES2_INCLUDE_DIR})
	
		target_link_libraries(Horde3D ${EGL_LIBRARIES} ${OPENGLES2_LIBRARIES} ${HORDE3D_EXTENSION_LIBS})
	else (FORCE_GLES)
		FIND_PACKAGE(OpenGL REQUIRED)
		target_link_libraries(Horde3D ${OPENGL_gl_LIBRARY} ${HORDE3D_EXTENSION_LIBS})
	endif (FORCE_GLES)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if (FORCE_GLES)
		FIND_PACKAGE(OpenGLESv2 REQUIRED)
		# add additional include paths for egl and gles
		include_directories(${EGL_INCLUDE_DIR} ${OPENGLES2_INCLUDE_DIR})
	
		target_link_libraries(Horde3D ${EGL_LIBRARIES} ${OPENGLES2_LIBRARIES} ${HORDE3D_EXTENSION_LIBS})
	else (FORCE_GLES)
		FIND_PACKAGE(OpenGL REQUIRED)
		if( EGL_FOUND )
            add_definitions( -DHAVE_EGL ${EGL_DEFINITIONS} )
        endif( EGL_FOUND )

        target_link_libraries(Horde3D ${OPENGL_gl_LIBRARY} ${EGL_LIBRARIES} ${HORDE3D_EXTENSION_LIBS})
	endif (FORCE_GLES)        	

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
		
	install(TARGETS Horde3D
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties(Horde3D PROPERTIES
		FRAMEWORK TRUE
		PRIVATE_HEADER "egAnimatables.h;egAnimation.h;egCamera.h;egCom.h;egExtensions.h;egGeometry.h;egLight.h;egMaterial.h;egModel.h;egModules.h;egParticle.h;egPipeline.h;egPrerequisites.h;egPrimitives.h;egRenderer.h;egRendererBase.h;egRendererBaseGL2.h;egRendererBaseGL4.h;egRendererBaseGLES3.h;egResource.h;egScene.h;egSceneGraphRes.h;egShader.h;egTexture.h;utImage.h;utTimer.h;utOpenGL.h;utOpenGLES3.h;"
		PUBLIC_HEADER "../../Bindings/C++/Horde3D.h")
	
	FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
	FIND_LIBRARY(CORE_FOUNDATION_LIBRARY CoreFoundation)
	target_link_libraries(Horde3D ${OPENGL_LIBRARY} ${HORDE3D_EXTENSION_LIBS} ${CORE_FOUNDATION_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
