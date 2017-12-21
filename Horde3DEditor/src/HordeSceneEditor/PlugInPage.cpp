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

#include "PlugInPage.h"
#include "QHordeSceneEditorSettings.h"

#include <QtCore/QPluginLoader>
#include <QtCore/QDir>
#include <QMessageBox>
#include <QtCore/QLibrary>

#include "AttachmentPlugIn.h"

PlugInPage::PlugInPage(QWidget* parent /*= 0*/) : QWizardPage(parent)
{
	setupUi(this);
	setTitle("Attachment PlugIn Configuration");
	setSubTitle("Select the plugin you want to use for attachment control");
	registerField("plugin", m_plugIn, "currentText", SIGNAL(currentIndexChanged(int)));

}


PlugInPage::~PlugInPage()
{
}


void PlugInPage::initializePage()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("General");
	QDir pluginsDir(settings.value("PlugInPath", QApplication::applicationDirPath()+QDir::separator()+"plugins").toString());
	settings.endGroup();	
	QStringList pluginFileNames;	
	foreach (QString fileName, pluginsDir.entryList(QDir::Files)) 
	{		
		if (QLibrary::isLibrary(fileName))
		{
			//qDebug("%s ", fileName.toLatin1().constData());
			QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
			QObject *plugin = loader.instance();			
			if( plugin ) 
			{
				AttachmentPlugIn *service = qobject_cast<AttachmentPlugIn*>(plugin);
				if( service ) m_plugIn->addItem(service->plugInName());
			}			
			else
				QMessageBox::warning(0, "Error", loader.errorString());
		}
	}
}


