include_directories(../Bindings/C++)

IF(GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})
    add_subdirectory(Framework)
    add_subdirectory(Chicago)
    add_subdirectory(Knight)
ENDIF(GLFW_FOUND)
