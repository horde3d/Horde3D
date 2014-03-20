add_executable(Knight WIN32 MACOSX_BUNDLE
    ../Framework/sampleapp.cpp
    app.cpp
    main.cpp
)

target_link_libraries(Knight Horde3D Horde3DUtils ${GLFW_LIBRARIES})

add_dependencies(Knight SampleFramework)
