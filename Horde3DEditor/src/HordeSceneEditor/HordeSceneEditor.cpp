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

#include "HordeSceneEditor.h"
#include "SceneWizard.h"

#include "PlugInManager.h"

#include "QSceneNode.h"
#include "QReferenceNode.h"
#include "QCameraNode.h"
#include "QLightNode.h"
#include "QJointNode.h"
#include "QModelNode.h"
#include "QMeshNode.h"
#include "QGroupNode.h"
#include "QEmitterNode.h"

#include "GLWidget.h"
#include "MiscToolBar.h"
#include "CameraToolBar.h"

#include "AboutDialog.h"
#include "FindDialog.h"
#include "SettingsDialog.h"
#include "QOneTimeDialog.h"
#include "QHordeSceneEditorSettings.h"

#include "AttachmentPlugIn.h"

#include <QSplashScreen>
#include <QtCore/QDateTime>
#include <QMessageBox>
#include <QtCore/QTextStream>
#include <QFileDialog>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtCore/QFileSystemWatcher>
#include <QUndoGroup>
#include <QUndoStack>
#include <QtOpenGL/QtOpenGL>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtCore/QPluginLoader>
#include <QtCore/QLibrary>

#include "SceneFile.h"

#include <Horde3D.h>
#include <Horde3DUtils.h>

