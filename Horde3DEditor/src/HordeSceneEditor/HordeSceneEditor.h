// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor.
//
// The Horde3D Scene Editor is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The Horde3D Scene Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************

#ifndef HORDESCENEEDITOR_H_
#define HORDESCENEEDITOR_H_

#include "ui_HordeSceneEditor.h"

#include <QtCore/QDir>
#include <QtXml/qdom.h>
#include <QtGui/QClipboard>

#include "PlugInManager.h"

class GLWidget;
class SceneFile;
class QXmlTreeNode;
class QCameraNode;
class MiscToolBar;
class CameraToolBar;
class QTimer;
class QFileSystemWatcher;
class QUndoGroup;

/**
 * \mainpage The Horde3D Scene Editor
 *
 * \section intro_sec Introduction
 *
 * The main purpose for the Horde3D scene editor is the creation and manipulation of Horde3D scenegraph files and 
 * the rapid prototyping of applications by providing a lua script interface to Horde3D.
 *
 * \section install_sec Installation
 *
 * The Horde3D Scene Editor comes with an MSI installer package that provides an interactive wizard for 
 * a customized installation on your computer.
 *  
 * \section config_sec Configuration
 * 
 * There are severall things you can configure in the Horde3D editor. 
 * 
 * \li A plugin folder, used to enhence the editor with plugins for manipulating Horde3D attachment nodes
 * \li A shader editor, used to edit shader code from within the editor
 * \li A model repository. Models, Materials and other Horde3D resources can be imported in a new scene
 *     by using the model repository. Files placed in the repository and added to a scene are copied to the 
 *     local scene folders. This guarantees that the original files are left unchanged.  
 *
 * The default configuration uses the plugin and repository folders created by the installer as subfolders of the
 * installation directory.  As a shader editor the windows notepad will be used by default.
 * To reconfigure these settings you can select the "Settings" menu entry in the File menu of the editor.
 *
 * Next to the folder settings, also the graphical user interface is in a way configurable. The layout of the dock widgets
 * used for the different manipulation of the scene file can be reordered. The layout will be saved automatically when closing 
 * the editor. You can also specifiy that only the dock widgets for the currently selected scene node will be visible by checking
 * "Toggle Dockwindow Visibility" in the Window menu.
 * 
 * \section plugin_sec PlugIns
 * Horde3D supports so called Attachment nodes. Each node in the scene graph can have one (and only one) optional 
 * attachment node that can contain user-defined xml code. If the Horde3D parser finds such a node a registered callback
 * will be called, with the scene graph id of the parent node and the attachment node's xml code as arguments.
 * By using this different enhancements to the scene graph, additional functionality can be created, like phyiscs or sound support.
 * The Horde3D editor provides an interface to such attachments using plugins. The plugin has to implement the AttachmentPlugIn 
 * interface. When you create a new scene you can select one of the available plugins to be used for the attachment management.
 *
 */

/**
 * \brief The HordeSceneEditor allows the creation of Horde3D SceneGraph files. 
 *
 * Additionally extension plugins can be used to create Attachment nodes.
 * 
 * You can configure an editor repository where often used files for scenes are
 * stored. These files can than be imported in a new scene easily.
 */
class HordeSceneEditor : public QMainWindow, protected Ui_HordeSceneEditor
{
	Q_OBJECT
	friend class PlugInManager;

public:
	static HordeSceneEditor* instance() { if (!m_instance) m_instance = new HordeSceneEditor();	return m_instance; }
	static void release();

	virtual ~HordeSceneEditor();

	GLWidget* glContext() {return m_glWidget;}

	SceneFile* currentScene() {return m_sceneFile;}

	PlugInManager* pluginManager() {return m_pluginManager;}

	void addCamera(QCameraNode* camera);
	void removeCamera(QCameraNode* camera);
	void pauseRendering( bool paused );

	void updateLog(QListWidgetItem* item, bool scroll = true);

public slots:
	void openScene(QString fileName = QString());
	void setCamera(const QCameraNode* camera);

protected:
	void closeEvent(QCloseEvent* event);
	void customEvent(QEvent* event);

private slots:
	
	void about();
	void onlineDoc();
	void saveScene();
	void saveSceneAs();
	bool closeScene();
	void cleanScene();
	void newScene();
	void loadStyle( const QString& fileName );

	void openRecentFile();
	void updateRecentFileActions();
	
	/**
	 * Toggles the enable state of all scene actions and windows, depending on the current selection or scene status
	 */
	void updateMenus();
	void updateLog(const QList<QListWidgetItem*>& items);
	void toggleLogMessages();

	/**
	 * Toggles the enable state of all node actions and window visibility in case toggle dock widget visibility is checked
	 */
	void updateNodeActions(QXmlTreeNode* node);
	void changeRenderTimer(int interval);

	void configure();
	void fileChanged(const QString& path);
	void setUndoStack(QUndoStack* undoStack);
	void togglePasteAction();

	void findLog(const QString& find = QString());	

	void setTransformationMode(int mode = -1);

	void setModified();
	
	void nodeRegistered(const QString& name, PlugInManager::NodeType type);
	void nodeUnregistered(const QString& name, PlugInManager::NodeType type);

	void toolRegistered(const QList<QAction*>& actions);

private:
	HordeSceneEditor(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	static HordeSceneEditor* m_instance;

	void addWatches(const QDir& directory, const QStringList& nameFilters);
	void updateFileSystemWatcher();

	void sceneCreated();

	void restoreWindowSettings();
	void saveWindowSettings();

	SceneFile*			m_sceneFile;
	QFileSystemWatcher*	m_fileSystemWatcher;
	QFileSystemWatcher*	m_styleSystemWatcher;

	GLWidget*			m_glWidget;
	QLabel*				m_fpsLabel;

	QTimer*				m_renderTimer;

	QAction*			m_recentSeparator;
	QAction*			m_cameraToolBarAction;
	QAction*			m_miscToolBarAction;
	QAction*			m_actionShowLog;

	QUndoGroup*			m_undoGroup;

	CameraToolBar*		m_cameraToolBar;
	MiscToolBar*		m_miscToolBar;

	PlugInManager*		m_pluginManager;

	enum {MaxRecentFiles = 3 };
	QAction*			m_recentFileActions[MaxRecentFiles];

};
#endif
