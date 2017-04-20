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

#include "PlugInManager.h"

#include "AttachmentPlugIn.h"
#include "ExtSceneNodePlugIn.h"
#include "ToolPlugIn.h"
#include "QHordeSceneEditorSettings.h"

#include <QtCore/QLibrary>
#include <QtCore/QPluginLoader>
#include <QtCore/QDir>
#include <QApplication>

#include "SceneFile.h"

extern "C"
{
    #include <lua.h>
}

PlugInManager::PlugInManager(QObject* parent /*= 0*/) : QObject(parent), m_propertyEditor(0), m_currentAttachmentPlugIn(0)
{
}

PlugInManager::~PlugInManager()
{
	if (m_currentAttachmentPlugIn) m_currentAttachmentPlugIn->init(0, m_propertyEditor);
	for( int i = 0; i < m_loaders.size(); ++i )
	{
		m_loaders[i]->unload();
		delete m_loaders[i];
	}
}

void PlugInManager::init()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("General");
	QDir pluginsDir(settings.value("PlugInPath", QApplication::applicationDirPath()+QDir::separator()+"plugins").toString());
	settings.endGroup();	
	QStringList pluginFileNames;	
	foreach (QString fileName, pluginsDir.entryList(QDir::Files)) 
	{		
		if (QLibrary::isLibrary(fileName))
		{
			QPluginLoader* loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
			QObject *plugin = loader->instance();			
			if (plugin) 
			{
				AttachmentPlugIn *attachmentPlugIn = qobject_cast<AttachmentPlugIn*>(plugin);
				if( attachmentPlugIn ) m_attachmentPluginRegistry[attachmentPlugIn->plugInName()] = attachmentPlugIn;

				ExtSceneNodePlugIn *extension = qobject_cast<ExtSceneNodePlugIn*>(plugin);
				if( extension ) extension->setPlugInManager( this );

				ToolPlugIn *tool = qobject_cast<ToolPlugIn*>(plugin);
				if( tool )
				{
					emit toolRegistered(tool->actions());
					m_toolPlugIns.push_back(tool);
				}

				if( !attachmentPlugIn && !extension && !tool )
				{
					loader->unload();
					delete loader;
				}
				else
					m_loaders.push_back( loader );
			}			
			else
			{
				qWarning("%s",qPrintable(loader->errorString()));
				delete loader;
			}
		}
	}	
}

void PlugInManager::setSceneFile(SceneFile* scene)
{
	// Notify previous plugin that another scene will be loaded
	if (m_currentAttachmentPlugIn) 
	{
		m_currentAttachmentPlugIn->init(0, m_propertyEditor);
		disconnect(m_currentAttachmentPlugIn, SIGNAL(modified(bool)), this, SIGNAL(sceneModified(bool)));
	}
	m_currentAttachmentPlugIn = 0;

	if (scene)
	{
		// Make the instance of this pluginmanager available in the scene
		scene->setPlugInManager(this);
		QMap<QString, AttachmentPlugIn*>::iterator iter = m_attachmentPluginRegistry.find(scene->attachmentPlugInName());
		if( iter != m_attachmentPluginRegistry.end() )
		{
			m_currentAttachmentPlugIn = iter.value();
			m_currentAttachmentPlugIn->init(scene, m_propertyEditor);
			connect(m_currentAttachmentPlugIn, SIGNAL(modified(bool)), this, SIGNAL(sceneModified(bool)));
		}
	}
	foreach(ToolPlugIn* plugin, m_toolPlugIns)
		plugin->setSceneFile(scene);
}

void PlugInManager::registerSceneNode(const QString& name, SceneNodeLoadFunc loadFunc, NodeCreateFunc createFunc)
{
	unregisterSceneNode(name);
	if( loadFunc ) m_sceneNodeLoadRegistry.insert(name, loadFunc);
	if( createFunc ) 
	{
		m_nodeCreateRegistry.insert(name, createFunc );
		emit nodeRegistered(name, SCENENODE);	
	}
}

void PlugInManager::unregisterSceneNode(const QString& name )
{
	QMap<QString, SceneNodeLoadFunc>::iterator loadIter = m_sceneNodeLoadRegistry.find(name);
	if( loadIter != m_sceneNodeLoadRegistry.end() ) m_sceneNodeLoadRegistry.erase(loadIter);
	QMap<QString, NodeCreateFunc>::iterator createIter = m_nodeCreateRegistry.find(name);
	if( createIter != m_nodeCreateRegistry.end() ) 
	{
		m_nodeCreateRegistry.erase(createIter);
		emit nodeUnregistered( name, SCENENODE );
	}
}

QSceneNode* PlugInManager::loadSceneNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent) const
{
	QSceneNode* node = 0;
	QMap<QString, SceneNodeLoadFunc>::const_iterator iter = m_sceneNodeLoadRegistry.find(xmlNode.tagName());
	if (iter != m_sceneNodeLoadRegistry.end()) node = (iter.value())(xmlNode, row, model, parent);
	return node;
}

void PlugInManager::registerExtraNode(const QString& name, NodeLoadFunc loadFunc, NodeCreateFunc createFunc)
{
	unregisterExtraNode(name);
	if( loadFunc ) m_nodeLoadRegistry.insert(name, loadFunc);
	if( createFunc ) 
	{
		m_nodeCreateRegistry.insert(name, createFunc );
		emit nodeRegistered(name, EXTRANODE);	
	}
}

void PlugInManager::unregisterExtraNode(const QString& name )
{
	QMap<QString, NodeLoadFunc>::iterator loadIter = m_nodeLoadRegistry.find(name);
	if( loadIter != m_nodeLoadRegistry.end() ) m_nodeLoadRegistry.erase(loadIter);
	QMap<QString, NodeCreateFunc>::iterator createIter = m_nodeCreateRegistry.find(name);
	if( createIter != m_nodeCreateRegistry.end() ) 
	{
		m_nodeCreateRegistry.erase(createIter);
		emit nodeUnregistered( name, EXTRANODE );
	}
}

QXmlTreeNode* PlugInManager::loadExtraNode(const QDomElement& xmlNode, int row, QXmlTreeModel* model, QXmlTreeNode* parent) const
{
	QXmlTreeNode* node = 0;
	QMap<QString, NodeLoadFunc>::const_iterator iter = m_nodeLoadRegistry.find(xmlNode.tagName());
	if (iter != m_nodeLoadRegistry.end()) node = (iter.value())(xmlNode, row, model, parent);
	return node;
}

QDomElement PlugInManager::createNode(const QString& name, QWidget* parent /*=0*/)
{
	QDomElement node;
	QMap<QString, NodeCreateFunc>::const_iterator iter = m_nodeCreateRegistry.find(name);
	if (iter != m_nodeCreateRegistry.end()) node = (iter.value())(parent);
	return node;
}

QStringList PlugInManager::sceneNodeNames()
{
	return m_sceneNodeLoadRegistry.keys();
}

void PlugInManager::registerLuaFunctions( lua_State *L )
{
	if( m_currentAttachmentPlugIn )
		m_currentAttachmentPlugIn->registerLuaFunctions( L );

	// TODO register extension plugins
}

void PlugInManager::setCurrentNode(QXmlTreeNode* node)
{
	if( m_currentAttachmentPlugIn )
		m_currentAttachmentPlugIn->setCurrentNode(node);
}