HordeSceneEditor::HordeSceneEditor(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QMainWindow(parent, flags), 
m_sceneFile(0), m_glWidget(0)
{
    // Allow access of scene editor instance by other classes without passing in a pointer
    qApp->setProperty("SceneEditorInstance", QVariant::fromValue<void*>(this));
    // Define default folder for asset repository (can be configured in settings)
    qApp->setProperty("DefaultRepoDir", QDir(QApplication::applicationDirPath()+"/../../Content").absolutePath());

    setupUi(this);


	// Set default GL Format
	QGLFormat fmt;
	fmt.setAlpha(true);
	fmt.setDepth(true);
	fmt.setDepthBufferSize(32);
	QGLFormat::setDefaultFormat(fmt);

	// Add FPS label to status bar
	m_fpsLabel = new QLabel(this);
	m_fpsLabel->setMinimumWidth(40);
	m_statusBar->addPermanentWidget(m_fpsLabel);
	m_renderTimer = new QTimer(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	m_renderTimer->setTimerType(Qt::PreciseTimer);
#endif
	// Delete the window when closing it
	setAttribute(Qt::WA_DeleteOnClose);
	QHBoxLayout* layout = new QHBoxLayout(m_glFrame);
    layout->setContentsMargins(0,0,0,0);
	m_glFrame->setLayout(layout);
    m_glFrame->setWindowFlags(Qt::FramelessWindowHint);

	// Add toolbar widgets
	m_cameraToolBar = new CameraToolBar(m_toolBar);
	m_cameraToolBarAction = m_toolBar->addWidget(m_cameraToolBar);
	// connect CameraToolBar camera state
	connect(m_cameraToolBar, SIGNAL(activeCamChanged(const QCameraNode*)), this, SLOT(setCamera(const QCameraNode*)));

	m_toolBar->addSeparator();

	m_miscToolBar = new MiscToolBar(m_toolBar);
	m_miscToolBarAction = m_toolBar->addWidget(m_miscToolBar);
	connect(m_miscToolBar->m_fpsSettings, SIGNAL(valueChanged(int)), this, SLOT(changeRenderTimer(int)));

	// Add toolbar group
	QActionGroup* grp = new QActionGroup(m_toolBar);
	grp->setExclusive(true);
	grp->addAction(m_actionDummyMode);
	grp->addAction(m_actionMoveObject);
	grp->addAction(m_actionRotateObject);
	grp->addAction(m_actionScaleObject);

	// connect transformation actions
	connect(m_actionMoveObject, SIGNAL(toggled(bool)), this, SLOT(setTransformationMode()));
	connect(m_actionRotateObject, SIGNAL(toggled(bool)), this, SLOT(setTransformationMode()));
	connect(m_actionScaleObject, SIGNAL(toggled(bool)), this, SLOT(setTransformationMode()));

	// Connect menu actions
	connect(m_actionOpen, SIGNAL(triggered()), this, SLOT(openScene()));
	connect(m_actionNew, SIGNAL(triggered()), this, SLOT(newScene()));
	connect(m_actionSave, SIGNAL(triggered()), this, SLOT(saveScene()));
	connect(m_actionSaveAs, SIGNAL(triggered()), this, SLOT(saveSceneAs()));
	connect(m_actionClose, SIGNAL(triggered()), this, SLOT(closeScene()));
	connect(m_actionCleanScene, SIGNAL(triggered()), this, SLOT(cleanScene()));

	connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(m_actionOnlineDocumentation, SIGNAL(triggered()), this, SLOT(onlineDoc()));

	connect(m_actionSettings, SIGNAL(triggered()), this, SLOT(configure()));

	connect(m_actionFindLog, SIGNAL(triggered()), this, SLOT(findLog()));
	connect(m_actionClearLog, SIGNAL(triggered()), m_logWidget, SLOT(clear()));
	connect(m_actionLogShowErrors, SIGNAL(triggered()), this, SLOT(toggleLogMessages()));
	connect(m_actionLogShowWarnings, SIGNAL(triggered()), this, SLOT(toggleLogMessages()));
	connect(m_actionLogShowDebug, SIGNAL(triggered()), this, SLOT(toggleLogMessages()));
	connect(m_actionLogShowInfo, SIGNAL(triggered()), this, SLOT(toggleLogMessages()));
	m_logWidget->addAction(m_actionFindLog);
	m_logWidget->addAction(m_actionClearLog);
	m_logWidget->addAction(m_actionLogShowInfo);
	m_logWidget->addAction(m_actionLogShowWarnings);
	m_logWidget->addAction(m_actionLogShowErrors);

	// Add Dock Widget actions to Window menu
	m_menuWindows->addAction(m_sceneElementDock->toggleViewAction());
	m_menuWindows->addAction(m_attachmentDock->toggleViewAction());
	m_actionShowLog = m_logWidgetDock->toggleViewAction();
	m_menuWindows->addAction(m_actionShowLog);
	m_menuWindows->addAction(m_nodeWidgetDock->toggleViewAction());
	m_menuWindows->addAction(m_materialWidgetDock->toggleViewAction());
	m_menuWindows->addAction(m_luaDockWidget->toggleViewAction());
	m_menuWindows->addAction(m_pipelineDock->toggleViewAction());

	// Create scene node factory
	m_pluginManager = new PlugInManager(this);
	connect(m_pluginManager, SIGNAL(nodeRegistered(const QString&, PlugInManager::NodeType)), this, SLOT(nodeRegistered(const QString&, PlugInManager::NodeType)));
	connect(m_pluginManager, SIGNAL(nodeUnregistered(const QString&, PlugInManager::NodeType)), this, SLOT(nodeUnregistered(const QString&, PlugInManager::NodeType)));
	connect(m_pluginManager, SIGNAL(toolRegistered(const QList<QAction*>&)), this, SLOT(toolRegistered(const QList<QAction*>&)));
	m_pluginManager->setPropertyWidget(m_nodeWidget);
	m_pluginManager->init();

	m_pluginManager->registerSceneNode("Model", QModelNode::loadNode, 0);
	m_pluginManager->registerSceneNode("Mesh", QMeshNode::loadNode, 0);
	m_pluginManager->registerSceneNode("Joint", QJointNode::loadNode, 0);
	m_pluginManager->registerSceneNode("Emitter", QEmitterNode::loadNode, SceneWizard::createEmitter);
	m_pluginManager->registerSceneNode("Light", QLightNode::loadNode, SceneWizard::createLight);
	m_pluginManager->registerSceneNode("Camera", QCameraNode::loadNode, SceneWizard::createCamera);
	m_pluginManager->registerSceneNode("Reference", QReferenceNode::loadNode, SceneWizard::createReference);
	m_pluginManager->registerSceneNode("Group", QGroupNode::loadNode, SceneWizard::createGroup);

	// connect different dock widgets 
	connect(m_sceneTreeWidget, SIGNAL(currentNodeChanged(QXmlTreeNode*)), m_pluginManager, SLOT(setCurrentNode(QXmlTreeNode*)));
	connect(m_sceneTreeWidget, SIGNAL(currentNodeChanged(QXmlTreeNode*)), m_nodeWidget, SLOT(setCurrentNode(QXmlTreeNode*)));
	connect(m_sceneTreeWidget, SIGNAL(currentNodeChanged(QXmlTreeNode*)), this, SLOT(updateNodeActions(QXmlTreeNode*)));

	connect(m_sceneTreeWidget, SIGNAL(nodePropertyChanged(QXmlTreeNode*)), m_cameraToolBar, SLOT(nodePropertyChanged(QXmlTreeNode*)));
	connect(m_sceneTreeWidget, SIGNAL(nodePropertyChanged(QXmlTreeNode*)), m_nodeWidget, SLOT(setCurrentNode(QXmlTreeNode*)));
	// Actions may depend on the dynamic properties
	connect(m_sceneTreeWidget, SIGNAL(nodePropertyChanged(QXmlTreeNode*)), this, SLOT(updateNodeActions(QXmlTreeNode*)));

	m_sceneTreeWidget->m_sceneTreeView->addAction(m_actionAddAttachment);
	connect(m_actionAddAttachment, SIGNAL(triggered()), m_sceneTreeWidget->m_sceneTreeView, SLOT(addAttachmentNode()));

	m_sceneTreeWidget->m_sceneTreeView->addAction(m_actionCopy);
	m_sceneTreeWidget->m_sceneTreeView->addAction(m_actionCut);
	m_sceneTreeWidget->m_sceneTreeView->addAction(m_actionPaste);
	m_sceneTreeWidget->m_sceneTreeView->addAction(m_actionRemove);
	m_sceneTreeWidget->m_extraTreeView->addAction(m_actionCopy);
	m_sceneTreeWidget->m_extraTreeView->addAction(m_actionCut);
	m_sceneTreeWidget->m_extraTreeView->addAction(m_actionPaste);
	m_sceneTreeWidget->m_extraTreeView->addAction(m_actionRemove);	
	connect(m_actionCut, SIGNAL(triggered()), m_sceneTreeWidget, SLOT(cutCurrentNode()));
	connect(m_actionCopy, SIGNAL(triggered()), m_sceneTreeWidget, SLOT(copyCurrentNode()));
	connect(m_actionPaste, SIGNAL(triggered()), m_sceneTreeWidget, SLOT(pasteNode()));
	connect(m_actionRemove, SIGNAL(triggered()), m_sceneTreeWidget, SLOT(removeCurrentNode()));
	connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(togglePasteAction()));

	// Action to apply the current camera transformation to the selected node
	connect(m_actionCameraTransformation2Node, SIGNAL(triggered()), m_sceneTreeWidget->m_sceneTreeView, SLOT(useCameraTransformation()));
	m_sceneTreeWidget->m_sceneTreeView->addAction(m_actionCameraTransformation2Node);

	// Action to apply the node transformation to the current camera
	connect(m_actionNodeTransformation2Camera, SIGNAL(triggered()), m_sceneTreeWidget->m_sceneTreeView, SLOT(setCameraTransformation()));
	m_sceneTreeWidget->m_sceneTreeView->addAction(m_actionNodeTransformation2Camera);

	// connect node settings with material widget
	connect(m_nodeWidget, SIGNAL(materialChanged(const QString&)), m_materialWidget, SLOT(setCurrentMaterial(const QString&)));

	// Create Recent File Actions
	for (int i = 0; i < MaxRecentFiles; ++i) 
	{
		m_recentFileActions[i] = new QAction(this);
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], SIGNAL(triggered()),this, SLOT(openRecentFile()));
	}
	// connect modified signals to this window
	connect(m_materialWidget, SIGNAL(modified(bool)), this, SLOT(setWindowModified(bool)));
	connect(m_pluginManager, SIGNAL(sceneModified(bool)), this, SLOT(setWindowModified(bool)));


	// Add recent file actions to file menu
	m_recentSeparator = m_menuFile->insertSeparator(m_actionQuit);
	for (int i = 0; i < MaxRecentFiles; ++i)
		m_menuFile->insertAction(m_recentSeparator, m_recentFileActions[i]);
	updateRecentFileActions();

	// create the undo group and connect different undo stacks with it
	m_undoGroup = new QUndoGroup(this);
	connect(m_undoGroup, SIGNAL(canUndoChanged(bool)), this, SLOT(setModified()));
	QAction* redoAction = m_undoGroup->createRedoAction(m_undoGroup);
	redoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
	m_menuEdit->insertAction(m_menuEdit->actions().isEmpty() ? 0 : m_menuEdit->actions().first(), redoAction);		
	QAction*  undoAction = m_undoGroup->createUndoAction(m_undoGroup);
	undoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
	m_menuEdit->insertAction(redoAction, undoAction);
	connect(m_sceneTreeWidget, SIGNAL(undoStackActivated(QUndoStack*)), this, SLOT(setUndoStack(QUndoStack*)));

	// create a file system watcher to detect changes made on a scene file outside of the editor
	m_fileSystemWatcher = new QFileSystemWatcher(this);
	connect(m_fileSystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(const QString&)));

	// update views and actions
	updateMenus();

	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("WindowState");
	setWindowState((Qt::WindowStates) settings.value("State", Qt::WindowMaximized).toUInt());
	m_styleSystemWatcher = new QFileSystemWatcher(this);
	m_styleSystemWatcher->addPath(
		QApplication::applicationDirPath()+QDir::separator()+
		"Styles"+QDir::separator()+
		settings.value("Style", "default.qss").toString() 
	);
	loadStyle(settings.value("Style", "default.qss").toString());
	connect(m_styleSystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(loadStyle( const QString&)));
	settings.endGroup();

}


