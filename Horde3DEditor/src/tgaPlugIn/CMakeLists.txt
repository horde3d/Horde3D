cmake_minimum_required(VERSION 3.0)

#FIND_PACKAGE(Qt4 REQUIRED)

SET(qtga_SOURCES
	main.cpp
	qtgahandler.cpp
)
SET(qtga_HEADERS
	qtgahandler.h	
)


INCLUDE_DIRECTORIES("../../include" "${CMAKE_CURRENT_BINARY_DIR}")


#INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_LIBRARY( qtga SHARED 
    ${qtga_SOURCES} 
    ${qtga_HEADERS}
)

TARGET_LINK_LIBRARIES( qtga ${QT_LIBRARIES} )
