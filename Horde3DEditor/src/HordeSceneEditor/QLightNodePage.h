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

#ifndef QLIGHTNODEPAGE_H_
#define QLIGHTNODEPAGE_H_

#include "ui_QLightNodePage.h"
#include <QWizard>

/**
 * Wizard Page for creating a QLightNode
 */
class QLightNodePage : public QWizardPage, protected Ui_QLightNodePage
{
	Q_OBJECT
public:
	QLightNodePage(QWidget* parent = 0);
	virtual ~QLightNodePage();

	void initializePage();
};
#endif
