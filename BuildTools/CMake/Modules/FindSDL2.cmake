
# This module defines
# SDL2_LIBRARY, the name of the library to link against
# SDL2_FOUND, if false, do not try to link to SDL2
# SDL2_INCLUDE_DIR, where to find SDL.h
#
# This module responds to the the flag:
# SDL2_BUILDING_LIBRARY
# If this is defined, then no SDL2main will be linked in because
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the the proper link flags
# as part of the returned SDL2_LIBRARY variable.
#
# Don't forget to include SDLmain.h and SDLmain.m your project for the
# OS X framework based version. (Other versions link to -lSDL2main which
# this module will try to find on your behalf.) Also for OS X, this
# module will automatically add the -framework Cocoa on your behalf.
#
#
# Additional Note: If you see an empty SDL2_LIBRARY_TEMP in your configuration
# and no SDL2_LIBRARY, it means CMake did not find your SDL2 library
# (SDL2.dll, libsdl2.so, SDL2.framework, etc).
# Set SDL2_LIBRARY_TEMP to point to your SDL2 library, and configure again.
# Similarly, if you see an empty SDL2MAIN_LIBRARY, you should set this value
# as appropriate. These values are used to generate the final SDL2_LIBRARY
# variable, but when these values are unset, SDL2_LIBRARY does not get created.
#
#
# $SDL2DIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL2DIR
# used in building SDL2.
# l.e.galup  9-20-02
#
# Modified by Eric Wing.
# Added code to assist with automated building by using environmental variables
# and providing a more controlled/consistent search behavior.
# Added new modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).
# Also corrected the header search path to follow "proper" SDL guidelines.
# Added a search for SDL2main which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of
# SDL2_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from SDL2/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention
# is #include "SDL.h", not <SDL2/SDL.h>. This is done for portability
# reasons because not all systems place things in SDL2/ (see FreeBSD).

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

#message("<FindSDL2.cmake>")

SET(SDL2_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	${SDL2_PATH}
)

FIND_PATH(SDL2_INCLUDE_DIR SDL.h
	HINTS
	$ENV{SDL2DIR}
	PATH_SUFFIXES include/SDL2 include
	PATHS ${SDL2_SEARCH_PATHS}
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(PATH_SUFFIXES lib64 lib/x64 lib)
else() 
	set(PATH_SUFFIXES lib/x86 lib)
endif() 

FIND_LIBRARY(SDL2_LIBRARY_TEMP
	NAMES SDL2
	HINTS
	$ENV{SDL2DIR}
	PATH_SUFFIXES ${PATH_SUFFIXES}
	PATHS ${SDL2_SEARCH_PATHS}
)

IF(NOT SDL2_BUILDING_LIBRARY)
	IF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
		# Non-OS X framework versions expect you to also dynamically link to
		# SDL2main. This is mainly for Windows and OS X. Other (Unix) platforms
		# seem to provide SDL2main for compatibility even though they don't
		# necessarily need it.
		FIND_LIBRARY(SDL2MAIN_LIBRARY
			NAMES SDL2main
			HINTS
			$ENV{SDL2DIR}
			PATH_SUFFIXES ${PATH_SUFFIXES}
			PATHS ${SDL2_SEARCH_PATHS}
		)
	ENDIF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
ENDIF(NOT SDL2_BUILDING_LIBRARY)

# SDL2 may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
	FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

# MinGW needs an additional link flag, -mwindows
# It's total link flags should look like -lmingw32 -lSDL2main -lSDL2 -mwindows
IF(MINGW)
	SET(MINGW32_LIBRARY mingw32 "-mwindows" CACHE STRING "mwindows for MinGW")
ENDIF(MINGW)

IF(SDL2_LIBRARY_TEMP)
	# For SDL2main
	IF(NOT SDL2_BUILDING_LIBRARY)
		IF(SDL2MAIN_LIBRARY)
			SET(SDL2_LIBRARY_TEMP ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY_TEMP})
		ENDIF(SDL2MAIN_LIBRARY)
	ENDIF(NOT SDL2_BUILDING_LIBRARY)

	# For OS X, SDL2 uses Cocoa as a backend so it must link to Cocoa.
	# CMake doesn't display the -framework Cocoa string in the UI even
	# though it actually is there if I modify a pre-used variable.
	# I think it has something to do with the CACHE STRING.
	# So I use a temporary variable until the end so I can set the
	# "real" variable in one-shot.
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} "-framework Cocoa")
	ENDIF()

	# For threads, as mentioned Apple doesn't need this.
	# In fact, there seems to be a problem if I used the Threads package
	# and try using this line, so I'm just skipping it entirely for OS X.
	IF(NOT APPLE)
		SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
	ENDIF(NOT APPLE)

	# For MinGW library
	IF(MINGW)
		SET(SDL2_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL2_LIBRARY_TEMP})
	ENDIF(MINGW)

	# Set the final string here so the GUI reflects the final state.
	SET(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP} CACHE STRING "Where the SDL2 Library can be found")
	# Set the temp variable to INTERNAL so it is not seen in the CMake GUI
	SET(SDL2_LIBRARY_TEMP "${SDL2_LIBRARY_TEMP}" CACHE INTERNAL "")
ENDIF(SDL2_LIBRARY_TEMP)


