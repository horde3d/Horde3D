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

#INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_LIBRARY( EditorLib STATIC ${EditorLib_SOURCES} ${EditorLib_HEADERS} )

TARGET_LINK_LIBRARIES( EditorLib ${LUA_LIBRARIES} ${QT_LIBRARIES} )
