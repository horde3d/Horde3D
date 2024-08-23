cmake_minimum_required(VERSION 3.14) # Native support for Android in CMAKE requires at least version 3.7.

# Project details.
project(Horde3D)

set(CMAKE_CXX_STANDARD 14)

SET(${PROJECT_NAME}_MAJOR_VERSION 2)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_LEVEL 0)

# Add local repository for FindXXX.cmake modules.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/BuildTools/CMake/Modules" ${CMAKE_MODULE_PATH})

# Check for strncpy_s function.
include(CheckFunctionExists)
check_function_exists(strncpy_s HAVE_STRNCPY_S)

# To build or not to build examples (removes GLFW or SDL dependency)
option(HORDE3D_BUILD_EXAMPLES "Builds Horde3D examples" ON)

# Specifies whether to build tests
option(HORDE3D_BUILD_TESTS "Build Horde3D unit and integration tests" OFF)

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

        IF( ${CMAKE_SYSTEM_NAME} MATCHES "iOS" )
            set( HORDE3D_FORCE_DOWNLOAD_SDL OFF )
        endif()

        IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten" )
            add_definitions( "--use-port=sdl2" ) # With this flag emscripten itself will handle the rest, including include-directories and linking
            add_link_options( "--use-port=sdl2" )
            set(SDL2_FOUND TRUE)
        ELSE()
            find_package(SDL2)
        ENDIF()

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

# Render backend selection defaults
option(HORDE3D_USE_GL2 "Add OpenGL 2 render backend. Turns off ES3 render backend." ON)
option(HORDE3D_USE_GL4 "Add OpenGL 4 render backend. Turns off ES3 render backend." ON)
option(HORDE3D_USE_GLES3 "Add OpenGL ES 3 render backend. Turns off desktop GL backends." OFF)
option(HORDE3D_USE_NULL "Add Null render backend. Used for testing. Can work with other render backends." OFF)

if(HORDE3D_USE_NULL)
    set(USE_NULL TRUE)
endif(HORDE3D_USE_NULL)

# Tests always enable Null render backend
if(HORDE3D_BUILD_TESTS)
    set(USE_NULL TRUE)
endif(HORDE3D_BUILD_TESTS)

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

IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    # Copy android project into build directory
    SET(HORDE3D_ANDROID_PROJECT_PATH "${PROJECT_SOURCE_DIR}/BuildTools/android")

    # For Linux we have to set ANDROID_SDK_ROOT variable
    set(ANDROID_SDK_ROOT_PATH "" CACHE STRING "Path to Android SDK")

    # Set rendering interface to GLES3
    set(USE_GLES3 TRUE)
	set(HORDE3D_USE_GL2 OFF CACHE BOOL "Add OpenGL 2 render backend. Turns off ES3 render backend." FORCE)
    set(HORDE3D_USE_GL4 OFF CACHE BOOL "Add OpenGL 4 render backend. Turns off ES3 render backend." FORCE)
    
    # On Linux set gradlew to be executable (a bit of a hack)
    if( ${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux" )
        execute_process( COMMAND chmod "+x" "${PROJECT_SOURCE_DIR}/BuildTools/android/gradlew" )
    endif()
#     file(COPY "BuildTools/android" DESTINATION ${HORDE3D_OUTPUT_PATH_PREFIX}/${CMAKE_SYSTEM_NAME}) 
#     set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${HORDE3D_OUTPUT_PATH_PREFIX}/${CMAKE_SYSTEM_NAME}/android/app/src/main/jniLibs/${ANDROID_ABI} )
ENDIF()

IF( ${CMAKE_SYSTEM_NAME} MATCHES "iOS" )
    include( "ios" )

    SET(HORDE3D_IOS_TOOLS_PATH "${PROJECT_SOURCE_DIR}/BuildTools/ios")

    set(IOS_CODE_SIGN_IDENTITY "Apple Development" CACHE STRING "Code sign (used for application signing)")
    set(IOS_DEVELOPMENT_TEAM "" CACHE STRING "Development team id (used for application signing)")

    # Set rendering interface to GLES3
    set(USE_GLES3 TRUE)
	set(HORDE3D_USE_GL2 OFF CACHE BOOL "Add OpenGL 2 render backend. Turns off ES3 render backend." FORCE)
    set(HORDE3D_USE_GL4 OFF CACHE BOOL "Add OpenGL 4 render backend. Turns off ES3 render backend." FORCE)
    set(HORDE3D_USE_GLES3 ON CACHE BOOL "Add OpenGL ES 3 render backend. Turns off desktop GL backends." FORCE)
ENDIF()


if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten" )
    # WebGL Only support OpenGL ES
    set(HORDE3D_EMSCRIPTEN_STACK_SIZE 524288 CACHE STRING "emscripten stack size (-sSTACKSIZE=...)")
    if(NOT HORDE3D_EMSCRIPTEN_STACK_SIZE)
        set(HORDE3D_EMSCRIPTEN_STACK_SIZE 524288)
    endif()

    set(USE_GLES3 TRUE)
    set(HORDE3D_USE_GL2 OFF CACHE BOOL "Add OpenGL 2 render backend. Turns off ES3 render backend." FORCE)
    set(HORDE3D_USE_GL4 OFF CACHE BOOL "Add OpenGL 4 render backend. Turns off ES3 render backend." FORCE)
    set(HORDE3D_USE_GLES3 ON CACHE BOOL "Add OpenGL ES 3 render backend. Turns off desktop GL backends." FORCE)

    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    add_link_options( -sFULL_ES3 )
    add_link_options( -sSTACK_SIZE=${HORDE3D_EMSCRIPTEN_STACK_SIZE} )

    if(HORDE3D_BUILD_EXAMPLES)
        set (SAMPLE_HORDE3D_EMSCRIPTEN_CONTENT_FOLDER "${HORDE3D_OUTPUT_PATH_PREFIX}/Content")
    endif(HORDE3D_BUILD_EXAMPLES)
endif()

IF(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(MSVC)

# Render backend selection
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
