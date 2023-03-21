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

#include "SceneFilePage.h"

#include <QFileDialog>
#include <QtCore/QFileInfo>
#include <QMessageBox>
#include <QCompleter>
#include <QFileSystemModel>

SceneFilePage::SceneFilePage(QWidget* parent /*= 0*/) : QWizardPage(parent)
{
	setupUi(this);
	setTitle(tr("Create a new scene file"));
	setSubTitle(tr(
		"Specify new scene file name and folder. "
		"The editor's scene description file as well as the pipeline configuration"
		"will be placed in the specified folder."
		"All resource directories specified on the next page will be set relative to this directory."));
	registerField("scenefile*", m_sceneFile);
	registerField("scenepath*", m_scenePath);
	registerField("scenegraphfile*", m_sceneGraphFile );
	QCompleter *completer = new QCompleter(m_scenePath);
	completer->setModel(new QFileSystemModel(completer));
	m_scenePath->setCompleter(completer);
	connect(m_setScenePath, SIGNAL(clicked()), this, SLOT(setScenePath()));
}


SceneFilePage::~SceneFilePage()
{
}


void SceneFilePage::setScenePath()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select scene path"), QDir::currentPath());
	if (!dir.isEmpty())
		m_scenePath->setText(dir);
}

void SceneFilePage::initializePage()
{
	int i = 1;
	while (QFile::exists(QString("NewScene%1.scn").arg(i)))
		++i;
	m_sceneFile->setText(QString("NewScene%1.scn").arg(i));
	m_sceneGraphFile->setText( QString("models") + QDir::separator() + QString("NewScene%1.scene.xml").arg(i) );
	m_scenePath->setText(QDir::currentPath());
}

bool SceneFilePage::validatePage()
{
	QDir path(m_scenePath->text()); 
	path.makeAbsolute();
	if (!path.exists())
	{
		int result = QMessageBox::question(this, tr("Create directory?"), tr("The directory %1 does not exist!\n\nCreate it?").arg(path.absolutePath()), 
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape);
		switch (result)
		{
		case QMessageBox::Cancel:
			return false;
		case QMessageBox::Yes:
			path.mkpath(path.absolutePath());
		case QMessageBox::No:
			break;
		}
	}
	if (!m_sceneFile->text().endsWith(".scn", Qt::CaseInsensitive))
		m_sceneFile->setText(m_sceneFile->text() + ".scn");
	if(QFileInfo(QDir(m_scenePath->text()), m_sceneFile->text()).exists())
	{
		if (QMessageBox::question(
			this, 
			tr("Overwrite?"), 
			tr("There is already a scene file with the name %1\nin %2\n\nShould the wizard overwrite it?").arg(m_sceneFile->text()).arg(m_scenePath->text()),
			QMessageBox::Ok, QMessageBox::Cancel | QMessageBox::Cancel | QMessageBox::Default) != QMessageBox::Ok)
			return false;
	}
	QDir::setCurrent(m_scenePath->text());	
	QFileInfo sceneGraphFile( m_sceneGraphFile->text() );
	if( sceneGraphFile.isAbsolute() )
	{
		QMessageBox::information( this, tr("Error"), tr("The scene graph file must be relative to the scene path!") );
		return false;
	}
	if( !sceneGraphFile.dir().exists() )
		path.mkpath( sceneGraphFile.path() );
	if( !sceneGraphFile.exists() )
	{
		QFile sceneGraph( sceneGraphFile.absoluteFilePath() );
		if( !sceneGraph.open( QIODevice::WriteOnly ) )
		{
			QMessageBox::warning( this, tr("Error"), tr("The scene graph file '%1' does not exist and couldn't be created!").arg( m_sceneGraphFile->text() ) );
			return false;
		}
		sceneGraph.close();
	}
	return true;
}
