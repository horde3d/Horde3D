#include "HordeModelDialog.h"

#include "QHordeSceneEditorSettings.h"
#include "HordeSceneEditor.h"
#include "ColladaImporter.h"
#include "GLWidget.h"
#include "SceneFile.h"

HordeModelDialog::HordeModelDialog(const QString& targetPath, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : HordeFileDialog(H3DResTypes::SceneGraph, targetPath, parent, flags),
m_glWidget(0), m_oldScene(0), m_newScene(0), m_currentModel(0), m_cameraID(0)
{
	m_glFrame->setLayout(new QHBoxLayout());
	initModelViewer();
    m_importButton = new QPushButton(tr("Import Collada File"), this);
	m_importButton->setToolTip(tr("Let you import an existing collada file into the repository!"));
	m_importButton->setWhatsThis(
		tr("For the usage of your favorite modelling tool's data, you have to export the data to the collada file format.\n"
		"You can then convert the collada file using the Horde3D Collada Converter to the Horde3D specific file format.\n"
		"Using this button makes the conversion process more easy, since the execution of the Collada Converter is done\n" 
		"via a graphical user interface and the output of the Horde3D collada converter is\n"
		"automatically copied to the specifc directories of your repository."));
    HordeFileDialog::gridLayout->addWidget(m_importButton, 3, 0, 1, 1);

	connect(m_importButton, SIGNAL(clicked()), this, SLOT(importCollada()));
}


HordeModelDialog::~HordeModelDialog()
{
	if (m_newScene)		
	{
		h3dRemoveNode(m_newScene);
		while (!m_resources.isEmpty())
			h3dRemoveResource(m_resources.takeLast());
		h3dReleaseUnusedResources();
	}
	if (m_oldScene != 0)
		h3dSetNodeFlags(m_oldScene, 0, true );
}

void HordeModelDialog::itemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	if (current && m_type == H3DResTypes::SceneGraph)
	{
		setCursor(Qt::BusyCursor);
		QHordeSceneEditorSettings settings(this);
		settings.beginGroup("Repository");
		// Load model depending on the source (scene / repository)
		loadModel(current->text(), current->type() == 1);
		// indicate current selection
		m_file->setText(current->text());
		settings.endGroup();
		unsetCursor();
	}
	else // no valid selection
		HordeFileDialog::itemChanged(current, previous);
}

void HordeModelDialog::importCollada()
{	
	QHordeSceneEditorSettings settings;
	settings.beginGroup("Repository");
	QString defaultRepoPath = settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString();
	settings.endGroup();	
	if( !QDir( defaultRepoPath ).exists() )
	{
		QMessageBox::warning( this, tr("Error"), tr("The current repository path\n'%1'\nis not accessible.\nPlease reconfigure the repository path in the editor settings first!").arg( defaultRepoPath ) );
		return;
	}
	ColladaImporter importDlg;
	if (!importDlg.setColladaFiles()) return;
    
	importDlg.initImportPath( defaultRepoPath );
	importDlg.exec();	
	m_fileList->clear();
	if ( HordeSceneEditor::instance()->currentScene() )
		// Add all files already existing in the current scene
		populateList(m_sceneResourcePath.absolutePath(), m_sceneResourcePath.absolutePath(), m_currentFilter, false);
	settings.beginGroup("Repository");
	// Add all files existing in the repository
	populateList( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString(), QDir( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString() ), m_currentFilter, true);
	settings.endGroup();
}



void HordeModelDialog::initModelViewer()
{
	
	SceneFile* scene = HordeSceneEditor::instance()->currentScene();
	if( scene )			
	{
		// Prepare xmlview for engine log
		m_xmlView->setStyleSheet("QTextEdit#m_xmlView { background: black;  color: white }");
		// Get previously used scene
		m_oldScene = h3dGetNodeChild(H3DRootNode, 0);
		// Disable Root scene 
		h3dSetNodeFlags(m_oldScene, H3DNodeFlags::Inactive, true );	
		// Add new scene for model view
		m_newScene = h3dAddGroupNode(H3DRootNode, "ModelView");		

		// Get Pipeline from the last active camera
		int pipelineID = h3dGetNodeParamI(HordeSceneEditor::instance()->glContext()->activeCam(), H3DCamera::PipeResI);
		// add new camera that will be used within the modelview
		m_cameraID = h3dAddCameraNode(m_newScene, "Camera", pipelineID);
		h3dSetupCameraView(m_cameraID, 45, 4.0f/3.0f, 0.1f, 5000.f);
		// Load default light specified in the scene file
		QDomElement standardLight(scene->sceneFileDom().documentElement().namedItem("LightParameters").toElement());
		H3DRes lightMaterial = 0;
		if (standardLight.hasAttribute("material"))
			lightMaterial = h3dAddResource( H3DResTypes::Material, qPrintable(standardLight.attribute("material")), 0 );
		h3dutLoadResourcesFromDisk(".");		
		// Add Light to cam
		H3DNode light = h3dAddLightNode( 
			m_cameraID, 
			"ModelViewLight", 
			lightMaterial, 
			qPrintable(standardLight.attribute("lightingcontext", "LIGHTING")), 
			qPrintable(standardLight.attribute("shadowcontext", "SHADOWMAP")));
		h3dSetNodeTransform( light, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
		h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 200.0f );
		h3dSetNodeParamF( light, H3DLight::FovF, 0, 110.0f );
		h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 0 );
		h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 1.0f );
		h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 1.0f );
		h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 1.0f );
		// Important to create the glwidget after adding the shaders 
		// because only the first context can create new shader programs
		m_glWidget = new GLWidget(HordeSceneEditor::instance()->glContext(), m_glFrame);	
		// Set widget as shared widget
		HordeSceneEditor::instance()->glContext()->setFullScreen(false, m_glWidget);
		m_glWidget->setActiveCam(m_cameraID);
		m_glFrame->layout()->addWidget(m_glWidget);
		m_glWidget->setNavigationSpeed(10.0);
	}
	QHordeSceneEditorSettings settings;
	settings.beginGroup("Repository");
	m_currentFilter = "*.scene.xml";
	if ( HordeSceneEditor::instance()->currentScene() )
		populateList( m_sceneResourcePath.absolutePath(), m_sceneResourcePath, m_currentFilter, false);
	populateList( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString(), QDir( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString() ), m_currentFilter, true);
	m_stackedWidget->setCurrentWidget(m_glFrame);
}

