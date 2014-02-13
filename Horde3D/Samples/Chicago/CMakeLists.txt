
include_directories(../../Bindings/C++)

IF(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(MSVC)

add_executable(Chicago WIN32 MACOSX_BUNDLE
	app.h
	crowd.h
	app.cpp
	crowd.cpp
	main.cpp
	)

target_link_libraries(Chicago Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Chicago project_glfw)
