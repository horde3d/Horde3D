cmake_minimum_required(VERSION 3.7) # Native support for Android in CMAKE requires at least version 3.7.

# Project details.
project(Horde3D)

set(CMAKE_CXX_STANDARD 11)

SET(${PROJECT_NAME}_MAJOR_VERSION 2)
SET(${PROJECT_NAME}_MINOR_VERSION 0)
SET(${PROJECT_NAME}_PATCH_LEVEL 0)

# Add local repository for FindXXX.cmake modules.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/BuildTools/CMake/Modules/" ${CMAKE_MODULE_PATH})

# To build or not to build examples (removes GLFW dependency)
option(HORDE3D_BUILD_EXAMPLES "Builds Horde3D examples" ON)

# Check the required windowing backend
if(HORDE3D_BUILD_EXAMPLES)
    include(CMakeDependentOption)
    # We can use SDL or GLFW

    # SDL is the only choice for IOS and Android
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR 
       ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR
       ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        option(HORDE3D_USE_SDL "Use SDL for window creation/event handling" OFF)
        option(HORDE3D_USE_GLFW "Use GLFW for window creation/event handling" ON)
    else()
        # IOS and android are not supported by glfw   
        option(HORDE3D_USE_SDL "Use SDL for window creation/event handling" ON)
    endif()

    if(HORDE3D_USE_GLFW)
        # Look for required GLFW library.
        option(HORDE3D_FORCE_DOWNLOAD_GLFW "Force linking Horde3D to a downloaded version of GLFW" OFF)
        find_package(GLFW)
        IF(GLFW_FOUND)
            include_directories(${GLFW_INCLUDE_DIR})
        ENDIF(GLFW_FOUND)
    endif()

    if(HORDE3D_USE_SDL)
        # Look for required SDL library.
        option(HORDE3D_FORCE_DOWNLOAD_SDL "Force linking Horde3D to a downloaded version of SDL" OFF)
        find_package(SDL2)
        IF(SDL2_FOUND)
            include_directories(${SDL2_INCLUDE_DIR})
        ENDIF(SDL2_FOUND)
    endif()

        # Only one library can be used, so disable the other one
    if(GLFW_FOUND)
        set(HORDE3D_USE_SDL OFF CACHE BOOL "Use SDL for window creation/event handling" FORCE)
    elseif(SDL_FOUND)
        set(HORDE3D_USE_GLFW OFF CACHE BOOL "Use GLFW for window creation/event handling" FORCE)
    endif()
else()
    message("Not building examples.")
endif(HORDE3D_BUILD_EXAMPLES)

# Render backend selection
option(HORDE3D_USE_GL2 "Add OpenGL 2 render backend. Turns off ES3 render backend." ON)
option(HORDE3D_USE_GL4 "Add OpenGL 4 render backend. Turns off ES3 render backend." ON)
option(HORDE3D_USE_GLES3 "Add OpenGL ES 3 render backend. Turns off desktop GL backends." OFF)

if(HORDE3D_USE_GLES3)
	set(USE_GLES3 TRUE)
	set(HORDE3D_USE_GL2 OFF CACHE BOOL "Add OpenGL 2 render backend. Turns off ES3 render backend." FORCE)
	set(HORDE3D_USE_GL4 OFF CACHE BOOL "Add OpenGL 4 render backend. Turns off ES3 render backend." FORCE)
endif(HORDE3D_USE_GLES3)

if(HORDE3D_USE_GL2 OR HORDE3D_USE_GL4)
	if(HORDE3D_USE_GL2)
		set(USE_GL2 TRUE)
	endif(HORDE3D_USE_GL2)
	
	if(HORDE3D_USE_GL4)
		set(USE_GL4 TRUE)
	endif(HORDE3D_USE_GL4)
	
	set(HORDE3D_USE_GLES3 OFF CACHE BOOL "Add OpenGL ES 3 render backend. Turns off desktop GL backends." FORCE)
endif(HORDE3D_USE_GL2 OR HORDE3D_USE_GL4)

if (NOT HORDE3D_USE_GL2 AND NOT HORDE3D_USE_GL4 AND NOT HORDE3D_USE_GLES3 )
    message( SEND_ERROR "Rendering backend is not selected!")
endif()

# Set binaries output folder.
SET(HORDE3D_OUTPUT_PATH_PREFIX "${PROJECT_BINARY_DIR}/Binaries")
SET(HORDE3D_OUTPUT_PATH_SUFFIX "")
IF(${CMAKE_CFG_INTDIR} STREQUAL ".")
    SET(HORDE3D_OUTPUT_PATH_SUFFIX ${CMAKE_BUILD_TYPE})
    IF(NOT HORDE3D_OUTPUT_PATH_SUFFIX)
        SET(HORDE3D_OUTPUT_PATH_SUFFIX "Release")
    ENDIF(NOT HORDE3D_OUTPUT_PATH_SUFFIX)
ENDIF(${CMAKE_CFG_INTDIR} STREQUAL ".")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${HORDE3D_OUTPUT_PATH_PREFIX}/${CMAKE_SYSTEM_NAME}/${HORDE3D_OUTPUT_PATH_SUFFIX} CACHE STRING "Where binaries and shared lib files go" FORCE)

IF(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(MSVC)


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG" )

# Manage extensions.
include(Extensions/Extensions.txt)

# Add engine target.
add_subdirectory(Horde3D)

option(HORDE3D_BUILD_EDITOR "Build Horde3D Editor" OFF)
if(HORDE3D_BUILD_EDITOR)
  add_subdirectory(Horde3DEditor)
endif(HORDE3D_BUILD_EDITOR)

if(POLICY CMP0058)
    cmake_policy(SET CMP0058 NEW)
endif()
