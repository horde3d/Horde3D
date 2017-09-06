#ifndef EXTTERRAINPLUGIN_H_
#define EXTTERRAINPLUGIN_H_

#include <QtCore/QObject>

#include "ExtSceneNodePlugIn.h"

class ExtTerrainPlugIn : public QObject, public ExtSceneNodePlugIn
{
	Q_OBJECT
	Q_INTERFACES(ExtSceneNodePlugIn)	
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "HordeSceneEditor.ExtSceneNodePlugIn/1.0")
#endif
public:
	ExtTerrainPlugIn(QObject* parent = 0);
	virtual ~ExtTerrainPlugIn();

	QString extensionName() const { return "Terrain"; }

	void setPlugInManager(PlugInManager* factory);

	void registerLuaFunctions(lua_State* lua);

private:
	PlugInManager*	m_factory;
};
#endif
