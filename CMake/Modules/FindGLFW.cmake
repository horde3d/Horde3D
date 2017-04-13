# Locate GLFW library (3.x)
#
# This module defines:
#
# GLFW_FOUND, if false, do not try to link to GLFW
# GLFW_LIBRARY, the name of the library to link against
# GLFW_LIBRARIES, the full list of libs to link against
# GLFW_INCLUDE_DIR, where to find glfw3.h
#=============================================================================

FIND_PATH(GLFW_INCLUDE_DIR glfw3.h
	HINTS
	$ENV{GLFWDIR}
	PATH_SUFFIXES include/GL include
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/include/GLFW
	/usr/include/GLFW
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

#MESSAGE("GLFW_INCLUDE_DIR is ${GLFW_INCLUDE_DIR}")

FIND_LIBRARY(GLFW_LIBRARY
	NAMES glfw3 GLFW
	HINTS
	$ENV{GLFWDIR}
	PATH_SUFFIXES lib64 lib
	PATHS
	/sw
	/usr
	/usr/local
	/opt/local
	/opt/csw
	/opt
)

SET(GLFW_FOUND FALSE)

IF(NOT GLFW_LIBRARY OR HORDE3D_FORCE_DOWNLOAD_GLFW)
    # If not found, try to build with local sources.
    # It uses CMake's "ExternalProject_Add" target.
    MESSAGE(STATUS "Preparing external GLFW project")
    INCLUDE(ExternalProject)
    ExternalProject_Add(project_glfw 
        URL https://github.com/glfw/glfw/releases/download/3.2.1/glfw-3.2.1.zip
        URL_MD5 824C99EEA073BDD6D2FEC76B538F79AF
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DGLFW_BUILD_DOCS:BOOL=OFF -DGLFW_BUILD_EXAMPLES:BOOL=OFF -DGLFW_BUILD_TESTS:BOOL=OFF
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 1
        LOG_INSTALL 1
    )
    MESSAGE(STATUS "External GLFW project done")

    ExternalProject_Get_Property(project_glfw install_dir)
    SET(GLFW_INCLUDE_DIR
        ${install_dir}/src/project_glfw/include
    )
    
    IF(WIN32)
	    SET(GLFW_LIBRARY
	        ${install_dir}/lib/glfw3.lib
	    )
	ELSE(WIN32)
	    SET(GLFW_LIBRARY
	        ${install_dir}/lib/libglfw3.a
	    )
	ENDIF(WIN32)
    
ENDIF(NOT GLFW_LIBRARY OR HORDE3D_FORCE_DOWNLOAD_GLFW)

#MESSAGE("GLFW_LIBRARY is ${GLFW_LIBRARY}")

IF(GLFW_LIBRARY)

    SET(GLFW_LIBRARIES ${GLFW_LIBRARY} CACHE STRING "All the libs required to link GLFW")

    # GLFW may require threads on your system.
    # The Apple build may not need an explicit flag because one of the
    # frameworks may already provide it.
    # But for non-OSX systems, I will use the CMake Threads package.
    # In fact, there seems to be a problem if I used the Threads package
    # and try using this line, so I'm just skipping it entirely for OS X.
    IF(UNIX AND NOT APPLE)
		FIND_PACKAGE(Threads)
		SET(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    
		if ( CMAKE_DL_LIBS )
			SET(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})
		endif()
			
		find_library(RT_LIBRARY rt)
		mark_as_advanced(RT_LIBRARY)
		if (RT_LIBRARY)
			SET( GLFW_LIBRARIES ${GLFW_LIBRARIES} ${RT_LIBRARY} )
		endif()
		
	ENDIF(UNIX AND NOT APPLE)

    # For OS X, GLFW uses Cocoa as a backend so it must link to Cocoa.
    IF(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
    FIND_LIBRARY(IOKIT_LIBRARY IOKit)
    FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo)
    SET(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
    ENDIF(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    # For MinGW library.
    IF(MINGW)
    # MinGW needs an additional library, mwindows
    # It's total link flags should look like -lmingw32 -lGLFW -lmwindows
    # (Actually on second look, I think it only needs one of the m* libraries.)
    SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
    SET(GLFW_LIBRARIES ${MINGW32_LIBRARY} ${GLFW_LIBRARIES})
    ENDIF(MINGW)
    
    # For Unix, GLFW should be linked to X11-related libraries.
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    FIND_PACKAGE(X11 REQUIRED)
	# FIND_LIBRARY(X11_LIBRARY X11)
    # FIND_LIBRARY(Xrandr_LIBRARY Xrandr)
    # FIND_LIBRARY(Xxf86vm_LIBRARY Xxf86vm)
    # FIND_LIBRARY(Xi_LIBRARY Xi)
    SET(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${X11_X11_LIB} ${X11_Xrandr_LIB} ${X11_Xxf86vm_LIB} ${X11_Xinput_LIB} ${X11_Xinerama_LIB} ${X11_Xcursor_LIB})
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    # Set the final string here so the GUI reflects the final state.
    SET(GLFW_LIBRARY ${GLFW_LIBRARY} CACHE STRING "Where the GLFW Library can be found")

    SET(GLFW_FOUND TRUE)

    #MESSAGE("GLFW_LIBRARIES is ${GLFW_LIBRARIES}")

ENDIF(GLFW_LIBRARY)

MESSAGE("-- Found GLFW: ${GLFW_FOUND}")