HordeSceneEditor::~HordeSceneEditor()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("WindowState");
	settings.setValue("State", (unsigned int) windowState());
	settings.endGroup();
	delete m_pluginManager;
}

void HordeSceneEditor::closeEvent(QCloseEvent* event)
{	
	if (closeScene())
	{
		// delete any existing finddialog to avoid Qt Loop Deadlock
		FindDialog::releaseDialog();
		QMainWindow::closeEvent(event);
	}
	else
		event->ignore();
}

void HordeSceneEditor::customEvent(QEvent* event)
{
    if( static_cast<int>(event->type()) == static_cast<int>(SceneFile::UpdateFileWatcherEvent) )
		updateFileSystemWatcher();
}


void HordeSceneEditor::about()
{
	AboutDialog dlg;
	dlg.exec();
}

void HordeSceneEditor::onlineDoc()
{
	QDesktopServices::openUrl(tr("http://horde3d.org/wiki/index.php5?title=Editor_Tutorials"));
}


void HordeSceneEditor::setCamera(const QCameraNode* camera)
{
	// Called from outside?
	if (sender() == 0)
		m_cameraToolBar->setActiveCamera(camera);
	else if( camera )
	{
		if (camera->pipeline().FileName.isEmpty())
			m_statusBar->showMessage(tr("ERROR: The camera %1 has no valid pipeline").arg(camera->name()));
		else
			m_pipelineWidget->loadPipeline(camera->pipeline().FileName, camera->pipeline().ResourceID);
		m_glWidget->setActiveCam(camera->hordeId());
	}
}

