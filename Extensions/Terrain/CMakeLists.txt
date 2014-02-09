add_subdirectory(Bindings)
add_subdirectory(Source)

IF(GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})
    add_subdirectory(Sample)
ENDIF(GLFW_FOUND)
