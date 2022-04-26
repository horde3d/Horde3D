include_directories(../Bindings/C++)
include_directories(../../../Horde3D/Bindings/C++)
include_directories(../../Overlays/Bindings/C++)
include_directories( ${CMAKE_BINARY_DIR} )
include_directories(../../../Horde3D/Samples/Framework)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Android" )
    include( "android.apk" )

    add_library( Terrain SHARED 
        app.cpp
        main.cpp
    )

    set( OUT_PATH "${PROJECT_BINARY_DIR}/Android/Terrain/app/src/main/jniLibs/${ANDROID_ABI}" )

    # Set output path to android project jniLibs, where gradle can find the built application in .so form
    set_target_properties( Terrain PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUT_PATH} )

    ### Copy engine libraries and SDL library
    add_custom_command( TARGET Terrain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3D>/$<TARGET_FILE_NAME:Horde3D> ${OUT_PATH} )
    add_custom_command( TARGET Terrain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3DUtils>/$<TARGET_FILE_NAME:Horde3DUtils> ${OUT_PATH} )
    add_custom_command( TARGET Terrain POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${SDL_LIB_PATH}/libSDL2.so ${OUT_PATH} )

    # Create apk
    android_create_apk( Terrain "com.horde3d.terrain" "${PROJECT_BINARY_DIR}/Android/Terrain" "" 
        ${HORDE3D_ANDROID_PROJECT_PATH} "${PROJECT_BINARY_DIR}/Binaries/Content" )

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "iOS" )
	set(RESOURCES
		"${PROJECT_BINARY_DIR}/Binaries/Content"
		${HORDE3D_IOS_TOOLS_PATH}/LaunchScreen.storyboard
		${HORDE3D_IOS_TOOLS_PATH}/Default.png
	)

	add_executable(Terrain MACOSX_BUNDLE
		app.cpp
		main.cpp
		${RESOURCES}
	)

	# Add plist
	set_target_properties(Terrain PROPERTIES
        MACOSX_BUNDLE ON
		MACOSX_BUNDLE_INFO_PLIST ${HORDE3D_IOS_TOOLS_PATH}/BundleInfo.plist.in)
		
	# Include ios macro
	include( "ios" )

    # Set development team
    SET_XCODE_PROPERTY(Terrain CODE_SIGN_IDENTITY ${IOS_CODE_SIGN_IDENTITY} )
	SET_XCODE_PROPERTY(Terrain DEVELOPMENT_TEAM ${IOS_DEVELOPMENT_TEAM})

	SET_XCODE_PROPERTY(Terrain ENABLE_BITCODE FALSE)

	SET_XCODE_PROPERTY(Terrain RESOURCE ${RESOURCES})
	SET(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${IOS_DEVELOPMENT_TEAM})
	set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	# Attach required frameworks
	target_link_libraries( Terrain "-framework AudioToolbox" "-framework AVFoundation" "-framework CoreAudio"
									"-framework CoreAudioKit" "-framework CoreFoundation" "-framework CoreGraphics"
									"-framework CoreMotion" "-framework CoreVideo" "-framework Foundation"
									"-framework GameController" "-framework QuartzCore" "-framework UiKit"
									"-framework Metal" "-framework OpenGLES")

else()
	# Desktop platforms
    add_executable(Terrain WIN32 MACOSX_BUNDLE
        app.cpp
        main.cpp
    )
endif()

if(HORDE3D_USE_GLFW)
    target_link_libraries(Terrain SampleFramework Horde3D Horde3DUtils ${GLFW_LIBRARIES})
elseif(HORDE3D_USE_SDL)
    target_link_libraries(Terrain SampleFramework Horde3D Horde3DUtils ${SDL2_LIBRARY})
endif()

add_dependencies(Terrain SampleFramework)

file(COPY Content DESTINATION ${HORDE3D_OUTPUT_PATH_PREFIX})