void HordeSceneEditor::sceneCreated()
{
	setWindowModified(false);
	if (m_sceneFile)
	{		
		QApplication::setOverrideCursor(Qt::BusyCursor);
		QPixmap pixmap(":/Icons/Resources/logo.png");
		QSplashScreen loadScreen(pixmap);
		loadScreen.show();
		loadScreen.showMessage(tr("Opening Scene"), Qt::AlignLeft, Qt::white);
		QApplication::processEvents();
		// set pipeline.xml
		loadScreen.showMessage(tr("Creating OpenGL Context"), Qt::AlignLeft, Qt::white);        
		m_glFrame->setVisible( true );	
		m_glWidget = new GLWidget(m_fpsLabel, m_glFrame, 0);
		connect(m_actionFullscreen, SIGNAL(toggled(bool)), m_glWidget, SLOT(setFullScreen(bool)));
		connect(m_actionDebugView, SIGNAL(triggered(bool)), m_glWidget, SLOT(enableDebugView(bool)));
		connect(m_actionRenderBB, SIGNAL(triggered(bool)), m_glWidget, SLOT(setAABBEnabled(bool)));
		connect(m_actionRenderBaseGrid, SIGNAL(triggered(bool)), m_glWidget, SLOT(setBaseGridEnabled(bool)));
		connect(m_miscToolBar->m_navigationSpeed, SIGNAL(valueChanged(double)), m_glWidget, SLOT(setNavigationSpeed(double)));
		connect(m_actionCollisionCheck, SIGNAL(toggled(bool)), m_glWidget, SLOT(setCollisionCheck(bool)));
		connect(m_glWidget, SIGNAL(transformationMode(int)), this, SLOT(setTransformationMode(int)));
		connect(m_glWidget, SIGNAL(moveObject(const float, const float, const float)), m_nodeWidget, SLOT(moveObject(const float, const float, const float)));
		connect(m_glWidget, SIGNAL(rotateObject(const float, const float, const float)), m_nodeWidget, SLOT(rotateObject(const float, const float, const float)));
		connect(m_glWidget, SIGNAL(scaleObject(const float, const float, const float)), m_nodeWidget, SLOT(scaleObject(const float, const float, const float)));
		connect(m_glWidget, SIGNAL(logMessages(const QList<QListWidgetItem*>&)), this, SLOT(updateLog(const QList<QListWidgetItem*>&)));
		connect(m_glWidget, SIGNAL(statusMessage(const QString&, int)), m_statusBar, SLOT(showMessage(const QString&, int)));
		connect(m_glWidget, SIGNAL(nodeSelected(int)), m_sceneTreeWidget, SLOT(selectNode(int)));
		connect(m_glWidget, SIGNAL(fullscreenActive(bool)), m_actionFullscreen, SLOT(setChecked(bool)));
		connect(m_glWidget, SIGNAL(resized(int, int)), m_cameraToolBar, SLOT(viewportResized(int,int)));
		connect(m_sceneTreeWidget, SIGNAL(currentNodeChanged(QXmlTreeNode*)), m_glWidget, SLOT(setCurrentNode(QXmlTreeNode*)));
		connect(m_sceneTreeWidget, SIGNAL(nodeAboutToChange()), m_glWidget, SLOT(resetMode()));
		// ensure initialization of Horde3D before we start loading resources
		m_glWidget->show();
		if ( !m_glWidget->isInitialized() )
		{
			closeScene();
			m_logWidgetDock->setVisible(true);
			QApplication::restoreOverrideCursor();
			return;
		}
		m_glWidget->setNavigationSpeed(m_miscToolBar->m_navigationSpeed->value());
		m_glWidget->enableDebugView(m_actionDebugView->isChecked());
		m_glWidget->setAABBEnabled(m_actionRenderBB->isChecked());
		m_glWidget->setBaseGridEnabled(m_actionRenderBaseGrid->isChecked());
		// set collision check settings	
		m_glWidget->setCollisionCheck(m_actionCollisionCheck->isChecked());
		m_glFrame->layout()->addWidget(m_glWidget);	
		setWindowTitle(m_sceneFile->absoluteSceneFilePath()+"[*]");
		loadScreen.showMessage(tr("Setting pathes"), Qt::AlignLeft, Qt::white);
		// set engine directories and options
		m_sceneFile->init();
		loadScreen.showMessage(tr("Clearing log"), Qt::AlignLeft, Qt::white);
		// init log view
		m_logWidget->clear();	
		// init attachment plugin
		m_pluginManager->setSceneFile(m_sceneFile);
		// set attachment plugin if available to display configuration widget
		m_attachmentDock->setWidget(m_pluginManager->attachmentPlugIn() ? m_pluginManager->attachmentPlugIn()->configurationWidget() : 0);
		// set attachment plugin for custom rendering and update methods
		m_glWidget->setAttachmentPlugIn(m_pluginManager->attachmentPlugIn());
		// Init Material Widget
		m_materialWidget->init();
		loadScreen.showMessage(tr("Loading Scene Elements"), Qt::AlignLeft, Qt::white);
		// load the scene	
		if (m_sceneTreeWidget->setScene(m_sceneFile))
		{
			loadScreen.showMessage(tr("Updating Log View"), Qt::AlignLeft, Qt::white);
			m_glWidget->updateLog();
			loadScreen.showMessage(tr("Loading resources"), Qt::AlignLeft, Qt::white);
			h3dutLoadResourcesFromDisk(".");
			loadScreen.showMessage(tr("Updating Recent Files"), Qt::AlignLeft, Qt::white);
			// Update recent files
			QHordeSceneEditorSettings settings(this);
			settings.beginGroup("RecentFiles");
			QStringList files = settings.value("RecentFileList").toStringList();
			files.removeAll(m_sceneFile->absoluteSceneFilePath());
			files.prepend(m_sceneFile->absoluteSceneFilePath());
			while (files.size() > MaxRecentFiles)
				files.removeLast();
			settings.setValue("RecentFileList", files);
			settings.endGroup();
			updateRecentFileActions();
			loadScreen.showMessage(tr("Setting Filewatcher..."), Qt::AlignLeft, Qt::white);
			// update file system watcher for scn File
			updateFileSystemWatcher();
			// enable lua bindings
			m_luaWidget->setScene(m_sceneFile);
			// update log after showing the scene tree (to ensure that all items have been loaded)		
			connect(m_renderTimer, SIGNAL(timeout()), m_glWidget, SLOT(updateGL()));
			m_renderTimer->start(m_miscToolBar->m_fpsSettings->value() > 0 ? 1000/m_miscToolBar->m_fpsSettings->value() : 0);
			m_glWidget->setFocus(Qt::ActiveWindowFocusReason);
			m_cameraToolBar->setActiveCamera(m_sceneFile->activeCam());
			// Hacky workaround for rendering bug in empty scenes (probably some initialization issues in case no geometry is drawn)
			m_glWidget->enableDebugView( true );	
			m_glWidget->updateGL();
			m_glWidget->enableDebugView( false );
		}
		else
		{
			closeScene();
			m_logWidgetDock->setVisible(true);
			loadScreen.finish(this);
			QApplication::restoreOverrideCursor();
			return;
		}
		loadScreen.finish(this);
		QApplication::restoreOverrideCursor();
	}
	updateMenus();
}

void HordeSceneEditor::newScene()
{
	// try to close any existing scene
	if (!closeScene())
		return; // if closing was canceled return
	// create a new scene using a wizard
	m_sceneFile = SceneWizard::createScene(this);
	m_actionRenderBaseGrid->setChecked( true );
	// if the file was created
	if (m_sceneFile) m_sceneFile = SceneFile::load(m_sceneFile->absoluteSceneFilePath()); // load the new scene
	// update gui 
	sceneCreated();
	// call plugin wizard
	if (m_pluginManager->attachmentPlugIn()) m_pluginManager->attachmentPlugIn()->sceneFileConfig();
}

void HordeSceneEditor::openScene(QString fileName /*= QString()*/)
{
	// check if there has been a file specified 
	if (fileName.isEmpty()) // if not open a dialog
		fileName = QFileDialog::getOpenFileName(this, tr("Select scene file"), QDir::currentPath(), tr("Scene files (*.scn);;All files (*.*)"));
	// if user canceled the file dialog return
	if (fileName.isEmpty())
		return;	
	// try to close any existing scene
	if (!closeScene())
		return; // if closing was canceled return
	// Open Config file where the pipeline.xml, scene directories and the scene graph file are stored
	m_sceneFile = SceneFile::load(fileName);
	// update gui
	sceneCreated();
}

void HordeSceneEditor::saveScene()
{
	Q_ASSERT ( !m_sceneFile->sceneFileName().isEmpty() );
	// Save current active camera
	m_sceneFile->setActiveCam(m_cameraToolBar->activeCameraName());
	// Save material and reload it
	m_materialWidget->save();
	// Now block the file system watcher since we don't want to be notified about the file changes of the scene file
	m_fileSystemWatcher->removePath(m_sceneFile->absoluteSceneFilePath());
	m_sceneTreeWidget->save();
	if (!m_sceneFile->save())
		QMessageBox::warning(this, tr("Error"), tr("Error writing to file"));
	else
		setWindowModified(false);	
	m_fileSystemWatcher->addPath(m_sceneFile->absoluteSceneFilePath());
}

