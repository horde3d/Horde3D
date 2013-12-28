
include_directories(../../Bindings/C++)

IF(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(MSVC)

add_executable(Knight WIN32 MACOSX_BUNDLE
	app.h
	app.cpp
	main.cpp
	)

target_link_libraries(Knight Horde3D Horde3DUtils ${GLFW_LIBRARIES})
