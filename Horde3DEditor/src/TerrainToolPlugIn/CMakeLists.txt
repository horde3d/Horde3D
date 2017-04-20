cmake_minimum_required(VERSION 3.0)

#FIND_PACKAGE(Qt4 REQUIRED)

SET(TerrainToolPlugIn_SOURCES
	ConverterWizard.cpp
	TerrainToolPlugIn.cpp
)
SET(TerrainToolPlugIn_HEADERS
	ConverterWizard.h
	TerrainToolPlugIn.h
)
SET(TerrainToolPlugIn_FORMS
	ConverterWizard.ui
)

SET(QT_USE_QTXML TRUE)

INCLUDE_DIRECTORIES("../../include" "../HordeSceneEditorCore" "${CMAKE_CURRENT_BINARY_DIR}")

#INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

ADD_LIBRARY( TerrainToolPlugIn SHARED  ${TerrainToolPlugIn_SOURCES}  ${TerrainToolPlugIn_FORMS}  ${TerrainToolPlugIn_HEADERS} )

TARGET_LINK_LIBRARIES( TerrainToolPlugIn ${QT_LIBRARIES} HordeSceneEditorCore ${LUA_LIBRARIES} )
