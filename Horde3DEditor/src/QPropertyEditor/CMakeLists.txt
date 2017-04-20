cmake_minimum_required(VERSION 3.0)

SET(QPropertyEditor_SOURCES
	ColorCombo.cpp
	EnumProperty.cpp
	Property.cpp
	QPropertyEditorWidget.cpp
	QPropertyModel.cpp
	QVariantDelegate.cpp
)
SET(QPropertyEditor_HEADERS
	ColorCombo.h
	EnumProperty.h
	Property.h
	QPropertyEditorWidget.h
	QPropertyModel.h
	QVariantDelegate.h
)

SET(QT_USE_QTXML TRUE)

INCLUDE_DIRECTORIES(../../include ${QT_INCLUDES} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} )
ADD_DEFINITIONS(${QT_DEFINITIONS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
   endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

ADD_LIBRARY( QPropertyEditor STATIC ${QPropertyEditor_SOURCES} ${QPropertyEditor_HEADERS} )

TARGET_LINK_LIBRARIES( QPropertyEditor ${QT_LIBRARIES} )