void HordeSceneEditor::saveSceneAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Choose filename"), QDir::currentPath(), tr("Scene Files (*.scn);;All files (*.*)"));
	if (fileName.isEmpty())
		return;

	m_sceneFile->saveAs( QFileInfo( fileName ) );
	setWindowTitle(m_sceneFile->sceneFileName()+"[*]");
	setWindowModified(false);
	updateFileSystemWatcher();
}

bool HordeSceneEditor::closeScene()
{
	// If we had no scene file opened, return immediately
	if (!m_sceneFile) return true;
	// Check if scene has been modified
	if (isWindowModified())
	{
		int result = QMessageBox::question(
			this, 
			tr("Save changes?"), 
			tr("The file has been changed! Save changes?"), 
			QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
		switch (result)
		{
		case QMessageBox::Yes:
			saveScene();
		case QMessageBox::No:
			break;
		case QMessageBox::Cancel:
			return false;
		};
	}
	// Save widget layout
	saveWindowSettings();
	// Check for modified materials
	m_materialWidget->closeMaterial();
	// Stop render events
	m_renderTimer->stop();
	//if (m_glWidget)	m_renderTimer->disconnect(m_glWidget, SLOT(updateGL()));
	// close scene graph tree and extra tree
	m_sceneTreeWidget->setScene(0);
	// Inform all plugins that the scene will be closed (must be after closing the scene tree, since 
	// closing the scene tree is the last access to Horde3D and the plugins may release Horde3D separately)
	m_pluginManager->setSceneFile(0);	
	// close lua stack
	m_luaWidget->setScene(0);
	// close attachment widget
	m_attachmentDock->setWidget(0);
	// close pipeline tree view
	m_pipelineWidget->closeTree();
	// remove file system watchers
	m_fileSystemWatcher->removePaths(m_fileSystemWatcher->files());
	// delete the scene file instance
	delete m_sceneFile;
	m_sceneFile = 0;
	// delete the gl context
	delete m_glWidget;
	m_glWidget = 0;
	// update the actions and menus to avoid invalid operations on not existing scene
	updateMenus();
	// reset window title
	setWindowTitle("Horde3D Scene Editor[*]");
	setWindowModified(false);
	return true;
}

void HordeSceneEditor::cleanScene()
{
	if (QMessageBox::question(this, 
		tr("Really cleanup scene?"), 
		tr("Cleaning scene will first save the current scene and then remove ALL files below the scene file directory" 
		   "\n\"%1\"\n"
		   "not referenced by the scene file!\n\nDo you really want to clean the scene?").arg(QFileInfo(m_sceneFile->absoluteSceneFilePath()).absolutePath()), 
		QMessageBox::Yes | QMessageBox::Default , QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes)
		return;
	QApplication::setOverrideCursor(Qt::BusyCursor);
	saveScene();
	if( !isWindowModified() )
	  m_sceneFile->cleanSceneDirectory();
	QApplication::restoreOverrideCursor();
}

void HordeSceneEditor::updateLog(QListWidgetItem* item, bool scroll /* = true*/)
{	
	Q_ASSERT(item);
	m_logWidget->addItem(item);		
	switch( item->type() )
	{
	case 1:
		item->setText(tr("Error: ") + item->text());
		item->setTextColor(QColor("#EE1100"));	
		item->setHidden(!m_actionLogShowErrors->isChecked());
		break;
	case 2:
		item->setText(tr("Warning: ") + item->text());
		item->setTextColor(QColor("#FFCC00"));
		item->setHidden(!m_actionLogShowWarnings->isChecked());
		break;
	case 3:
		item->setTextColor(QColor("#C0C0C0"));
		item->setHidden(!m_actionLogShowInfo->isChecked());
		break;
	case 4:
		item->setTextColor(QColor("#CC8080"));
		item->setHidden(!m_actionLogShowDebug->isChecked());
		break;
	}
	if( scroll ) m_logWidget->scrollToBottom();
}

void HordeSceneEditor::updateLog(const QList<QListWidgetItem*>& items)
{	
	QList<QListWidgetItem*>::const_iterator iter = items.constBegin();
	while (iter != items.constEnd())
	{	
		updateLog(*iter, false);
		++iter;
	}
	m_logWidget->scrollToBottom();
}

void HordeSceneEditor::toggleLogMessages()
{
	bool hideInfo = !m_actionLogShowInfo->isChecked();
	bool hideWarnings = !m_actionLogShowWarnings->isChecked();
	bool hideErrors = !m_actionLogShowErrors->isChecked();
	bool hideDebug = !m_actionLogShowDebug->isChecked();
	for (int i=0; i<m_logWidget->count(); ++i)
	{
		QListWidgetItem* item = m_logWidget->item(i);
		switch(item->type())
		{
		case 4:
			item->setHidden(hideDebug);
			break;
		case 3:
			item->setHidden(hideInfo);
			break;
		case 2:
			item->setHidden(hideWarnings);
			break;
		case 1:
			item->setHidden(hideErrors);
			break;		
		}
	}
}

void HordeSceneEditor::updateMenus()
{
	restoreWindowSettings();
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("WindowState");
	m_nodeWidgetDock->setVisible(m_sceneFile!=0);
	m_sceneElementDock->setVisible(m_sceneFile!=0);
	m_pipelineDock->setVisible(m_sceneFile!=0 && settings.value("PipelineDock", true).toBool());
	m_logWidgetDock->setVisible(m_sceneFile!=0 && settings.value("LogWidgetDock", true).toBool());
	m_luaDockWidget->setVisible(m_sceneFile!=0 && settings.value("LuaDialog", false).toBool());
	m_attachmentDock->setVisible(m_sceneFile!=0 && m_pluginManager->attachmentPlugIn() != 0);
	m_materialWidgetDock->setVisible(m_sceneFile!=0  && settings.value("MaterialDock", true).toBool());
	if( m_sceneFile ) togglePasteAction();
	else m_actionPaste->setEnabled(false);
	m_actionFullscreen->setEnabled(m_sceneFile!=0);
	m_actionCollisionCheck->setEnabled(m_sceneFile!=0);
	m_actionDebugView->setEnabled(m_sceneFile!=0);
	m_actionSave->setEnabled(m_sceneFile!=0);
	m_actionSaveAs->setEnabled(m_sceneFile!=0);
	m_actionClose->setEnabled(m_sceneFile!=0);
	m_actionCleanScene->setEnabled(m_sceneFile!=0);
	m_actionRenderBB->setEnabled(m_sceneFile!=0);
	m_actionRenderBaseGrid->setEnabled(m_sceneFile!=0);	
	// Toggle all dock widget actions...
	foreach(QAction* action, m_menuWindows->actions())
		action->setEnabled(m_sceneFile!=0);
	// ... except the log one, this can be always enabled
	m_actionShowLog->setEnabled(true);
	m_menuScene->setEnabled(m_sceneFile != 0);
	m_menuEdit->setEnabled(m_sceneFile != 0);
	m_menuView->setEnabled(m_sceneFile != 0);
	m_glFrame->setVisible( m_sceneFile != 0 );	
	m_cameraToolBarAction->setVisible(m_sceneFile!=0);
	m_miscToolBarAction->setVisible(m_sceneFile!=0);
	m_cameraToolBar->setEnabled(m_sceneFile!=0);
	m_miscToolBar->setEnabled(m_sceneFile!=0);
	m_menuExtras->setEnabled( !m_menuExtras->actions().isEmpty() );
	m_sceneTreeWidget->m_extraTreeView->setEnabled( !m_menuExtras->actions().isEmpty() );
	settings.endGroup();
}


void HordeSceneEditor::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if( action ) openScene(action->data().toString());
}

void HordeSceneEditor::updateRecentFileActions()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("RecentFiles");
	QStringList files = settings.value("RecentFileList").toStringList();
	settings.endGroup();
	int numRecentFiles = qMin(files.size(), (int) MaxRecentFiles);
	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
		m_recentFileActions[i]->setText(text);
		m_recentFileActions[i]->setData(files[i]);
		m_recentFileActions[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		m_recentFileActions[j]->setVisible(false);
	m_recentSeparator->setVisible(numRecentFiles > 0);
}

void HordeSceneEditor::updateNodeActions(QXmlTreeNode* node)
{
	m_actionCopy->setEnabled(node!=0);
	m_actionCut->setEnabled(node!=0);
	m_actionRemove->setEnabled(node!=0);
	m_actionMoveObject->setEnabled( node != 0 && node->property("Position").isValid() );
	m_actionRotateObject->setEnabled(node != 0 && node->property("Rotation").isValid() );
	m_actionScaleObject->setEnabled( node != 0 && node->property("Scale").isValid() );
	m_actionCameraTransformation2Node->setEnabled( 
		m_actionMoveObject->isEnabled() || 
		m_actionRotateObject->isEnabled() || 
		m_actionScaleObject->isEnabled() );
	m_actionNodeTransformation2Camera->setEnabled( m_actionCameraTransformation2Node->isEnabled() );
	bool sceneNode = node != 0 && node->property("__Horde3D").isValid();
	m_actionAddAttachment->setEnabled( sceneNode && m_pluginManager->attachmentPlugIn());
	m_actionAddAttachment->setText( 
		sceneNode && !node->xmlNode().firstChildElement("Attachment").isNull() ? 
		tr("Remove Attachment") : tr("Add Attachment"));
	togglePasteAction();
	if (m_actionToggleDockwindowVisibility->isChecked())
	{
		if( node )
		{			
			m_nodeWidgetDock->setVisible(node);
			m_materialWidgetDock->setVisible(node->xmlNode().hasAttribute("material"));
		}
		else
		{
			m_materialWidgetDock->setVisible(false);
			m_nodeWidgetDock->setVisible(false);
		}
	}
}

void HordeSceneEditor::changeRenderTimer(int interval)
{	
	if (m_renderTimer->isActive())
		m_renderTimer->start(interval > 0 ? (int) (1000.0/interval + 0.5) : 0);
}


void HordeSceneEditor::configure()
{
	SettingsDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QHordeSceneEditorSettings settings(this);
		settings.beginGroup("General");
		int limit = settings.value("UndoStackSize").toInt();
		QList<QUndoStack*> stacks = m_undoGroup->stacks();
		foreach(QUndoStack* stack, stacks)
			stack->setUndoLimit(limit);		
		if( !m_styleSystemWatcher->files().empty() )
			m_styleSystemWatcher->removePaths( m_styleSystemWatcher->files() );
		QFileInfo style = QApplication::applicationDirPath()+QDir::separator()+"Styles"+QDir::separator()+settings.value("Style", "default.qss").toString();
		if( style.exists() )
			m_styleSystemWatcher->addPath( style.absoluteFilePath() );					
		loadStyle( style.absoluteFilePath() );
		settings.endGroup();
		if (m_glWidget)
			m_glWidget->loadButtonConfig();
	}
}


