FIND_PACKAGE(OpenGL REQUIRED)

SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTXML TRUE)

SET(Horde3DEditor_SOURCES
    AboutDialog.cpp
    CameraToolBar.cpp
    CheckBoxList.cpp
    ColladaImporter.cpp
    ExtraTreeView.cpp
    FindDialog.cpp
    OpenGLWidget.cpp
    HordeModelDialog.cpp
    HordeSceneEditor.cpp
    ImgLabel.cpp
    LuaBindingsHorde.cpp
    LuaController.cpp
    MaterialWidget.cpp
    MiscToolBar.cpp
    main.cpp
    NodeWidget.cpp
    PipelineTreeModel.cpp
    PipelineTreeView.cpp
    PlugInPage.cpp
    QCameraNode.cpp
    QCameraNodePage.cpp
    QEmitterNode.cpp
    QEmitterNodePage.cpp
    QGroupNode.cpp
    QGroupNodePage.cpp
    QJointNode.cpp
    QLightNode.cpp
    QLightNodePage.cpp
    QMeshNode.cpp
    QModelNode.cpp
    QOneTimeDialog.cpp
    QPipelineCommandQueueNode.cpp
    QPipelineNode.cpp
    QPipelineRenderTargetNode.cpp
    QPipelineClearTargetNode.cpp
    QPipelineDrawGeometryNode.cpp
    QPipelineBindBufferNode.cpp
    QPipelineUnbindBuffersNode.cpp
    QPipelineSwitchTargetNode.cpp
    QPipelineDrawQuadNode.cpp
    QPipelineDrawOverlaysNode.cpp
    QPipelineDoForwardLightLoopNode.cpp
    QPipelineDoDeferredLightLoopNode.cpp
    QPipelineSetUniformNode.cpp
    QPipelineSetupNode.cpp
    QPipelineStageNode.cpp
    QReferenceNode.cpp
    QReferenceNodePage.cpp
    QStdOutListener.cpp
    QTexUnit.cpp
    RenderTargetView.cpp
    SceneFilePage.cpp
    SceneTreeView.cpp
    SceneTreeWidget.cpp
    SceneWizard.cpp
    SettingsDialog.cpp
    )


SET(Horde3DEditor_HEADERS
    AboutDialog.h
    CameraToolBar.h
    CheckBoxList.h
    ColladaImporter.h
    ExtraTreeView.h
    FindDialog.h
    OpenGLWidget.h
    HordeModelDialog.h
    HordeSceneEditor.h
    ImgLabel.h
    LuaController.h
    MaterialWidget.h
    MiscToolBar.h
    NodeWidget.h
    PipelineTreeModel.h
    PipelineTreeView.h
    PlugInPage.h
    QCameraNode.h
    QCameraNodePage.h
    QEmitterNode.h
    QEmitterNodePage.h
    QGroupNode.h
    QGroupNodePage.h
    QJointNode.h
    QLightNode.h
    QLightNodePage.h
    QMeshNode.h
    QModelNode.h
    QOneTimeDialog.h
    QPipelineCommandQueueNode.h
    QPipelineNode.h
    QPipelineRenderTargetNode.h
    QPipelineClearTargetNode.h
    QPipelineDrawGeometryNode.h
    QPipelineBindBufferNode.h
    QPipelineUnbindBuffersNode.h
    QPipelineSwitchTargetNode.h
    QPipelineDrawQuadNode.h
    QPipelineDrawOverlaysNode.h
    QPipelineDoForwardLightLoopNode.h
    QPipelineDoDeferredLightLoopNode.h
    QPipelineSetUniformNode.h
    QPipelineSetupNode.h
    QPipelineStageNode.h
    QReferenceNode.h
    QReferenceNodePage.h
    QStdOutListener.h
    QTexUnit.h
    RenderTargetView.h
    SceneFilePage.h
    SceneTreeView.h
    SceneTreeWidget.h
    SettingsDialog.h
    SceneWizard.h
    LuaBindingsHorde.h
    )

SET(Horde3DEditor_RESOURCES
    resources.qrc
    )

SET(Horde3DEditor_FORMS
    AboutDialog.ui
    CameraToolBar.ui
    ColladaImporter.ui
    FindDialog.ui
    HordeSceneEditor.ui
    LuaController.ui
    MaterialWidget.ui
    MiscToolBar.ui
    PlugInPage.ui
    QCameraNodePage.ui
    QEmitterNodePage.ui
    QGroupNodePage.ui
    QLightNodePage.ui
    QOneTimeDialog.ui
    QReferenceNodePage.ui
    RenderTargetView.ui
    SceneFilePage.ui
    SceneTreeWidget.ui
    SettingsDialog.ui
    )

source_group("Forms" FILES ${Horde3DEditor_FORMS})

IF(MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(MSVC)

INCLUDE_DIRECTORIES("../HordeSceneEditorCore" "${CMAKE_CURRENT_SOURCE_DIR}/.." "${CMAKE_SOURCE_DIR}/Horde3D/Bindings/C++" "${CMAKE_CURRENT_BINARY_DIR}" ${LUA_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}  )

ADD_DEFINITIONS(${QT_DEFINITIONS})

IF( APPLE )
set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.horde3d.${PROJECT_NAME}")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} Version 0.11.0")
set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
set(MACOSX_BUNDLE_SHORT_VERSION_STRING 0.11.0)
set(MACOSX_BUNDLE_BUNDLE_VERSION 0.11.0)
set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2023. All Rights Reserved.")

ADD_EXECUTABLE(Horde3DEditor
    MACOSX_BUNDLE
    ${Horde3DEditor_SOURCES}
    ${Horde3DEditor_HEADERS}
    ${Horde3DEditor_FORMS}
    ${Horde3DEditor_RESOURCES})
else()
    ADD_EXECUTABLE(Horde3DEditor
        ${Horde3DEditor_SOURCES}
        ${Horde3DEditor_HEADERS}
        ${Horde3DEditor_FORMS}
        ${Horde3DEditor_RESOURCES})
endif()

TARGET_LINK_LIBRARIES(Horde3DEditor ${QT_LIBRARIES} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} HordeSceneEditorCore EditorLib QXmlTree QPropertyEditor ${LUA_LIBRARIES} Horde3D Horde3DUtils )

file(COPY Styles DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY SampleScenes DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if(UNIX AND NOT APPLE)
    if(NOT CMAKE_INSTALL_DATADIR)
       SET(CMAKE_INSTALL_DATADIR /usr/share CACHE PATH "Output directory for data and resource files")
    endif(NOT CMAKE_INSTALL_DATADIR)

    # Create desktop file with absolute path to the built executable
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Horde3DEditor.desktop.in 
                    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Horde3DEditor.desktop )

    # Install the desktop and appdata files
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Horde3DEditor.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Horde3DEditor.appdata.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/appdata)

    foreach(size 16 32 48 64)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/editor_${size}.png
		        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps"
		        RENAME horde3deditor.png)
	endforeach()

	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/editor.svg
		    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
            RENAME  horde3deditor.svg)

    # Install the editor
    install(TARGETS Horde3DEditor DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    install(DIRECTORY Styles DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    
endif()
