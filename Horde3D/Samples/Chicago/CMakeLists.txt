add_executable(Chicago WIN32 MACOSX_BUNDLE
	app.cpp
	crowd.cpp
	main.cpp
)

if(HORDE3D_USE_GLFW)
	target_link_libraries(Chicago Horde3D Horde3DUtils SampleFramework ${GLFW_LIBRARIES})
elseif(HORDE3D_USE_SDL)
	target_link_libraries(Chicago Horde3D Horde3DUtils SampleFramework ${SDL2_LIBRARY})
endif()

add_dependencies(Chicago SampleFramework)
