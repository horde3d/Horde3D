// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the GameEngine developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// The GameEngine is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The GameEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************


// ****************************************************************************************
//
// GameEngine Horde3D Editor Plugin of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#include "PathPage.h"

#include <QtCore/qsignalmapper.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>

PathPage::PathPage(QWidget* parent /*= 0*/) : QWizardPage(parent)
{
	setupUi(this);
		m_mapper = new QSignalMapper(this);
	setTitle(tr("Configure GameEngine Directory Settings"));
	setSubTitle(tr("The resource directories will be saved relative to the scenefile where possible. If a directory does not exist already, it will be created."));

	connect(m_setMediaDir, SIGNAL(clicked()), m_mapper, SLOT(map()));
	m_mapper->setMapping(m_setMediaDir, m_mediaDir);

	connect(m_setScriptDir, SIGNAL(clicked()), m_mapper, SLOT(map()));
	m_mapper->setMapping(m_setScriptDir, m_scriptDir);

	connect(m_mapper, SIGNAL(mapped(QWidget*)), this, SLOT(setDirectory(QWidget*)));

	registerField("mediadir", m_mediaDir);
	registerField("scriptdir", m_scriptDir);
}


PathPage::~PathPage()
{
}



void PathPage::initializePage()
{
	m_mediaDir->setText(QString(".")+QDir::separator() + "media");
	m_scriptDir->setText(QString(".")+QDir::separator() + "script");
}

void PathPage::setDirectories( const QString& mediaDir, const QString& scriptDir )
{
	if( !mediaDir.isEmpty() ) m_mediaDir->setText(mediaDir);
	if( !scriptDir.isEmpty() ) m_scriptDir->setText(scriptDir);
}

void PathPage::setDirectory(QWidget *edit)
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::currentPath());
	if (!directory.isEmpty())
	{		
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(edit);
		if (lineEdit)
			lineEdit->setText(QDir::current().relativeFilePath(directory));
	}	
}

bool PathPage::validatePage()
{	
	QDir mediaDir(m_mediaDir->text());
	QDir scriptDir(m_scriptDir->text());

	if (!mediaDir.exists() || !scriptDir.exists())
	{
		int result = QMessageBox::question(
			this, 
			tr("Create directories"), 
			tr("One or more directories does not exist yet!\n\nShould these directories be created?"), 
			QMessageBox::Yes | QMessageBox::Default, 
			QMessageBox::No, 
			QMessageBox::Cancel | QMessageBox::Escape);
		if (result == QMessageBox::No)
			return true;
		if (result == QMessageBox::Cancel)
			return false;
	}
	if (!mediaDir.exists() && !mediaDir.mkpath(mediaDir.absolutePath()))
	{
		QMessageBox::warning(0, tr("Error"), tr("An error occured when trying to create %1\n").arg(mediaDir.absolutePath()));
		return false;
	}
	if (!scriptDir.exists() && !scriptDir.mkpath(scriptDir.absolutePath()))
	{
		QMessageBox::warning(0, tr("Error"), tr("An error occured when trying to create %1\n").arg(scriptDir.absolutePath()));
		return false;
	}
	return true;
}
