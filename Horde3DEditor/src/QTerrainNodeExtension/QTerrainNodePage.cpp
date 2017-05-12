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
#include "QTerrainNodePage.h"

#include <QtCore/QDir>

QTerrainNodePage::QTerrainNodePage(QWidget* parent /*= 0*/) : QWizardPage(parent)
{
	setupUi(this);
	setTitle(tr("Add new terrain"));
	setSubTitle(tr("Create a new terrain for the current scene"));
	registerField("name*", m_name);
	registerField("heightmap", m_heightMap, "textureFile", SIGNAL(textureChanged()));
	registerField("material", m_material, "materialFile", SIGNAL(materialChanged()));	
}


QTerrainNodePage::~QTerrainNodePage()
{
}

void QTerrainNodePage::initializePage()
{
	m_material->init( QDir::currentPath() );	
	m_heightMap->init( QDir::currentPath() );
}