void HordeModelDialog::loadModel(const QString& fileName, bool repoFile)
{
	QString scenePath = QDir::currentPath();
	if( repoFile ) // set repository pathes
	{
		QHordeSceneEditorSettings settings;
		settings.beginGroup("Repository");
		QDir::setCurrent( settings.value( "repositoryDir", DefaultRepoPath.absolutePath() ).toString() );
		settings.endGroup();
	}
	if (m_currentModel != 0)
	{
		h3dRemoveNode(m_currentModel);
		m_currentModel = 0;
	}
	else
		m_stackedWidget->setCurrentWidget(m_glFrame);
	// make the first glcontext current because the shared context is not allowed
	// to create new shader programs that may be loaded by the selected model
	HordeSceneEditor::instance()->glContext()->makeCurrent();
	// Add resource for model 
	H3DRes envRes = h3dAddResource( H3DResTypes::SceneGraph, qPrintable(fileName), 0 );
	// Load data
	if (envRes == 0 || !h3dutLoadResourcesFromDisk( "." )) // if loading failed
	{
		// Clear log
		m_xmlView->clear();
		int level;
		QString message; // Print log messages to xmlview
		while(!(message = h3dGetMessage(&level, 0)).isEmpty())
		{			
			switch( level )
			{
			case 1:
				m_xmlView->append("<span style=\"color: #FFCC00\">"+message+"</font>");
				break;
			case 2:
				m_xmlView->append("<span style=\"color: #EE1100\">"+message+"</font>");				
				break;
			default:
				m_xmlView->append("<span style=\"color: #C0C0C0\">"+message+"</font>");				
				break;
			}								
		}		
		// Show XML View Widget
		m_stackedWidget->setCurrentWidget(m_xmlView);	
		// Remove the added resource
		if( envRes ) h3dRemoveResource(envRes);		
		// Restore Path settings if the selected model was from the repository
		if( repoFile ) 
			QDir::setCurrent( scenePath );
		// Release unused resources within Horde3D
		h3dReleaseUnusedResources();
		return;
	}
	if (!m_resources.contains(envRes))
		m_resources.push_back(envRes);
	// Add to scene graph
	m_currentModel = h3dAddNodes( m_newScene, envRes );
	float minX, minY, minZ, maxX, maxY, maxZ;			
	// get bounding box to scale each model to the same size
	h3dGetNodeAABB(m_currentModel, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);
	//qDebug("Bounding: %.3f, %.3f, %.3f, %.3f, %.3f, %.3f", minX, minY, minZ, maxX, maxY, maxZ);
	float scale(8.0f / sqrtf(((maxX-minX)*(maxX-minX) + (maxY-minY)*(maxY-minY) + (maxZ-minZ)*(maxZ-minZ))));
	// scale model 
	h3dSetNodeTransform(m_currentModel, 0, 0, 0, 0, 0, 0, scale, scale, scale);	
	// get new bounding box
	h3dGetNodeAABB(m_currentModel, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);
	// adjust transformation centered to the camera
	const float* temp;
	h3dGetNodeTransMats(m_currentModel, &temp, 0);
	const_cast<float*>(temp)[12] -= (maxX+minX)/2;
	const_cast<float*>(temp)[13] -= (maxY+minY)/2;
	const_cast<float*>(temp)[14] += -10-maxZ;
	h3dSetNodeTransMat(m_currentModel, temp);
	// Rotate object if it is very small in the y-dimension
	if (abs(maxY-minY) < 0.5)
		h3dSetNodeTransform(m_currentModel, temp[12], temp[13], temp[14], 45, 0, 0, scale, scale, scale);
	// Rotate object if it is very small in the x-dimension
	if (abs(maxX-minX) < 0.5)
		h3dSetNodeTransform(m_currentModel, temp[12], temp[13], temp[14], 0, 45, 0, scale, scale, scale);
	// reset camera position
	h3dSetNodeTransform(m_cameraID, 0, 0, 0, 0, 0, 0, 1, 1, 1);	
	if( repoFile )
		QDir::setCurrent( scenePath );
}


QString HordeModelDialog::getModelFile(const QString& targetPath, QWidget* parent, const QString& caption)
{
	HordeModelDialog dlg(targetPath, parent);
	dlg.setWindowTitle(caption);
	if (dlg.exec() == QDialog::Accepted)
	{
		return dlg.fileName();
	}
	else
		return QString();
}
