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
#include "FindDialog.h"

FindDialog* FindDialog::m_instance = 0;

FindDialog* FindDialog::showFindDialog(QWidget* parent /*= 0*/)
{
	if (m_instance == 0)
		m_instance = new FindDialog(parent);
	m_instance->setVisible(true);
	return m_instance;
}

void FindDialog::releaseDialog()
{
	delete m_instance;
	m_instance = 0;
}

FindDialog::FindDialog(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QDialog(parent, flags)
{
	setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
}


FindDialog::~FindDialog()
{
	m_instance = 0;
}

void FindDialog::accept()
{
	emit find(m_searchText->text());
}



