include_directories(..)
include_directories(../../Source/Horde3DEngine)
include_directories(../../Source/Shared)
include_directories(. ${CMAKE_BINARY_DIR})

add_executable(H3DUnitTests WIN32 MACOSX_BUNDLE
    test_shaderparser.cpp
    ../main.cpp
)

target_link_libraries(H3DUnitTests Horde3D)
