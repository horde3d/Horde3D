include_directories(../Bindings/C++)
include_directories(../../../Horde3D/Bindings/C++)

add_executable(Terrain WIN32 MACOSX_BUNDLE
    ../../../Horde3D/Samples/Framework/sampleapp.cpp
	app.cpp
	main.cpp
)
	
target_link_libraries(Terrain Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Terrain SampleFramework)

file(COPY Content DESTINATION ${HORDE3D_OUTPUT_PATH_PREFIX})
