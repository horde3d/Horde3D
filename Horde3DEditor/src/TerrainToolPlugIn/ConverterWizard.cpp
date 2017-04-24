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

#include "ConverterWizard.h"

#include <QImageWriter>
#include <QImageReader>
#include <QMessageBox>
#include <QFileDialog>

ConverterWizard::ConverterWizard(QWidget* parent /*= 0*/) : QWizardPage(parent)
{
	setupUi(this);
	QList<QByteArray> formats = QImageWriter::supportedImageFormats();
	foreach(QByteArray format, formats)
	{		
		if (format.toUpper() == "PSD" ||
			format.toUpper() == "PNG" ||
			format.toUpper() == "TGA")
			m_format->addItem(QString(format));
	}	
	setTitle(tr("Texture Converter for Heightmaps"));
	setSubTitle(tr("Using this wizard you can convert 8bit textures or 16bit PGM files to a "
		"32bit image that can be used by the standard Horde3D Terrain Node Extension.\n"
		"The converted 32bit image will contain the normal map data as well as the heightmap data and "
		"will be saved relative to the scene's texture directory."));
	registerField("heightmap*", m_heightMap);
	registerField("target*", m_target);
	registerField("format", m_format, "currentText");
	registerField("filter_radius", m_filterRadius);
	connect(m_setHeightMap, SIGNAL(clicked()), this, SLOT(setHeightMap()));
}


ConverterWizard::~ConverterWizard()
{
}

bool ConverterWizard::validatePage()
{
	QImageReader reader(m_heightMap->text());
	if ( !reader.canRead() )
		QMessageBox::warning(this, tr("Error"), tr("Can not read heightmap %1 !\n%2").arg(m_heightMap->text()).arg(reader.errorString()));
	else
	{
		QSize size = reader.size();
		if (size.isValid() && ( size.width() != size.height() || ( ( size.width() & ( size.width() - 1 ) ) != 0 ) ) )
			QMessageBox::warning(this, tr("Error"), tr("The heightmap must have equal width and height and the side length must be a power of two! e.g. 1024x1024"));
		else
			return true;
	}
	return false;
}

void ConverterWizard::setHeightMap()
{
	QString filter = tr("Image files (");
	QList<QByteArray> formats = QImageReader::supportedImageFormats();
	foreach(QByteArray format, formats)
		filter += "*." + QString(format)+" ";
	filter+= ")";	
	filter+= tr(";;All files (*.*)");
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select heightmap"), QDir::currentPath(), filter);
	if (fileName.isEmpty())
		return;
	QImageReader reader(fileName);
	QSize size = reader.size();
	if (size.isValid() && ( size.width() != size.height() || ( ( size.width() & ( size.width() - 1 ) ) != 0 ) ) )
		QMessageBox::warning(this, tr("Error"), tr("The heightmap must have equal width and height and the side length must be a power of two! e.g. 1024x1024"));
	else
		m_heightMap->setText(fileName);
}
