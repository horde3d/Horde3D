
include_directories(../../Bindings/C++)
include_directories(../glfw)

IF(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(MSVC)

add_executable(Knight WIN32 MACOSX_BUNDLE
	app.h
	app.cpp
	main.cpp
	)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
target_link_libraries(Knight Horde3D Horde3DUtils glfw)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
FIND_LIBRARY(X11_LIBRARY X11)
target_link_libraries(Knight Horde3D Horde3DUtils glfw ${X11_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		
	   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
       FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
       FIND_LIBRARY(AGL_LIBRARY AGL)
       target_link_libraries(Knight Horde3D Horde3DUtils glfw ${COCOA_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
