include_directories(../../Bindings/C++)

ExternalProject_Get_Property(project_glfw GLFW_SOURCE_DIR)
# message(STATUS "GLFW include path for Chicago: ${GLFW_SOURCE_DIR}")
include_directories(${GLFW_SOURCE_DIR}/include)
link_directories(${GLFW_SOURCE_DIR}/src/Debug)

IF(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(MSVC)

add_executable(Chicago WIN32 MACOSX_BUNDLE
	app.h
	crowd.h
	app.cpp
	crowd.cpp
	main.cpp
	)
	
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
target_link_libraries(Chicago Horde3D Horde3DUtils glfw3)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
FIND_LIBRARY(X11_LIBRARY X11)
target_link_libraries(Chicago Horde3D Horde3DUtils glfw3 ${X11_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		
	   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
       FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
       FIND_LIBRARY(AGL_LIBRARY AGL)
       FIND_LIBRARY(IOKIT IOKit)
       target_link_libraries(Chicago Horde3D Horde3DUtils glfw3 ${IOKIT} ${COCOA_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
