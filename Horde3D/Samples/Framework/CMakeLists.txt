add_library(SampleFramework STATIC "")

if(HORDE3D_USE_GLFW)
    include_directories(${GLFW_INCLUDE_DIR})

    target_sources( SampleFramework 
                    PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR}/sampleapp.cpp
                        ${CMAKE_CURRENT_LIST_DIR}/GLFWFramework.cpp
                        ${CMAKE_CURRENT_LIST_DIR}/GLFWFramework.h
                    PUBLIC
                        ${CMAKE_CURRENT_LIST_DIR}/sampleapp.h
                        ${CMAKE_CURRENT_LIST_DIR}/FrameworkBackend.h)

    add_definitions( -DGLFW_BACKEND )

elseif(HORDE3D_USE_SDL)
    include_directories(${SDL2_INCLUDE_DIR})

    target_sources( SampleFramework 
                    PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR}/sampleapp.cpp
                        ${CMAKE_CURRENT_LIST_DIR}/SDLFramework.cpp
                        ${CMAKE_CURRENT_LIST_DIR}/SDLFramework.h
                    PUBLIC
                        ${CMAKE_CURRENT_LIST_DIR}/sampleapp.h
                        ${CMAKE_CURRENT_LIST_DIR}/FrameworkBackend.h)

    add_definitions( -DSDL_BACKEND )
endif()

#add_dependencies(SampleFramework project_glfw)
