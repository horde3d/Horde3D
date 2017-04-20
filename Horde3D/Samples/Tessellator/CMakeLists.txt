cmake_minimum_required(VERSION 3.1)

add_executable(Tessellator WIN32 MACOSX_BUNDLE
    ../Framework/sampleapp.cpp
	app.cpp
	main.cpp
)
set_property(TARGET Tessellator PROPERTY CXX_STANDARD 11)

target_link_libraries(Tessellator Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Tessellator SampleFramework)
