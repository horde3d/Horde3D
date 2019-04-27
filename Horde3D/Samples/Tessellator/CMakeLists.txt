add_executable(Tessellator WIN32 MACOSX_BUNDLE
	app.cpp
	main.cpp
)
set_property(TARGET Tessellator PROPERTY CXX_STANDARD 11)

if(HORDE3D_USE_GLFW)
    target_link_libraries(Tessellator Horde3D Horde3DUtils SampleFramework ${GLFW_LIBRARIES})
elseif(HORDE3D_USE_SDL)
    target_link_libraries(Tessellator Horde3D Horde3DUtils SampleFramework ${SDL2_LIBRARY})
endif()

add_dependencies(Tessellator SampleFramework)
include_directories(../Framework)
