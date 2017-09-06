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

#ifndef QCAMERANODEPAGE_H_
#define QCAMERANODEPAGE_H_

#include "ui_QCameraNodePage.h"
#include <QWizard>
#include <QtXml/qdom.h>

/**
 * Wizard Page for creating a QCameraNode
 */
class QCameraNodePage : public QWizardPage, protected Ui_QCameraNodePage
{
	Q_OBJECT
public:
	/**
	 * \brief Constructor
	 * @param parent widget that will be set as parent to the newly created page
	 */
	QCameraNodePage(/*const QDomElement& init = QDomElement(),*/ QWidget* parent = 0);
	/// Destructor
	virtual ~QCameraNodePage();

	void initializePage();

	bool isComplete() const;

private slots:
	void switchPage(bool asym);
	void importCamera();

private:
	QDomElement m_init;
};
#endif

