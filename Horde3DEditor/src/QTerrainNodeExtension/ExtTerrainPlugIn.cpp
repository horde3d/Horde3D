#include "ExtTerrainPlugIn.h"

#include "PlugInManager.h"
#include "QTerrainNode.h"

#include <QtCore/qplugin.h>

ExtTerrainPlugIn::ExtTerrainPlugIn(QObject* parent /*= 0*/) : QObject(parent)
{
	
}


ExtTerrainPlugIn::~ExtTerrainPlugIn()
{
	m_factory->unregisterSceneNode( extensionName() );
}

void ExtTerrainPlugIn::setPlugInManager(PlugInManager *factory)
{
	m_factory = factory;
	factory->registerSceneNode( extensionName(), QTerrainNode::loadNode, QTerrainNode::createNode );
}

void ExtTerrainPlugIn::registerLuaFunctions(lua_State *lua)
{
}