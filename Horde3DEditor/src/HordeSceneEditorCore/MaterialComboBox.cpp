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

#include "MaterialComboBox.h"

#include <QtCore/QDir>
#include "HordeFileDialog.h"
#include "QHordeSceneEditorSettings.h"

MaterialComboBox::MaterialComboBox(QWidget* parent /*= 0*/) : QComboBox(parent)
{	
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(currentChanged(int)));	
}


MaterialComboBox::~MaterialComboBox()
{
}

void MaterialComboBox::init(const QString& resourcePath )
{
	clear();	
	addItem(tr("No material"), QVariant((int) -1));
	m_resourcePath = resourcePath;
	blockSignals(true);
	if (!resourcePath.isEmpty())
		addMaterials(resourcePath, resourcePath);
	addItem(tr("Clone/Import from Repository"), QVariant((int) QVariant::UserType));	
	QHordeSceneEditorSettings settings;	
	setItemData( count() - 1, settings.value( "ImportEntryColor", QColor( 132, 255, 136 ) ), Qt::BackgroundColorRole );
	if (count() == 1)
		setCurrentIndex(-1);
	blockSignals(false);
}

QString MaterialComboBox::materialFile() const
{
	if (currentIndex() == -1 || itemData(currentIndex(), Qt::UserRole).toInt() == -1)
		return QString();
	else
		return currentText();
}

void MaterialComboBox::setMaterialFile(const QString& filename)
{
	setMaterial(Material(filename));
}

Material MaterialComboBox::material() const
{
	return Material(materialFile());
}

void MaterialComboBox::setMaterial(Material material)
{
	blockSignals(true);
	if ( material.FileName.isEmpty() )
		setCurrentIndex(0); // the no material entry
	else
		setCurrentIndex(findText(material.FileName));
	blockSignals(false);
	m_init = material.FileName;
}

void MaterialComboBox::currentChanged(int index)
{
	if (itemData(index).isValid() && itemData(index) == QVariant((int)QVariant::UserType))
	{
		QString newMaterial = HordeFileDialog::getResourceFile( H3DResTypes::Material, m_resourcePath, this, tr("Select material to clone/import"));
		if (!newMaterial.isEmpty())
		{
			int index2 = findText(newMaterial);			
			if (index2 == -1)
			{
				blockSignals(true);
				removeItem(index);
				addItem(newMaterial);
				addItem(tr("Clone/Import from Repository"), QVariant(QVariant::UserType));				
				blockSignals(false);
				QHordeSceneEditorSettings settings;	
				setItemData( count() - 1, settings.value( "ImportEntryColor", QColor( 132, 255, 136 ) ), Qt::BackgroundColorRole );
				index2 = findText(newMaterial);				
			}
			setCurrentIndex(index2);
			return;
		}
		else
		{
			setCurrentIndex(findText(m_init));
			return;
		}
	}
	if (m_init != currentText())
	{
		emit editFinished();
		emit materialChanged();
	}
}

void MaterialComboBox::addMaterials(const QDir& base, const QString dir)
{	
	QList<QFileInfo> materials = QDir(dir).entryInfoList(QStringList("*.material.xml"), QDir::Files | QDir::Readable);
	foreach(QFileInfo material, materials)
	{
		addItem(base.relativeFilePath(material.absoluteFilePath()));
	}	
	QList<QFileInfo> materialDirs = QDir(dir).entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
	foreach(QFileInfo directory, materialDirs)
	{
		addMaterials(base, directory.absoluteFilePath());
	}
}
