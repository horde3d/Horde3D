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
#include "QEmitterNodePage.h"

#include "HordeSceneEditor.h"

#include <Horde3DUtils.h>

QEmitterNodePage::QEmitterNodePage(QWidget* parent /*= 0*/) : QWizardPage(parent)
{
	setupUi(this);
	setTitle(tr("Add new emitter"));
	setSubTitle(tr("Create a new particle source for the current scene"));
	registerField("name*", m_name);
	registerField("material*", m_material, "materialFile", SIGNAL(materialChanged()));		
	registerField("particleEffect", m_effect, "effectFile", SIGNAL(effectChanged()));	
	registerField("maxCount", m_maxCount);
	registerField("respawnCount", m_respawnCount);
	registerField("emissionRate", m_emissionRate);
}


QEmitterNodePage::~QEmitterNodePage()
{
}

void QEmitterNodePage::initializePage()
{
	m_material->init( QDir::currentPath() );
	m_effect->init( QDir::currentPath() );		
}

