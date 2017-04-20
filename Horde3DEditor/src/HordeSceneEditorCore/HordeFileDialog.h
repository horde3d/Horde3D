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

#ifndef HORDEFILEDIALOG_H_
#define HORDEFILEDIALOG_H_

#include "ui_HordeFileDialog.h"

#include <QtCore/QFileInfo>
#include <QDomElement>
#include <QtCore/QDir>

#include <Horde3D.h>
#include <Horde3DUtils.h>

class Importer;

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_DECLARE_METATYPE(QFileInfo)
#endif


/**
 * The HordeFileDialog provides an user friendly dialog to select different Horde resources
 * 
 * It provides a preview for models and textures as well as an editor for shaders. Next to the
 * files available in the current scene directories it allows the import of files from an editor
 * repository where often used files can be stored. 
 * @see HordeSceneEditor
 */
class HordeFileDialog : public QDialog, protected Ui_HordeFileDialog
{
	Q_OBJECT
public:
	HordeFileDialog(H3DResTypes::List type, const QString& resourcePath, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~HordeFileDialog();

	QString fileName();

	/**
	 * Returns the selected pipeline filename relative to the currently set scene resource path
	 * @param resourceType
	 * @param targetPath 
	 * @param parent widget the HordeFileDialog will be child of
	 * @param caption string to be displayed as window caption	 
	 * @return QString filename of the selected pipeline
	 */
	static QString getResourceFile( H3DResTypes::List resourceType, const QString& targetPath, QWidget* parent, const QString& caption);

public slots:
	void accept();
	void reject();

protected slots:
	virtual void itemChanged(QListWidgetItem* current, QListWidgetItem* previous);

	
protected:

	void initTextureView(H3DResTypes::List type);
	void initShaderView();
	void initMaterialView();
	void initEffectView();
	void initPipelineView();

	void populateList(const QString& path, const QDir& baseDir, const QString& filter, bool repo);

	void loadXmlFile(const QFileInfo& fileName);
	void loadTexture(const QFileInfo& fileName);

	Importer*				m_importer;

	QString					m_currentFilter;

	QList<unsigned int>		m_resources;
	
	
	H3DResTypes::List		m_type;

    const QDir				DefaultRepoPath;

	QDir					m_sceneResourcePath;

};

#endif
