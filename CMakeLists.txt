cmake_minimum_required(VERSION 2.8)

# Avoid warning under CMake 2.6.
cmake_policy(SET CMP0003 NEW)

project(Horde3D)

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

# Requisities and targets.
include(Requisities.txt)
include(Extensions/Extensions.txt)

add_subdirectory(Horde3D)

