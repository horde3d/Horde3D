set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt6 COMPONENTS Widgets Gui Core Xml OpenGL)
if( Qt6_FOUND )
    SET(QT_LIBRARIES "Qt6::Widgets;Qt6::Gui;Qt6::Core;Qt6::Xml;Qt6::OpenGL")
    FUNCTION(QT4_ADD_RESOURCES)
    ENDFUNCTION(QT4_ADD_RESOURCES)

    set(CMAKE_CXX_STANDARD 17) # Qt6 requires c++ 17
else()
    find_package(Qt5 COMPONENTS Widgets Gui Core Xml OpenGL)
    if( Qt5_FOUND )
        SET(QT_LIBRARIES "Qt5::Widgets;Qt5::Gui;Qt5::Core;Qt5::Xml;Qt5::OpenGL")
        FUNCTION(QT4_ADD_RESOURCES)
        ENDFUNCTION(QT4_ADD_RESOURCES)
    else()
        message( FATAL_ERROR "Required Qt components not found!" )
    endif()
endif()

FIND_PACKAGE(Lua REQUIRED)

add_subdirectory(src/EditorLib)
add_subdirectory(src/QPropertyEditor)
add_subdirectory(src/QXmlTree)
add_subdirectory(src/tgaPlugIn)
add_subdirectory(src/psdPlugIn)
add_subdirectory(src/HordeSceneEditorCore)
add_subdirectory(src/HordeSceneEditor)

if(HORDE3D_BUILD_TERRAIN)
    add_subdirectory(src/QTerrainNodeExtension)
    add_subdirectory(src/TerrainToolPlugIn)
endif(HORDE3D_BUILD_TERRAIN)
