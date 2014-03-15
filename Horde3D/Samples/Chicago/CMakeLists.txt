add_executable(Chicago WIN32 MACOSX_BUNDLE
    ../Framework/sampleapp.cpp
	app.cpp
	crowd.cpp
	main.cpp
)

target_link_libraries(Chicago Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Chicago project_glfw)
