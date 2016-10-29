add_executable(Tessellator WIN32 MACOSX_BUNDLE
    ../Framework/sampleapp.cpp
	app.cpp
	main.cpp
)

target_link_libraries(Tessellator Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Tessellator SampleFramework)
