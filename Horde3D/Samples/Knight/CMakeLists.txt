if( ${CMAKE_SYSTEM_NAME} MATCHES "Android" )
    include( "android.apk" )

    add_library( Knight SHARED 
        app.cpp
        main.cpp
    )

    # Set output path to android project jnilibs, where gradle can find the built application in .so form
    set_target_properties( Knight PROPERTIES 
                           LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jnilibs/${ANDROID_ABI}" )

    ### Copy engine libraries and SDL library
    add_custom_command( TARGET Knight POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3D>/$<TARGET_FILE_NAME:Horde3D> "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jnilibs/${ANDROID_ABI}" )
    add_custom_command( TARGET Knight POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3DUtils>/$<TARGET_FILE_NAME:Horde3DUtils> "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jnilibs/${ANDROID_ABI}" )
    add_custom_command( TARGET Knight POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy ${SDL_LIB_PATH}/libSDL2.so "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jnilibs/${ANDROID_ABI}" )

    # Create apk
    android_create_apk( Knight "com.horde3d.knight" "${PROJECT_BINARY_DIR}/Android/Knight" "" 
                        ${HORDE3D_ANDROID_PROJECT_PATH} "${PROJECT_BINARY_DIR}/Binaries/Content" )
else()
    add_executable(Knight WIN32 MACOSX_BUNDLE
        app.cpp
        main.cpp
    )
endif()

if(HORDE3D_USE_GLFW)
    target_link_libraries(Knight Horde3D Horde3DUtils SampleFramework ${GLFW_LIBRARIES})
elseif(HORDE3D_USE_SDL)
    target_link_libraries(Knight Horde3D Horde3DUtils SampleFramework ${SDL2_LIBRARY})
endif()

add_dependencies(Knight SampleFramework)
include_directories(../Framework)

