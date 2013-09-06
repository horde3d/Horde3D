
include_directories(../Shared)
include_directories(../../Bindings/C++)


add_library(Horde3DUtils SHARED
	main.cpp
	../../Bindings/C++/Horde3DUtils.h
	)
	
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
