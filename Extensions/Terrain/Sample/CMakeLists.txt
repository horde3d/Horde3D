include_directories(../Bindings/C++)
include_directories(../../../Horde3D/Bindings/C++)

ExternalProject_Get_Property(project_glfw install_dir)
include_directories(${install_dir}/src/project_glfw/include)
link_directories(${install_dir}/src/project_glfw-build/src/Debug)


IF(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup ")
ENDIF(MSVC)

add_executable(Terrain WIN32 MACOSX_BUNDLE
	app.h
	app.cpp
	main.cpp
	)
add_dependencies(Terrain project_glfw)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
target_link_libraries(Terrain Horde3D Horde3DUtils glfw3)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
FIND_LIBRARY(X11_LIBRARY X11)
target_link_libraries(Terrain Horde3D Horde3DUtils glfw3 ${X11_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		
	   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
       FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
       FIND_LIBRARY(AGL_LIBRARY AGL)
       FIND_LIBRARY(IOKIT IOKit)
       target_link_libraries(Terrain Horde3D Horde3DUtils glfw3 ${IOKIT} ${COCOA_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")




