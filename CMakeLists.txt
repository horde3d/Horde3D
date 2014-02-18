cmake_minimum_required(VERSION 2.8)

# Project details.
project(Horde3D)

SET(${PROJECT_NAME}_MAJOR_VERSION 1)
SET(${PROJECT_NAME}_MINOR_VERSION 0)
SET(${PROJECT_NAME}_PATCH_LEVEL 0)

# Add local repository for FindXXX.cmake modules.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/Modules/" ${CMAKE_MODULE_PATH})

# Check for strncpy_s function.
include(CheckFunctionExists)
check_function_exists(strncpy_s HAVE_STRNCPY_S)

# Look for required GLFW library.
option(HORDE3D_FORCE_DOWNLOAD_GLFW "Force linking Horde3D to a downloaded version of GLFW" OFF)
find_package(GLFW)
IF(GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})
ENDIF(GLFW_FOUND)

# Set binaries output folder.
SET(HORDE3D_BUILD_TYPE ${CMAKE_CFG_INTDIR})
IF(HORDE3D_BUILD_TYPE STREQUAL ".")
SET(HORDE3D_BUILD_TYPE ${CMAKE_BUILD_TYPE})
ENDIF(HORDE3D_BUILD_TYPE STREQUAL ".")
IF(NOT HORDE3D_BUILD_TYPE)
SET(HORDE3D_BUILD_TYPE "Release")
ENDIF(NOT HORDE3D_BUILD_TYPE)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Binaries/${CMAKE_SYSTEM_NAME}/${HORDE3D_BUILD_TYPE}/
    CACHE STRING "Where binaries and shared lib files go" FORCE)

# Manage extensions.
include(Extensions/Extensions.txt)

# Add engine target.
add_subdirectory(Horde3D)

