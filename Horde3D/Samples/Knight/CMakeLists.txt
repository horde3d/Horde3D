if( ${CMAKE_SYSTEM_NAME} MATCHES "Android" )
    include( "android.apk" )

    add_library( Knight SHARED 
        app.cpp
        main.cpp
    )

    # Set output path to android project jniLibs, where gradle can find the built application in .so form
    set_target_properties( Knight PROPERTIES 
                           LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jniLibs/${ANDROID_ABI}" )

    ### Copy engine libraries and SDL library
    add_custom_command( TARGET Knight POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3D>/$<TARGET_FILE_NAME:Horde3D> "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jniLibs/${ANDROID_ABI}" )
    add_custom_command( TARGET Knight POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3DUtils>/$<TARGET_FILE_NAME:Horde3DUtils> "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jniLibs/${ANDROID_ABI}" )
    add_custom_command( TARGET Knight POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy ${SDL_LIB_PATH}/libSDL2.so "${PROJECT_BINARY_DIR}/Android/Knight/app/src/main/jniLibs/${ANDROID_ABI}" )

    # Create apk
    android_create_apk( Knight "com.horde3d.knight" "${PROJECT_BINARY_DIR}/Android/Knight" "" 
                        ${HORDE3D_ANDROID_PROJECT_PATH} "${PROJECT_BINARY_DIR}/Binaries/Content" )
                        
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "iOS" )
    set(RESOURCES
        "${PROJECT_BINARY_DIR}/Binaries/Content"
        ${HORDE3D_IOS_TOOLS_PATH}/LaunchScreen.storyboard
        ${HORDE3D_IOS_TOOLS_PATH}/Default.png
    )

    add_executable(Knight MACOSX_BUNDLE
        app.cpp
        main.cpp
        ${RESOURCES}
    )

    # Add plist
    set_target_properties(Knight PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST ${HORDE3D_IOS_TOOLS_PATH}/BundleInfo.plist.in)
        
    # Include ios macro
    include( "ios" )

    # Set development team
    SET_XCODE_PROPERTY(Knight CODE_SIGN_IDENTITY ${IOS_CODE_SIGN_IDENTITY} )
    SET_XCODE_PROPERTY(Knight DEVELOPMENT_TEAM ${IOS_DEVELOPMENT_TEAM})

    SET_XCODE_PROPERTY(Knight ENABLE_BITCODE FALSE)

    SET_XCODE_PROPERTY(Knight RESOURCE ${RESOURCES})
    SET(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${IOS_DEVELOPMENT_TEAM})
    set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # Attach required frameworks
    target_link_libraries( Knight "-framework AudioToolbox" "-framework AVFoundation" "-framework CoreAudio"
                                    "-framework CoreAudioKit" "-framework CoreFoundation" "-framework CoreGraphics"
                                    "-framework CoreMotion" "-framework CoreVideo" "-framework Foundation"
                                    "-framework GameController" "-framework QuartzCore" "-framework UiKit"
                                    "-framework Metal" "-framework OpenGLES")

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten" )
    add_executable(Knight
            app.cpp
            main.cpp
    )
    # note: When using Horde3D for you own project, it would be advised to also save the value "${PROJECT_BINARY_DIR}/Binaries/Content" somewhere in a definition.
    #   For example: 'add_definitions(-DHORDE3D_CONTENT_FOLDER="${PROJECT_BINARY_DIR}/Binaries/Content")'.
    #   So that you can just call 'h3dutLoadResourcesFromDisk(HORDE3D_CONTENT_FOLDER)' in your cpp file.
    # The 'Framework' library project, that these examples depend on, does this as well with 'EXAMPLE_HORDE3D_EMSCRIPTEN_CONTENT_FOLDER'.
    target_link_options(Knight PRIVATE --preload-file "${PROJECT_SOURCE_DIR}/Horde3D/Binaries/Content") # do not use '${PROJECT_BINARY_DIR}/Binaries/Content'. The contents does not exist when embedding it appears.

else()
    # Desktop platforms
    add_executable(Knight WIN32 MACOSX_BUNDLE
        app.cpp
        main.cpp
    )
endif()

if(HORDE3D_USE_GLFW)
    target_link_libraries(Knight SampleFramework Horde3D Horde3DUtils ${GLFW_LIBRARIES})
elseif(HORDE3D_USE_SDL)
    target_link_libraries(Knight SampleFramework Horde3D Horde3DUtils ${SDL2_LIBRARY})
endif()

add_dependencies(Knight SampleFramework)
include_directories(../Framework)