void HordeSceneEditor::fileChanged(const QString& path)
{
	if (m_sceneFile->absoluteSceneFilePath() == path)
	{		
		QHordeSceneEditorSettings settings(this);
		settings.beginGroup("ConfirmDialogs");
		int reloadMode = settings.value("ReloadMode", 0).toInt();
		if (reloadMode != 0)
		{
			reloadMode = QOneTimeDialog::question(this, tr("Reload scene"), 
				tr("%1\n\nThe current scene file has been changed outside of the Editor!\n\nDo you want to reload it? (Unsaved changes will be lost)").arg(path),
				QDialogButtonBox::Yes, QDialogButtonBox::No);
			if (reloadMode & QOneTimeDialog::ApplyAlways)
				settings.setValue("ReloadMode", reloadMode & ~QOneTimeDialog::ApplyAlways);
		}
		if( (reloadMode & ~QOneTimeDialog::ApplyAlways) == QDialogButtonBox::Yes)
		{
			setWindowModified(false);
			openScene(path);
			return;
		}
	}
	QString relPath = QDir::current().relativeFilePath( path );
	H3DRes resource = 0;
	// Shader file?
	if (resource == 0 )
		resource = h3dFindResource(H3DResTypes::Shader, qPrintable( relPath ) );
	// Code file?
	if (resource == 0 )
		resource = h3dFindResource(H3DResTypes::Code, qPrintable( relPath ) );		
	// Material ? 
	if (resource == 0 )
		resource = h3dFindResource(H3DResTypes::Material, qPrintable( relPath ) );
	// Texture ? 
	if (resource == 0 )
		resource = h3dFindResource(H3DResTypes::Texture, qPrintable( relPath ) );
	// Effect ?
	if (resource == 0 )
		resource = h3dFindResource(H3DResTypes::ParticleEffect, qPrintable( relPath ) );
	// Pipeline ?
	if (resource == 0 )
		resource = h3dFindResource(H3DResTypes::Pipeline, qPrintable( relPath ) );		

	// If the resource was loaded before 	
	if (resource != 0)
	{
		// Ensure file has been written completly
		QTime time = QTime::currentTime();
		QFile file(path);
		while (!file.open(QIODevice::ReadOnly) || file.size() == 0)
		{
			QApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents);
			if (time.secsTo(QTime::currentTime()) > 4)
			{
				m_statusBar->showMessage("Aborting reload of resource! Timeout for getting read access reached!", 10000);
				return;
			}
			file.close();
		}
		file.close();

		// Mark it invalid
		h3dUnloadResource(resource);
		QListWidgetItem* item = new QListWidgetItem(tr("Reloading resource: %1").arg(path), m_logWidget);
		item->setForeground(QColor("#22CC22"));
		// Reload it again
		h3dutLoadResourcesFromDisk(".");
		// If it's a pipeline check if we have to update the pipeline view
		if (h3dGetResType(resource) == H3DResTypes::Pipeline && m_cameraToolBar->currentPipelineID() == resource)
			m_pipelineWidget->loadPipeline(m_cameraToolBar->currentPipelineFile(), m_cameraToolBar->currentPipelineID());
	}
    // See http://stackoverflow.com/questions/18300376/qt-qfilesystemwatcher-signal-filechanged-gets-emited-only-once/30076119
    m_fileSystemWatcher->removePath(path);
    m_fileSystemWatcher->addPath(path);
}

