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

#include "QGroupNodePage.h"


QGroupNodePage::QGroupNodePage(QWidget* parent /*= 0*/) : QWizardPage(parent)
{
	setupUi(this);
	setTitle(tr("Add new group node"));
	setSubTitle(tr("A group node can contain different child nodes that will be affected by the group node's transformation."
		"Additionally a group node can be the scene graph representation for custom attachment nodes."));
	registerField("name*", m_name);
}


QGroupNodePage::~QGroupNodePage()
{
}



