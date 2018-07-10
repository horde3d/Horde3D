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

#include "ShaderComboBox.h"

#include <QtCore/QDir>
#include "HordeFileDialog.h"
#include "QHordeSceneEditorSettings.h"

ShaderComboBox::ShaderComboBox(QWidget* parent /*= 0*/) : QComboBox(parent)
{
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(currentChanged(int)));
}


ShaderComboBox::~ShaderComboBox()
{
}

void ShaderComboBox::init( const QString& resourcePath )
{
	clear();
	m_resourcePath = resourcePath;
	blockSignals(true);
	addShader(resourcePath);
	addItem(tr("Import from Repository"), QVariant((int) QVariant::UserType));
	if (count() == 1)
		setCurrentIndex(-1);
	blockSignals(false);
}

Shader ShaderComboBox::shader() const
{
	return Shader(currentText());
}

void ShaderComboBox::setShader(Shader shader)
{
	blockSignals(true);
	setCurrentIndex(findText(shader.FileName));	
	blockSignals(false);
	m_init = shader.FileName;
}

void ShaderComboBox::currentChanged(int index)
{
	if (itemData(index).isValid() && itemData(index) == QVariant((int)QVariant::UserType))
	{		
		QString newShader = HordeFileDialog::getResourceFile( H3DResTypes::Shader, m_resourcePath, this, tr("Select shader to import"));
		if (!newShader.isEmpty())
		{
			int index2 = findText(newShader);
			if (index2 == -1)
			{
				blockSignals(true);
				removeItem(index);
				addItem(newShader);
				addItem(tr("Import from Repository"), QVariant(QVariant::UserType));
				blockSignals(false);
				QHordeSceneEditorSettings settings;	
				setItemData( count() - 1, settings.value( "ImportEntryColor", QColor( 132, 255, 136 ) ), Qt::BackgroundColorRole );
				index2 = findText(newShader);				
			}
			setCurrentIndex(index2);
			return;
		}
		else
			setCurrentIndex(findText(m_init));
			return;
	}
	if (m_init != currentText())
	{
		emit shaderChanged();
		emit editFinished();
	}
}

void ShaderComboBox::addShader(const QString dir)
{
	QDir base(m_resourcePath);
	QList<QFileInfo> shaders = QDir(dir).entryInfoList(QStringList("*.shader"), QDir::Files | QDir::Readable);
	foreach(QFileInfo shader, shaders)
	{
		addItem(base.relativeFilePath(shader.absoluteFilePath()));
	}	
	QList<QFileInfo> shaderDirs = QDir(dir).entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
	foreach(QFileInfo directory, shaderDirs)
	{
		addShader(directory.absoluteFilePath());
	}
}