void HordeSceneEditor::setUndoStack(QUndoStack* undoStack)
{
	m_undoGroup->addStack(undoStack);	
	m_undoGroup->setActiveStack(undoStack);
}

void HordeSceneEditor::togglePasteAction()
{
	const QMimeData* mime = QApplication::clipboard()->mimeData();;
	if (mime->hasFormat("text/plain"))
	{
		QDomDocument dom("Paste");
		m_actionPaste->setEnabled(dom.setContent(mime->data("text/plain")));				
	}
	else
		m_actionPaste->setEnabled(false);
}

void HordeSceneEditor::findLog(const QString& find/*= QString()*/)
{
	if (find.isEmpty())
	{
		FindDialog* dlg = FindDialog::showFindDialog(this);
		// Avoid multiple connections
		dlg->disconnect(SIGNAL(find(const QString&)));
		connect(dlg, SIGNAL(find(const QString&)), this, SLOT(findLog(const QString&)));
	}
	else
	{
		QList<QListWidgetItem*> items = m_logWidget->findItems(find, Qt::MatchContains);
		if (items.isEmpty())
			m_statusBar->showMessage(tr("%1 not found in log data").arg(find), 5000);
		else
		{
			m_logWidgetDock->setVisible(true);
			int nextIndex = 0;
			for (int i=0; i<items.size(); ++i)
			{
				if (m_logWidget->row(items[i]) > m_logWidget->currentRow())
				{
					nextIndex = i;
					break;
				}
			}
			m_logWidget->setCurrentRow(m_logWidget->row(items[nextIndex]));
		}
	}
}

void HordeSceneEditor::setTransformationMode(int mode /*= -1*/)
{
	if( !m_glWidget) return;

	disconnect(m_glWidget, SIGNAL(transformationMode(int)), this, SLOT(setTransformationMode(int)));
	switch(mode)
	{
	case -1:
		{					
			if (m_actionMoveObject->isChecked())
				m_glWidget->setTransformationMode(GLWidget::MoveObject);				
			else if (m_actionRotateObject->isChecked())
				m_glWidget->setTransformationMode(GLWidget::RotateObject);
			else if (m_actionScaleObject->isChecked())
				m_glWidget->setTransformationMode(GLWidget::ScaleObject);
			else 
				m_glWidget->setTransformationMode(GLWidget::None);			
		}
		break;
	case GLWidget::MoveObject:
		m_actionMoveObject->setChecked(true);
		break;
	case GLWidget::RotateObject:
		m_actionRotateObject->setChecked(true);
		break;
	case GLWidget::ScaleObject:
		m_actionScaleObject->setChecked(true);
		break;
	case GLWidget::None:
		m_actionDummyMode->setChecked(true);
		break;
	}
	connect(m_glWidget, SIGNAL(transformationMode(int)), this, SLOT(setTransformationMode(int)));
}

void HordeSceneEditor::setModified()
{
	if (!isWindowModified() && m_undoGroup->canUndo())
		setWindowModified(true);
}

void HordeSceneEditor::nodeRegistered(const QString& name, PlugInManager::NodeType type)
{
	QAction* action = new QAction(tr("Add %1").arg(name), this);
	action->setData(name);
	if(type == PlugInManager::SCENENODE)
	{
		connect(action, SIGNAL(triggered()), m_sceneTreeWidget->m_sceneTreeView, SLOT(addNode()));
		m_menuScene->insertAction(m_menuScene->actions().first(), action);
		m_sceneTreeWidget->m_sceneTreeView->addAction(action);
	}
	else if( type == PlugInManager::EXTRANODE )
	{
		connect(action, SIGNAL(triggered()), m_sceneTreeWidget->m_extraTreeView, SLOT(addNode()));
		m_menuExtras->addAction(action);
		m_sceneTreeWidget->m_extraTreeView->addAction(action);
	}
}

void HordeSceneEditor::nodeUnregistered(const QString& name, PlugInManager::NodeType /*type*/)
{
	QList<QAction*> actions = m_menuScene->actions();
	actions << m_menuExtras->actions();
	foreach(QAction* action, actions)
	{
		if (action->text() == tr("Add %1").arg(name))
		{
			delete action;
			break;
		}
	}
}

void HordeSceneEditor::toolRegistered(const QList<QAction*>& actions)
{
	m_menuTools->addActions(actions);
}

