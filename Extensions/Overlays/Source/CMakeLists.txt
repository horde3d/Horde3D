SET(OVERLAYS_EXTENSION_SOURCES
	extension.cpp	
	overlays.cpp
)

SET(OVERLAYS_EXTENSION_HEADERS	
	extension.h	
	overlays.h
)

# CMAKE_BINARY_DIR is used for inclusion of automatically generated files
include_directories( ${CMAKE_BINARY_DIR} )

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
ADD_LIBRARY( Horde3DOverlays STATIC ${OVERLAYS_EXTENSION_SOURCES}  ${OVERLAYS_EXTENSION_HEADERS} )

