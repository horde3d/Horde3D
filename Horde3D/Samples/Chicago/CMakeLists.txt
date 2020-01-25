if( ${CMAKE_SYSTEM_NAME} MATCHES "Android" )
	include( "android.apk" )

	add_library( Chicago SHARED 
		app.cpp
		crowd.cpp
		main.cpp
	)

	set( OUT_PATH "${PROJECT_BINARY_DIR}/Android/Chicago/app/src/main/jniLibs/${ANDROID_ABI}" )

	# Set output path to android project jniLibs, where gradle can find the built application in .so form
	set_target_properties( Chicago PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUT_PATH} )

	### Copy engine libraries and SDL library
	add_custom_command( TARGET Chicago POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3D>/$<TARGET_FILE_NAME:Horde3D> ${OUT_PATH} )
	add_custom_command( TARGET Chicago POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Horde3DUtils>/$<TARGET_FILE_NAME:Horde3DUtils> ${OUT_PATH} )
	add_custom_command( TARGET Chicago POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SDL_LIB_PATH}/libSDL2.so ${OUT_PATH} )

	# Create apk
	android_create_apk( Chicago "com.horde3d.chicago" "${PROJECT_BINARY_DIR}/Android/Chicago" "" 
		${HORDE3D_ANDROID_PROJECT_PATH} "${PROJECT_BINARY_DIR}/Binaries/Content" )
else()
	add_executable(Chicago WIN32 MACOSX_BUNDLE
		app.cpp
		crowd.cpp
		main.cpp
	)
endif()

if(HORDE3D_USE_GLFW)
	target_link_libraries(Chicago Horde3D Horde3DUtils SampleFramework ${GLFW_LIBRARIES})
elseif(HORDE3D_USE_SDL)
	target_link_libraries(Chicago Horde3D Horde3DUtils SampleFramework ${SDL2_LIBRARY})
endif()

add_dependencies(Chicago SampleFramework)
