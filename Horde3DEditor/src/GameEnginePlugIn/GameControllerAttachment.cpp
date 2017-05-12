// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the GameEngine developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// The GameEngine is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The GameEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************


// ****************************************************************************************
//
// GameEngine Horde3D Editor Plugin of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#include "GameControllerAttachment.h"

#include "PathPage.h"
#include "GameControllerWidget.h"
#include "AttachmentTreeModel.h"

#include "ExtraTreeModel.h"
#include "SceneFile.h"
#include "PlugInManager.h"
//#include "CustomAttachmentTypes.h"

#include "QExtraNode.h"
#include "QGameEntityNode.h"

#include <QPropertyEditor/QPropertyEditorWidget.h>

#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEngine_BulletPhysics.h>
#include <GameEngine/GameEngine_Sound.h>
#include <GameEngine/GameEngine_SceneGraph.h>

#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qdir.h>
#include <QtWidgets/QWizard>
#include <QtCore/qplugin.h>

extern "C"
{
	#include "Lua/lua.h"
}

#include <Horde3D/Horde3DUtils.h>

GameControllerAttachment::GameControllerAttachment(QObject* parent /*= 0*/) : AttachmentPlugIn(parent)
{
	m_widget = new GameControllerWidget();
	m_widget->setVisible(false);
	connect(m_widget, SIGNAL(modified(bool)), this, SLOT(emitModified(bool)));
	connect(this, SIGNAL(updateFinished()), m_widget, SLOT(emitUpdateFinished()));
	//CustomAttachmentTypes::registerTypes();
}

GameControllerAttachment::~GameControllerAttachment() 
{
	delete m_widget;
	GameEngine::release();
}

void GameControllerAttachment::emitModified(bool changed)
{
	emit modified(changed);
}

QWidget* GameControllerAttachment::configurationWidget()
{
	return m_widget;
}

void GameControllerAttachment::init(SceneFile* file, QPropertyEditorWidget* widget)
{
	if (file)
	{
		 //Horde3D specific initialization has been done by the HordeSceneEditor already
		 //so we only have to initialize directories and the attachment callback
		GameEngine::setSoundResourceDirectory( qPrintable(file->sceneFileDom().documentElement().firstChildElement("EnginePath").attribute("mediapath")) );

		file->pluginManager()->registerExtraNode("GameEntity", QGameEntityNode::loadNode, QGameEntityNode::createNode);
		//if( widget ) widget->registerCustomPropertyCB(CustomAttachmentTypes::createCustomProperty);

		GameEngine::init();
		m_widget->init();
	}
	else
	{ 
		if( m_sceneFile )
		{
			m_sceneFile->pluginManager()->unregisterExtraNode("GameEntity");
		}
		//if( widget ) widget->unregisterCustomPropertyCB(CustomAttachmentTypes::createCustomProperty);
		GameEngine::release();
		m_widget->release();
	}
	m_sceneFile = file;
}

void GameControllerAttachment::setCurrentNode(QXmlTreeNode* parentNode)
{	
	if( parentNode && parentNode->property("__AttachmentModel").isValid() && parentNode->property("__AttachmentModel").value<void*>() != 0)
		m_widget->setCurrentNode(parentNode);
	else	
		m_widget->setCurrentNode(0);			
	m_currentNode = parentNode;
}

void GameControllerAttachment::update()
{
	GameEngine::update();
	emit updateFinished();
}

void GameControllerAttachment::render(int activeCameraID)
{
	GameEngine::setActiveCamera( activeCameraID );
}

