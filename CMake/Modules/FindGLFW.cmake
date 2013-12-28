# Locate GLFW library (3.x)
# This module defines
# GLFW_LIBRARY, the name of the library to link against
# GLFW_FOUND, if false, do not try to link to GLFW
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

#MESSAGE("GLFW_LIBRARY is ${GLFW_LIBRARY}")

# GLFW may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lGLFW -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
IF(MINGW)
SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
ENDIF(MINGW)

SET(GLFW_FOUND FALSE)

IF(GLFW_LIBRARY)

    SET(GLFW_LIBRARIES ${GLFW_LIBRARY} CACHE STRING "All the libs required to link GLFW")

    # For threads, as mentioned Apple doesn't need this.
    # In fact, there seems to be a problem if I used the Threads package
    # and try using this line, so I'm just skipping it entirely for OS X.
    IF(NOT APPLE)
    SET(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    ENDIF(NOT APPLE)

    # For OS X, GLFW uses Cocoa as a backend so it must link to Cocoa.
    IF(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")		
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
    FIND_LIBRARY(AGL_LIBRARY AGL)
    SET(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${COCOA_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
    ENDIF(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    # For MinGW library.
    IF(MINGW)
    SET(GLFW_LIBRARIES ${MINGW32_LIBRARY} ${GLFW_LIBRARIES})
    ENDIF(MINGW)
    
    # For Unix, GLFW should be linked to X11-releated libraries.
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    FIND_LIBRARY(X11_LIBRARY X11)
    FIND_LIBRARY(Xrandr_LIBRARY Xrandr)
    FIND_LIBRARY(Xxf86vm_LIBRARY Xxf86vm)
    FIND_LIBRARY(Xi_LIBRARY Xi)
    SET(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${X11_LIBRARY} ${Xrandr_LIBRARY} ${Xxf86vm_LIBRARY} ${Xi_LIBRARY})
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    # Set the final string here so the GUI reflects the final state.
    SET(GLFW_LIBRARY ${GLFW_LIBRARY} CACHE STRING "Where the GLFW Library can be found")

    SET(GLFW_FOUND TRUE)

    #MESSAGE("GLFW_LIBRARIES is ${GLFW_LIBRARIES}")

ENDIF(GLFW_LIBRARY)

MESSAGE("-- Found GLFW: ${GLFW_FOUND}")
