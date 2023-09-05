SET(QT_USE_QTXML TRUE)

SET(QXmlTree_SOURCES
    QXmlTreeModel.cpp
    QXmlTreeNode.cpp
    QXmlTreeUndoCommands.cpp
    QXmlTreeView.cpp
    )

SET(QXmlTree_HEADERS
    QXmlTreeUndoCommands.h
    QXmlTreeModel.h
    QXmlTreeNode.h
    QXmlTreeView.h
    )

if( Qt6_FOUND )
	INCLUDE_DIRECTORIES(../../include ${QT_INCLUDES} ${Qt6Widgets_INCLUDE_DIRS} ${Qt6Core_INCLUDE_DIRS} ${Qt6Xml_INCLUDE_DIRS} )
else()
	INCLUDE_DIRECTORIES(../../include ${QT_INCLUDES} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS}  ${Qt5Xml_INCLUDE_DIRS} )
endif()

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_LIBRARY( QXmlTree STATIC ${QXmlTree_SOURCES}  ${QXmlTree_HEADERS} )

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

TARGET_LINK_LIBRARIES( QXmlTree )