void GameControllerAttachment::initNodeAttachment(QXmlTreeNode* sceneNode)
{	
	Q_ASSERT(!sceneNode->xmlNode().firstChildElement("Attachment").isNull());

	QString entityName = sceneNode->xmlNode().firstChildElement("Attachment").attribute("name");
	// If there is already an entity with this name we have to rename this one
	while (GameEngine::entityWorldID(qPrintable(entityName)) != 0)
		entityName = entityName + QString::number(sceneNode->property("ID").toInt());
	if (entityName != sceneNode->xmlNode().firstChildElement("Attachment").attribute("name"))
		sceneNode->xmlNode().firstChildElement("Attachment").setAttribute("name", entityName);
	// Create the Entity within the GameEngine
	unsigned int entityID = GameEngine::createGameEntity( qPrintable(QString("<GameEntity type=\"%1\" name=\"%2\"/>").arg(plugInName()).arg(entityName)) );
	// Add Scene Graph Component
	GameEngine::setComponentData( entityID, "Horde3D", qPrintable( QString("<Horde3D id=%1 />").arg(sceneNode->property("ID").toInt()) ) ); 
	// Create a treemodel for the Attachment Widget
	AttachmentTreeModel* model = new AttachmentTreeModel(sceneNode, sceneNode->xmlNode().firstChildElement("Attachment"));
	// Store treemodel as dynamic property
	sceneNode->setProperty("__AttachmentModel", QVariant::fromValue<void*>(model));	
}

void GameControllerAttachment::destroyNodeAttachment(QXmlTreeNode* sceneNode)
{
	unsigned int entityID = GameEngine::entityWorldID( qPrintable(sceneNode->xmlNode().firstChildElement("Attachment").attribute("name")) );
	GameEngine::removeGameEntity(entityID);
}

void GameControllerAttachment::createNodeAttachment()
{	
	Q_ASSERT(m_currentNode != 0);	
	QDomElement node = m_currentNode->xmlNode().insertBefore(QDomDocument().createElement("Attachment"), QDomNode()).toElement();
	node.setAttribute("type", plugInName());
	node.setAttribute("name", m_currentNode->property("Name").toString() + "_" + m_currentNode->property("ID").toString());
	initNodeAttachment(m_currentNode);
	setCurrentNode(m_currentNode);
}

void GameControllerAttachment::removeNodeAttachment()
{
	QDomElement node = m_currentNode->xmlNode().firstChildElement("Attachment");
	unsigned int entityID = GameEngine::entityWorldID( qPrintable(node.attribute("name")) );
	// Reset scene graph component to avoid removement of Horde's scenegraph node
	GameEngine::setEntitySceneGraphID( entityID, 0 );
	m_currentNode->xmlNode().removeChild(node);	
	if( m_currentNode->property("__AttachmentModel").isValid() )
	{
		delete static_cast<AttachmentTreeModel*>(m_currentNode->property("__AttachmentModel").value<void*>());
		m_currentNode->setProperty("__AttachmentModel", QVariant::fromValue<void*>(0));
		GameEngine::removeGameEntity( entityID );
	}
	setCurrentNode(m_currentNode);
}

QXmlTreeModel* GameControllerAttachment::initExtras( const QDomElement &extraNode, QObject* parent)
{
	ExtraTreeModel* model = new ExtraTreeModel(m_sceneFile->pluginManager(), extraNode, parent);	
	return model;
}

void GameControllerAttachment::sceneFileConfig()
{
	QDomDocument sceneFile(m_sceneFile->sceneFileDom());
	QDomElement pathNode(sceneFile.documentElement().firstChildElement("EnginePath"));
	// Create a wizard for the configuration of the directories
	QWizard wizard;
	PathPage* page = new PathPage(&wizard);	
	page->setDirectories( 
		pathNode.attribute("mediapath"), 
		pathNode.attribute("scriptpath")
	);
	wizard.addPage(page);
	if (wizard.exec() == QDialog::Accepted)
	{
		pathNode.setAttribute("mediapath", wizard.field("mediadir").toString());
		pathNode.setAttribute("scriptpath", wizard.field("scriptdir").toString());
	}
}

void GameControllerAttachment::registerLuaFunctions(lua_State* lua)
{
	GameEngine::registerLuaStack(lua);
}

QFileInfoList GameControllerAttachment::findReferences(const QDomElement &node) const
{
	QFileInfoList references;	
	if (node.tagName() == "Sound3D" && node.hasAttribute("file"))
	{
		QFileInfo file(node.attribute("file"));
		references.append(file);
	}
	if( node.tagName() == "StaticAnimation" && node.hasAttribute("file"))
	{
		QFileInfo file(node.attribute("file"));
		references.append(file);
	}
	QDomNodeList children = node.childNodes();
	for (int i=0; i<children.size(); ++i)
		references << findReferences(children.at(i).toElement());
	return references;
}