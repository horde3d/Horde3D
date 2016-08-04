include_directories(./src ./include)

set(GLFW_SOURCES
	src/clipboard.c
	src/context.c
	src/gamma.c
	src/init.c
	src/input.c
	src/joystick.c
	src/monitor.c
	src/time.c
	src/window.c
	)


if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(GLFW_SOURCES
		${GLFW_SOURCES}
		src/wgl_context.c
		src/win32_clipboard.c
		src/win32_gamma.c
		src/win32_init.c
		src/win32_joystick.c
		src/win32_monitor.c
		src/win32_time.c
		src/win32_window.c
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	FIND_LIBRARY(X11_LIBRARY X11)
	set(GLFW_SOURCES
		${GLFW_SOURCES}
		src/glx_context.c
		src/x11_clipboard.c
		src/x11_gamma.c
		src/x11_init.c
		src/x11_joystick.c
		src/x11_monitor.c
		src/x11_time.c
		src/x11_unicode.c
		src/x11_window.c
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")



if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	FIND_LIBRARY(COCOA_LIBRARY Cocoa)
	FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
	FIND_LIBRARY(AGL_LIBRARY AGL)
	set(GLFW_SOURCES
		${GLFW_SOURCES}
		src/nsgl_context.m
		src/cocoa_gamma.c
		src/cocoa_time.c
		src/cocoa_clipboard.m
		src/cocoa_init.m
		src/cocoa_joystick.m
		src/cocoa_monitor.m
		src/cocoa_window.m
	)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


add_library(glfw STATIC
	${GLFW_SOURCES}
	)

add_definitions(-D_GLFW_USE_CONFIG_H)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	FIND_PACKAGE(OpenGL REQUIRED)
	if(NOT OPENGL_FOUND)
		message(ERROR " OPENGL not found!")
	endif(NOT OPENGL_FOUND)	
	set(CMAKE_REQUIRED_INCLUDES ${OPENGL_INCLUDE_DIR})
	set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})
	include(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
	CHECK_FUNCTION_EXISTS(glXGetProcAddressARB USE_GLX_GET_PROC_ADDRESS_ARB)
    	IF (USE_GLX_GET_PROC_ADDRESS_ARB)
		add_definitions(-D_GLFW_HAS_GLXGETPROCADDRESSARB)
   	ENDIF (USE_GLX_GET_PROC_ADDRESS_ARB)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

