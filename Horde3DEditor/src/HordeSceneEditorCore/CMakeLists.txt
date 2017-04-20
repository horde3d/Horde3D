cmake_minimum_required(VERSION 3.0)

#FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGL QtXml )
IF(MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(MSVC)

SET(HordeSceneEditorCore_SOURCES
	CustomTypes.cpp
	EffectComboBox.cpp
	EffectProperty.cpp
	FrustumProperty.cpp
	HordeFileDialog.cpp
	Importer.cpp
	MaterialComboBox.cpp
	MaterialProperty.cpp
	OverwriteFilesDialog.cpp
	PipelineComboBox.cpp
	PipelineProperty.cpp
	PlugInManager.cpp
	QHordeSceneEditorSettings.cpp
	QSceneNode.cpp
	QUniform.cpp
	SceneFile.cpp
	SceneTreeModel.cpp
	ShaderComboBox.cpp
	ShaderData.cpp
	TextureComboBox.cpp
	TextureProperty.cpp
	Vec3fProperty.cpp
)
SET(HordeSceneEditorCore_HEADERS
	ToolPlugIn.h
	QHordeSceneEditorSettings.h
	ShaderData.h
	CustomTypes.h
	ExtSceneNodePlugIn.h
	Importer.h
	SceneFile.h
	AttachmentPlugIn.h
	EffectComboBox.h
	EffectProperty.h
	FrustumProperty.h
	HordeFileDialog.h
	MaterialComboBox.h
	MaterialProperty.h
	OverwriteFilesDialog.h
	PipelineComboBox.h
	PipelineProperty.h
	PlugInManager.h
	QSceneNode.h
	QUniform.h
	SceneTreeModel.h
	ShaderComboBox.h
	TextureComboBox.h
	TextureProperty.h
	Vec3fProperty.h
)

SET(HordeSceneEditorCore_FORMS
	HordeFileDialog.ui
	OverwriteFilesDialog.ui
)

SET(QT_USE_QTXML TRUE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

INCLUDE_DIRECTORIES( "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/Horde3D/Bindings/C++" "${CMAKE_CURRENT_SOURCE_DIR}/.." ${LUA_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/../../)

ADD_DEFINITIONS(${QT_DEFINITIONS})

source_group("Forms" FILES ${HordeSceneEditorCore_FORMS})

ADD_LIBRARY( HordeSceneEditorCore STATIC ${HordeSceneEditorCore_SOURCES} ${HordeSceneEditorCore_HEADERS} ${HordeSceneEditorCore_FORMS} )

TARGET_LINK_LIBRARIES( HordeSceneEditorCore ${QT_LIBRARIES} )
