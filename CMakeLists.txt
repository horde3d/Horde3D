cmake_minimum_required(VERSION 2.8)

# Avoid warning under CMake 2.6.
cmake_policy(SET CMP0003 NEW)

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
IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.4)
  IF(WIN32)
	IF(MSVC)
	    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Horde3D/Binaries
		  CACHE STRING "Where binaries and .dll files go" FORCE)  
	ELSE(MSVC)
	    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Horde3D/Binaries/Win32
		  CACHE STRING "Where binaries and .dll files go" FORCE)  
	ENDIF(MSVC)
  ELSE(WIN32)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Horde3D/Binaries/${CMAKE_SYSTEM_NAME}
	    CACHE STRING "Where binaries and .dll files go" FORCE)
  ENDIF(WIN32)
ELSE("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.4)    
  IF(WIN32)
	IF(MSVC)
		SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Horde3D/Binaries 
		CACHE STRING "Where executable files go" FORCE)
	ELSE(MSVC)
		SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Horde3D/Binaries/Win32
		CACHE STRING "Where executable files go" FORCE)  	
	ENDIF(MSVC)
	IF(MSVC)
		SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Horde3D/Binaries
		CACHE STRING "Where dll files go" FORCE)  
	ELSE(MSVC)
		SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Horde3D/Binaries/Win32
		CACHE STRING "Where dll files go" FORCE)  
	ENDIF(MSVC)		
  ELSE(WIN32)
     SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Horde3D/Binaries/${CMAKE_SYSTEM_NAME}
	  CACHE STRING "Where executable files go" FORCE)    
  ENDIF(WIN32) 
ENDIF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.4)

# Manage extensions.
include(Extensions/Extensions.txt)

# Add engine target.
add_subdirectory(Horde3D)

