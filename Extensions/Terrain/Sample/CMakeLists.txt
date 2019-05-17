include_directories(../Bindings/C++)
include_directories(../../../Horde3D/Bindings/C++)
include_directories(../../Overlays/Bindings/C++)

add_executable(Terrain WIN32 MACOSX_BUNDLE
	app.cpp
	main.cpp
)

if(HORDE3D_USE_GLFW)
    target_link_libraries(Terrain Horde3D Horde3DUtils SampleFramework ${GLFW_LIBRARIES})
elseif(HORDE3D_USE_SDL)
    target_link_libraries(Terrain Horde3D Horde3DUtils SampleFramework ${SDL2_LIBRARY})
endif()

add_dependencies(Terrain SampleFramework)

file(COPY Content DESTINATION ${HORDE3D_OUTPUT_PATH_PREFIX})
