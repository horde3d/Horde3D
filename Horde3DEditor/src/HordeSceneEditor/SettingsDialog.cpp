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

#include "SettingsDialog.h"
#include "QHordeSceneEditorSettings.h"

#include <QtCore/QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCore/QSignalMapper>

SettingsDialog::SettingsDialog(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QDialog(parent, flags), m_restart(false)
{
	setupUi(this);
	m_mapper = new QSignalMapper(this);
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("General");
	m_shaderEditor->setText(settings.value("ShaderEditor", "notepad.exe").toString());
	m_plugInPath->setText( QDir::toNativeSeparators( settings.value("PlugInPath", QApplication::applicationDirPath()+QDir::separator()+"plugins").toString() ) );
	m_undoSteps->setValue(settings.value("UndoStackSize", 250).toInt());
	m_glClear->setChecked( settings.value("glClear", true ).toBool() );
	switch(settings.value("SelectButton", Qt::RightButton).toInt())
	{
	case Qt::LeftButton:
		m_selectButton->setCurrentIndex(0);
		break;
	case Qt::RightButton:
		m_selectButton->setCurrentIndex(1);
		break;
	case Qt::MidButton:
		m_selectButton->setCurrentIndex(2);
		break;
	}
	switch(settings.value("CameraMoveButton", Qt::LeftButton).toInt())
	{
	case Qt::LeftButton:
		m_cameraMoveButton->setCurrentIndex(0);
		break;
	case Qt::RightButton:
		m_cameraMoveButton->setCurrentIndex(1);
		break;
	case Qt::MidButton:
		m_cameraMoveButton->setCurrentIndex(2);
		break;
	}
	switch(settings.value("ResetSelectButton", Qt::MidButton).toInt())
	{
	case Qt::LeftButton:
		m_resetSelectButton->setCurrentIndex(0);
		break;
	case Qt::RightButton:
		m_resetSelectButton->setCurrentIndex(1);
		break;
	case Qt::MidButton:
		m_resetSelectButton->setCurrentIndex(2);
		break;
	}
	connect(m_selectButton, SIGNAL(currentIndexChanged(int)), this, SLOT(updateButtons()));
	connect(m_resetSelectButton, SIGNAL(currentIndexChanged(int)), this, SLOT(updateButtons()));
	connect(m_cameraMoveButton, SIGNAL(currentIndexChanged(int)), this, SLOT(updateButtons()));

	QStringList styles = QDir(QApplication::applicationDirPath()+QDir::separator()+"Styles").entryList(QStringList("*.qss"), QDir::Files | QDir::Readable);
	foreach(QString style, styles)
		m_styleCombo->addItem(style);
	m_styleCombo->setCurrentIndex(m_styleCombo->findText(settings.value("Style", "default.qss").toString()));
	settings.endGroup();

	settings.beginGroup("Repository");
    m_repositoryPath->setText( QDir::toNativeSeparators( settings.value("repositoryDir", qApp->property("DefaultRepoDir") ).toString() ) );
	settings.endGroup();

	settings.beginGroup("ConfirmDialogs");
	int syncMode = settings.value("TransformationMismatchMode", 0).toInt();	
	if ( syncMode == QDialogButtonBox::Save )
		m_transMismatchBehavior->setCurrentIndex(1);
	else if ( syncMode == QDialogButtonBox::Reset )
		m_transMismatchBehavior->setCurrentIndex(2);
	else	// Ask Mode
		m_transMismatchBehavior->setCurrentIndex(0);
	settings.endGroup();


	connect(m_setPluginPath, SIGNAL(clicked()), m_mapper, SLOT(map()));
	m_mapper->setMapping(m_setPluginPath, m_plugInPath);
	connect(m_setRepositoryPath, SIGNAL(clicked()), m_mapper, SLOT(map()));
	m_mapper->setMapping(m_setRepositoryPath, m_repositoryPath);
	connect(m_mapper, SIGNAL(mapped(QWidget*)), this, SLOT(setPath(QWidget*)));
	connect(m_setShaderEditor, SIGNAL(clicked()), this, SLOT(setShaderEditor()));
	connect(m_plugInPath, SIGNAL(textChanged(const QString&)), this, SLOT(restart()));
	connect(m_repositoryPath, SIGNAL(textChanged(const QString&)), this, SLOT(restart()));
	connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
}


SettingsDialog::~SettingsDialog()
{	

}

void SettingsDialog::setPath(QWidget* path)
{
	QLineEdit* edit = qobject_cast<QLineEdit*>(path);
	Q_ASSERT(edit != 0);
	QString folder = QFileDialog::getExistingDirectory(this, tr("Select folder"), edit->text());
	if (!folder.isEmpty())
		edit->setText(folder);
}

void SettingsDialog::setShaderEditor()
{
#ifdef _WIN32
	QString shaderEditor = QFileDialog::getOpenFileName(this, tr("Select editor application"), QApplication::applicationDirPath(), tr("Executables (*.exe *.com *.bat)"));
#else
	QString shaderEditor = QFileDialog::getOpenFileName(this, tr("Select editor application"), QApplication::applicationDirPath(), tr("Executables (*.*)"));
#endif
	if (!shaderEditor.isEmpty())
		m_shaderEditor->setText(shaderEditor);
}

void SettingsDialog::saveSettings()
{
	if (m_restart)
		QMessageBox::information(this, tr("Attention"), tr("At least one change you made requires a restart of the application!"));
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("General");	
	if ( !m_styleCombo->currentText().isEmpty() )
		settings.setValue("Style", m_styleCombo->currentText());
	settings.setValue("PlugInPath", QDir::toNativeSeparators ( m_plugInPath->text() ) );
	settings.setValue("ShaderEditor", m_shaderEditor->text());
	settings.setValue("UndoStackSize", m_undoSteps->value());
	settings.setValue("glClear", m_glClear->isChecked() );
	switch(m_resetSelectButton->currentIndex())
	{
	case 0:
		settings.setValue("ResetSelectButton", Qt::LeftButton);
		break;
	case 1:
		settings.setValue("ResetSelectButton", Qt::RightButton);
		break;
	case 2:
		settings.setValue("ResetSelectButton", Qt::MidButton);
		break;
	}
	switch(m_cameraMoveButton->currentIndex())
	{
	case 0:
		settings.setValue("CameraMoveButton", Qt::LeftButton);
		break;
	case 1:
		settings.setValue("CameraMoveButton", Qt::RightButton);
		break;
	case 2:
		settings.setValue("CameraMoveButton", Qt::MidButton);
		break;
	}
	switch(m_selectButton->currentIndex())
	{
	case 0:
		settings.setValue("SelectButton", Qt::LeftButton);
		break;
	case 1:
		settings.setValue("SelectButton", Qt::RightButton);
		break;
	case 2:
		settings.setValue("SelectButton", Qt::MidButton);
		break;
	}
	settings.endGroup();	
	settings.beginGroup("Repository");
	settings.setValue("repositoryDir", QDir::toNativeSeparators( m_repositoryPath->text() ) );
	settings.endGroup();

	settings.beginGroup("ConfirmDialogs");
	switch( m_transMismatchBehavior->currentIndex() )
	{
	case 0:
		settings.setValue("TransformationMismatchMode", 0);
		break;
	case 1:
		settings.setValue("TransformationMismatchMode", QDialogButtonBox::Save);
		break;
	case 2:
		settings.setValue("TransformationMismatchMode", QDialogButtonBox::Reset);
		break;
	}
	settings.endGroup();	

}

void SettingsDialog::updateButtons()
{
	if( sender() == m_selectButton )	
	{	
		if (m_resetSelectButton->currentIndex() == m_selectButton->currentIndex())
			m_resetSelectButton->setCurrentIndex( (m_resetSelectButton->currentIndex() + 1 ) % 3 );
		else if (m_cameraMoveButton->currentIndex() == m_selectButton->currentIndex())
			m_cameraMoveButton->setCurrentIndex( (m_cameraMoveButton->currentIndex() + 1 ) % 3 );
	}
	else if (sender() == m_resetSelectButton)
	{
		if (m_cameraMoveButton->currentIndex() == m_resetSelectButton->currentIndex())
			m_cameraMoveButton->setCurrentIndex( (m_cameraMoveButton->currentIndex() + 1 ) % 3 );
		if (m_selectButton->currentIndex() == m_resetSelectButton->currentIndex())
			m_selectButton->setCurrentIndex( (m_selectButton->currentIndex() + 1 ) % 3 );
	}
	else if ( sender() == m_cameraMoveButton )
	{
		if (m_selectButton->currentIndex() == m_cameraMoveButton->currentIndex())
			m_selectButton->setCurrentIndex( (m_selectButton->currentIndex() + 1 ) % 3 );
		else if (m_resetSelectButton->currentIndex() == m_cameraMoveButton->currentIndex())
			m_resetSelectButton->setCurrentIndex( (m_resetSelectButton->currentIndex() + 1 ) % 3 );
	}
}