void HordeSceneEditor::updateFileSystemWatcher()
{	
	if ( !m_fileSystemWatcher->files().isEmpty() )
		m_fileSystemWatcher->removePaths(m_fileSystemWatcher->files());
	if (m_sceneFile) // required since this method can also be called from within the new scene wizard where the scene file is not yet created
	{
		m_fileSystemWatcher->addPath(m_sceneFile->absoluteSceneFilePath());
		QStringList textureFilter;
		textureFilter << "*.jpg" << "*.tga" << "*.png" << "*.bmp" << "*.psd" << "*.dds";
		addWatches(QDir::currentPath(), textureFilter );
		addWatches(QDir::currentPath(), QStringList("*.particle.xml"));
		addWatches(QDir::currentPath(), QStringList("*.material.xml"));
		addWatches(QDir::currentPath(), QStringList("*.shader"));
		addWatches(QDir::currentPath(), QStringList("*.glsl"));
		addWatches(QDir::currentPath(), QStringList("*.pipeline.xml"));
		m_materialWidget->init();
	}
}

void HordeSceneEditor::loadStyle( const QString& fileName )
{		
	QFile file( fileName );
	if (file.open(QFile::ReadOnly))
	{
		QApplication::setOverrideCursor(Qt::BusyCursor);
		QString styleSheet = QLatin1String(file.readAll());     
		qApp->setStyleSheet(styleSheet);    
		QApplication::restoreOverrideCursor();
	}
	else
		m_statusBar->showMessage(tr("Couldn't open style file %1").arg(fileName), 10000);	
}

void HordeSceneEditor::addCamera(QCameraNode* camera)
{
	m_cameraToolBar->m_activeCam->addItem(camera->name(), QVariant::fromValue<void*>(camera));
	connect(camera, SIGNAL(pipelineChanged(QCameraNode*)), m_cameraToolBar, SLOT(pipelineChanged(QCameraNode*)));
}

void HordeSceneEditor::removeCamera(QCameraNode* camera)
{
	int index = m_cameraToolBar->m_activeCam->findData(QVariant::fromValue<void*>(camera));
	if (index != -1) m_cameraToolBar->m_activeCam->removeItem(index);
}

QCameraNode *HordeSceneEditor::activeCam()
{
	return m_cameraToolBar->currentCam();
}

void HordeSceneEditor::pauseRendering( bool paused )
{
	if( paused )
		m_renderTimer->stop();
	else 
		m_renderTimer->start( m_miscToolBar->m_fpsSettings->value() > 0 ? 1000/m_miscToolBar->m_fpsSettings->value() : 0 );
}

void HordeSceneEditor::addWatches(const QDir &directory, const QStringList& nameFilters )
{
	QFileInfoList files = directory.entryInfoList(nameFilters, QDir::Readable | QDir::Files | QDir::NoSymLinks);

	QStringList watches;
	foreach(QFileInfo file, files)
		watches	 << file.absoluteFilePath();
	if (!watches.isEmpty())
		m_fileSystemWatcher->addPaths(watches);		

	QFileInfoList subDirs = directory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::NoSymLinks);
	foreach (QFileInfo subDir, subDirs)
		addWatches(QDir(subDir.absoluteFilePath()), nameFilters);
}

void HordeSceneEditor::restoreWindowSettings()
{
	// restore window settings
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("WindowState");
	restoreState(settings.value("DockWidgets").toByteArray());
	m_actionToggleDockwindowVisibility->blockSignals(true);
	m_actionToggleDockwindowVisibility->setChecked(settings.value("DockWidgetToggling", false).toBool());
	m_actionToggleDockwindowVisibility->blockSignals(false);
	settings.endGroup();
	settings.beginGroup("RenderView");
	m_miscToolBar->m_navigationSpeed->setValue(settings.value("NavigationSpeed", 15.0).toDouble());
	m_miscToolBar->m_fpsSettings->setValue(settings.value("MaxFPS", 0.0).toDouble());
	m_actionCollisionCheck->setChecked(settings.value("CollisionCheck", false).toBool());
	m_actionRenderBaseGrid->setChecked(settings.value("BaseGrid", false).toBool());
	m_actionRenderBB->setChecked(settings.value("BoundingBoxForSelection", false).toBool());
	settings.endGroup();
	settings.beginGroup("LogWidget");
	m_actionLogShowInfo->setChecked(settings.value("ShowInfos", true).toBool());
	m_actionLogShowWarnings->setChecked(settings.value("ShowWarnings", true).toBool());
	m_actionLogShowErrors->setChecked(settings.value("ShowErrors", true).toBool());
	m_actionLogShowDebug->setChecked(settings.value("ShowDebugInfos", true).toBool());
	settings.endGroup();
}

void HordeSceneEditor::saveWindowSettings()
{
	// save window settings
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("WindowState");
	settings.setValue("DockWidgets", saveState());	
	settings.setValue("DockWidgetToggling", m_actionToggleDockwindowVisibility->isChecked());
	settings.setValue("LuaDialog", m_luaDockWidget->isVisible());
	settings.setValue("LogWidgetDock", m_logWidgetDock->isVisible());
	settings.setValue("PipelineDock", m_pipelineDock->isVisible());
	settings.setValue("MaterialDock", m_materialWidgetDock->isVisible());
	settings.endGroup();

	settings.beginGroup("RenderView");
	settings.setValue("NavigationSpeed", m_miscToolBar->m_navigationSpeed->value());
	settings.setValue("MaxFPS", m_miscToolBar->m_fpsSettings->value());
	settings.setValue("CollisionCheck", m_actionCollisionCheck->isChecked());
	settings.setValue("BaseGrid", m_actionRenderBaseGrid->isChecked());
	settings.setValue("BoundingBoxForSelection", m_actionRenderBB->isChecked());
	settings.endGroup();
	settings.beginGroup("LogWidget");
	settings.setValue("ShowInfos", m_actionLogShowInfo->isChecked());
	settings.setValue("ShowWarnings", m_actionLogShowWarnings->isChecked());
	settings.setValue("ShowErrors", m_actionLogShowErrors->isChecked());
	settings.setValue("ShowDebugInfos", m_actionLogShowDebug->isChecked());	
	settings.endGroup();
}
