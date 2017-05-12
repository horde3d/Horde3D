add_executable(ParticleVortex WIN32 MACOSX_BUNDLE
    ../Framework/sampleapp.cpp
	app.cpp
	main.cpp
)
set_property(TARGET ParticleVortex PROPERTY CXX_STANDARD 11)

target_link_libraries(ParticleVortex Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(ParticleVortex SampleFramework)
