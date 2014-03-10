add_executable(Knight WIN32 MACOSX_BUNDLE
    ../Framework/app.cpp
    app.cpp
    main.cpp
)

target_link_libraries(Knight Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Knight project_glfw)
