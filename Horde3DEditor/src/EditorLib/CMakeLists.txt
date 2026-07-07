SET(EditorLib_SOURCES
   HighLighter.cpp
   LuaFunctions.cpp
   TextEdit.cpp
)

SET(EditorLib_HEADERS
   LuaFunctions.h
   HighLighter.h
   TextEdit.h
)

INCLUDE_DIRECTORIES("../../include" ${CMAKE_BINARY_DIR} ${LUA_INCLUDE_DIR})

if( Qt6_FOUND )
	INCLUDE_DIRECTORIES(../../include ${QT_INCLUDES} ${Qt6Widgets_INCLUDE_DIRS} ${Qt6Core_INCLUDE_DIRS} )

	list( APPEND EditorLib_SOURCES
		QRegExp.cpp
	)

	list( APPEND EditorLib_HEADERS
		QRegExp.h
	)
else()
	INCLUDE_DIRECTORIES(../../include ${QT_INCLUDES} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} )
endif()

#INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_LIBRARY( EditorLib STATIC ${EditorLib_SOURCES} ${EditorLib_HEADERS} )

TARGET_LINK_LIBRARIES( EditorLib ${LUA_LIBRARIES} ${QT_LIBRARIES} )
