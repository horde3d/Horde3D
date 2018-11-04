add_library(SampleFramework OBJECT sampleapp.cpp)

include_directories(${GLFW_INCLUDE_DIR})
#add_dependencies(SampleFramework project_glfw)
