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

#ifndef TERRAINTOOLPLUGIN_H_
#define TERRAINTOOLPLUGIN_H_

#include <QtCore/QObject>

#include "../HordeSceneEditorCore/ToolPlugIn.h"

class QProgressDialog;

class TerrainToolPlugIn : public QObject, public ToolPlugIn
{
	Q_OBJECT
	Q_INTERFACES(ToolPlugIn)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	Q_PLUGIN_METADATA(IID "HordeSceneEditor.ToolPlugIn/1.0")
#endif
public:
	TerrainToolPlugIn(QObject* parent = 0);
	virtual ~TerrainToolPlugIn();

	QString plugInName() const;

	QList<QAction*> actions() const;

	void setSceneFile(SceneFile* sceneFile);

private slots:
	void heightMapConverter();

private:
	QImage readPGM( const QString& fileName, QProgressDialog* dlg = 0 );
	QImage smoothInputImage( const QImage& image, QProgressDialog* dlg = 0 );
	void calculateNormalMap( QImage& image, QProgressDialog* dlg = 0 );

	QList<QAction*>		m_actions;
	SceneFile*			m_scene;
	int					m_filterRadius;
};
#endif
