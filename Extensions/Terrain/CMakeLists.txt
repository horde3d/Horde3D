add_subdirectory(Bindings)
add_subdirectory(Source)

IF(GLFW_FOUND)
    add_subdirectory(Sample)
ENDIF(GLFW_FOUND)
