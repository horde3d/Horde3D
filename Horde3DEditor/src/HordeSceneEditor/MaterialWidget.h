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

#ifndef MATERIALWIDGET_H_
#define MATERIALWIDGET_H_

#include "ui_MaterialWidget.h"

#include <QtXml/qdom.h>
#include <ShaderData.h>

#include <Horde3D.h>

class QFileSystemWatcher;

/**
 * Class for manipulating material files 
 */
class MaterialWidget : public QWidget, protected Ui_MaterialWidget
{
	Q_OBJECT
public:
	MaterialWidget(QWidget* parent = 0, Qt::WindowFlags flags = (Qt::WindowFlags) 0);
	virtual ~MaterialWidget();
	
	void init();

	bool isModified() {return m_saveButton->isEnabled();}
	void closeMaterial();

public slots:
	void setCurrentMaterial(const QString& materialFileName);
	void save();

signals:
	void modified(bool);

private slots:
	void editShader();
	void initValues();
	void texUnitChanged(int);
	void texUnitDataChanged();
	void uniformChanged(int);
	void uniformDataChanged();
	void linkChanged(int index);
	void shaderChanged();
	void classChanged();
	void flagsChanged( int index, bool checked );
	void syncWithShader();

private:
	void release();

	ShaderData*			m_shaderData;
	QString				m_currentMaterialFile;
	QDomDocument		m_materialXml;
	QString				m_resourcePath;
	H3DRes				m_matHandle;
	H3DRes				m_shaderHandle;
	QFileSystemWatcher* m_shaderWatcher;

};
#endif

