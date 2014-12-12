cmake_minimum_required(VERSION 2.4)

# Avoid warning under CMake 2.6
IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.4)
  cmake_policy(SET CMP0003 NEW)
ENDIF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.4)

project(Horde3D)

# accumulate list of sources of extension modules
# this can not be done inside the Extensions folder since cmake can only pass variables to subdirectories, never upwards...
set(HORDE3D_EXTENSION_INSTALLS)
set(HORDE3D_EXTENSION_INCLUDE_DIRS)
set(HORDE3D_EXTENSION_LIBS)


# terrain extension (check egExtensions.cpp if it's activated)
option(HORDE3D_BUILD_TERRAIN "Build the terrain extension into Horde3D" ON)
if(HORDE3D_BUILD_TERRAIN)
	add_subdirectory(Extensions/Terrain)
	set(HORDE3D_EXTENSION_INSTALLS
		${HORDE3D_EXTENSION_INSTALLS}
		"extMan().installExtension( new Horde3DTerrain::ExtTerrain() );\n"		
	)
	set(HORDE3D_EXTENSION_INCLUDES
		${HORDE3D_EXTENSION_INCLUDES}
		"\n#include \"Extensions/Terrain/Source/extension.h\"\n"
	)
	set(HORDE3D_EXTENSION_LIBS
		${HORDE3D_EXTENSION_LIBS}
		Horde3DTerrain		
	)
endif(HORDE3D_BUILD_TERRAIN)

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

# create egExtensions.cpp
CONFIGURE_FILE(Horde3D/Source/Horde3DEngine/egExtensions_auto_include.h.in ${CMAKE_BINARY_DIR}/egExtensions_auto_include.h)
CONFIGURE_FILE(Horde3D/Source/Horde3DEngine/egExtensions_auto_install.h.in ${CMAKE_BINARY_DIR}/egExtensions_auto_install.h)

include(CheckFunctionExists)
check_function_exists(strncpy_s HAVE_STRNCPY_S)

add_subdirectory(Horde3D)