IF (HORDE3D_FORCE_DOWNLOAD_SDL)
	# Download SDL sources and build it ourselves

	# If not found, try to build with local sources.
    # It uses CMake's "ExternalProject_Add" target.
    MESSAGE(STATUS "Preparing external SDL project")
    INCLUDE(ExternalProject)

	INCLUDE(ExternalProject)
	IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
		# Create external project for sdl with parameters specific for android
		ExternalProject_Add(project_sdl
		URL https://www.libsdl.org/release/SDL2-2.0.9.zip
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}	-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DANDROID_PLATFORM=${ANDROID_PLATFORM} -DANDROID_ABI=${ANDROID_ABI} -DANDROID_DL_LIBRARY=${ANDROID_DL_LIBRARY}
		LOG_DOWNLOAD 1
		LOG_UPDATE 1
		LOG_CONFIGURE 1
		LOG_BUILD 1
		LOG_TEST 1
		LOG_INSTALL 1
		)

		MESSAGE(STATUS "External SDL project done")
	else() # other platforms
		ExternalProject_Add(project_sdl
			URL https://www.libsdl.org/release/SDL2-2.0.9.zip
			CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
			LOG_DOWNLOAD 1
			LOG_UPDATE 1
			LOG_CONFIGURE 1
			LOG_BUILD 1
			LOG_TEST 1
			LOG_INSTALL 1
		)
		MESSAGE(STATUS "External SDL project done")
	endif()

	ExternalProject_Get_Property(project_sdl install_dir)
    SET(SDL2_INCLUDE_DIR
        ${install_dir}/include/SDL2
    )
    
    IF(MSVC)
       SET(SDL_LIBRARY_PATH ${install_dir}/lib/SDL2.lib ) # still needs dll in the end, static libraries are incomplete and not recomended
	ELSE(MSVC)
		IF (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
			SET(SDL_LIBRARY_PATH ${install_dir}/lib/libSDL2.so )
		ELSEIF (UNIX AND NOT APPLE)
			SET(SDL_LIBRARY_PATH ${install_dir}/lib/libSDL2-2.0.so )
		ELSE()
			# Mac
			SET(SDL_LIBRARY_PATH ${install_dir}/lib/libSDL2-2.0.dylib )
		ENDIF()
    ENDIF(MSVC)

    add_library(SDL2_LIBRARY SHARED IMPORTED)
    set_property(TARGET SDL2_LIBRARY PROPERTY IMPORTED_LOCATION  ${SDL_LIBRARY_PATH} )
#    add_custom_command(OUTPUT ${SDL_LIBRARY_PATH} VERBATIM COMMAND ${CMAKE_COMMAND} -E echo "Building GLFW" DEPENDS project_glfw)
#   add_custom_target(SDL_LIBRARY_EXTERN DEPENDS ${SDL_LIBRARY_PATH} project_glfw)
    add_dependencies(SDL2_LIBRARY project_sdl)
#    add_dependencies(SDL_LIBRARY SDL_LIBRARY_EXTERN)
    set(SDL2_LIBRARY ${SDL_LIBRARY_PATH} ${CMAKE_THREAD_LIBS_INIT})

	# Copy library to binaries folder
	if(MSVC)
		add_custom_command(TARGET project_sdl POST_BUILD VERBATIM COMMAND ${CMAKE_COMMAND} -E copy "${install_dir}\\bin\\SDL2.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}Debug\\" )
		add_custom_command(TARGET project_sdl POST_BUILD VERBATIM COMMAND ${CMAKE_COMMAND} -E copy "${install_dir}\\bin\\SDL2.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}Release\\" )
	endif()

	# For android make sdl library path available for other projects (used for samples in android build)
	IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
		get_filename_component( SDL_LIB_PATH ${SDL2_LIBRARY} DIRECTORY )
	ENDIF()

ELSE(HORDE3D_FORCE_DOWNLOAD_SDL)
	# SDL is built somewhere else, use the prebuilt library

	INCLUDE(FindPackageHandleStandardArgs)

	FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR)

	get_filename_component( SDL_LIB_PATH ${SDL2_LIBRARY} DIRECTORY )

	# Copy sdl lib to output folder, where other binaries reside
	IF(MSVC)
		
		MESSAGE("${SDL_LIB_PATH}")

		file(COPY ${SDL_LIB_PATH}/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
		file(COPY ${SDL_LIB_PATH}/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
	ENDIF()

	# For android make sdl library path available for other projects (used for samples in android build)
	IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
#		get_filename_component( SDL_LIB_PATH ${SDL2_LIBRARY} DIRECTORY )
		MESSAGE("${SDL_LIB_PATH}")

	#	file(COPY ${SDL_LIB_PATH}/libSDL2.so DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}) 
	ENDIF()

	# IF( ${CMAKE_SYSTEM_NAME} MATCHES "iOS" )
	# 	# Attach required frameworks
	# 	target_link_libraries( SDL2_LIBRARY "-framework AudioToolbox" "-framework AVFoundation" "-framework CoreAudio"
	# 										"-framework CoreAudioKit" "-framework CoreFoundation" "-framework CoreGraphics"
	# 										"-framework CoreMotion" "-framework CoreVideo" "-framework Foundation"
	# 										"-framework GameController" "-framework QuartzCore" "-framework UiKit"
	# 										"-framework Metal" "-framework OpenGLES")
	# ENDIF()
ENDIF(HORDE3D_FORCE_DOWNLOAD_SDL)



