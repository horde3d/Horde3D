
include_directories(../Shared)
include_directories(../../Bindings/C++)

# For all platforms except iOS, engine is built as shared library
if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
	add_library(Horde3DUtils STATIC
		main.cpp
		../../Bindings/C++/Horde3DUtils.h
		)
else()
# Other platforms
	add_library(Horde3DUtils SHARED
		main.cpp
		../../Bindings/C++/Horde3DUtils.h
		)	
endif()

target_link_libraries(Horde3DUtils Horde3D)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(TARGETS Horde3DUtils
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties(Horde3DUtils PROPERTIES
		FRAMEWORK TRUE
		PUBLIC_HEADER "../../Bindings/C++/Horde3DUtils.h")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
