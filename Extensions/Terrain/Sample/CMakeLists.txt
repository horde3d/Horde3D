
include_directories(../Bindings/C++)
include_directories(../../../Horde3D/Bindings/C++)

IF(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(MSVC)

add_executable(Terrain WIN32 MACOSX_BUNDLE
	app.h
	app.cpp
	main.cpp
	)
	
target_link_libraries(Terrain Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Terrain project_glfw)
