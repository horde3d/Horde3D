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

#include "HordeFileDialog.h"
#include "Importer.h"
#include "QHordeSceneEditorSettings.h"

#include <QDir>
#include <QFileDialog>
#include <QLayout>
#include <QTextStream>
#include <QMessageBox>
#include <QtCore/QFile>
#include <QImageReader>

#include <math.h>

#include "SceneFile.h"

#include <Horde3DUtils.h>


HordeFileDialog::HordeFileDialog(H3DResTypes::List type, const QString& resourcePath, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QDialog(parent, flags), 
m_type(type), DefaultRepoPath( qApp->property("DefaultRepoDir").toString() ), m_sceneResourcePath(resourcePath)
{
	setupUi(this);
	m_xmlView->setTabStopWidth(12);
	m_xmlView->setWordWrapMode(QTextOption::NoWrap);

	switch(type)
	{
	case H3DResTypes::Shader:
		initShaderView();
		break;
	case H3DResTypes::Material:
		initMaterialView();
		break;
	case H3DResTypes::Texture:
		initTextureView(H3DResTypes::Texture);
		break;
	case H3DResTypes::ParticleEffect:
		initEffectView();
		break;
	case H3DResTypes::Pipeline:
		initPipelineView();
		break;
        default:
                break;
	}
	connect(m_fileList, SIGNAL(currentItemChanged(QListWidgetItem* , QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*, QListWidgetItem*)));
	m_importer = new Importer();
	m_importer->setTargetPath(m_sceneResourcePath);	
}


HordeFileDialog::~HordeFileDialog()
{
	delete m_importer;
}

QString HordeFileDialog::fileName()
{
	return m_file->text();
}

void HordeFileDialog::accept()
{
	if( m_fileList->currentItem() == 0)
	{
		QMessageBox::information(this, tr("No model selected"), tr("You have to select a model from the list first!"));
		return;
	}

	m_importer->clearErrorLog();
	// If it's not a repostory file we want to import...
	if ( m_fileList->currentItem()->type() != 1)
	{
		QString customData;
		QFileInfo target;
		switch (m_type)
		{
		case H3DResTypes::Shader:
		case H3DResTypes::Material:
		case H3DResTypes::ParticleEffect:
		case H3DResTypes::Pipeline:
			customData = m_xmlView->toPlainText();
		case H3DResTypes::SceneGraph:
		case H3DResTypes::Texture:
			target = QFileInfo(m_sceneResourcePath, fileName());
			break;
		default:
			QMessageBox::warning(this, tr("Error"), tr("Unresolved type in HordeFileDialog"));
			return;
		}
		QFile file( m_fileList->currentItem()->data(Qt::UserRole).value<QFileInfo>().absoluteFilePath() );
		if (file.fileName() != target.absoluteFilePath())
		{				
			if ( target.exists() && 
				QMessageBox::question(this, tr("Overwrite?"), tr("Do you want to overwrite the existing file\n%1").arg(target.absoluteFilePath()), 
				QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
			{
				return;
			}
			QFile::remove(target.absoluteFilePath());
			if ( !file.copy(target.absoluteFilePath()) )
			{					
				QMessageBox::warning(this, tr("Error"), tr("Couldn't write to file %1!").arg(target.absoluteFilePath()));
			}
		}	
		else if( !customData.isEmpty() )
		{
			if( !file.open(QIODevice::WriteOnly | QIODevice::Truncate) )
				QMessageBox::warning(this, tr("Error"), tr("Couldn't write to file %1 !\n\n%2").arg(file.fileName()).arg(file.errorString()));
			else
			{
				file.write(customData.toLatin1());
				file.close();
			}
		}
	}
	// check if it is a repo file	
	else if (m_fileList->currentItem()->type() == 1)
	{

		switch (m_type)
		{
		case H3DResTypes::SceneGraph:
			m_importer->importScene(m_fileList->currentItem()->data(Qt::UserRole).value<QFileInfo>(), 
				QFileInfo(m_importer->targetPath(), fileName()).absoluteFilePath());
			break;
		case H3DResTypes::Shader:
			m_importer->importShader(m_fileList->currentItem()->data(Qt::UserRole).value<QFileInfo>(), 
				QFileInfo(m_importer->targetPath(), fileName()).absoluteFilePath(),
				m_xmlView->toPlainText());
			break;
		case H3DResTypes::Texture:
			m_importer->importTexture(m_fileList->currentItem()->data(Qt::UserRole).value<QFileInfo>(), 
				QFileInfo(m_importer->targetPath(), fileName()).absoluteFilePath() );
			break;
		case H3DResTypes::Material:
			m_importer->importMaterial(m_fileList->currentItem()->data(Qt::UserRole).value<QFileInfo>(), 
				QFileInfo(m_importer->targetPath(), fileName()).absoluteFilePath(),
				m_xmlView->toPlainText());
			break;			
		case H3DResTypes::ParticleEffect:
			m_importer->importEffect(m_fileList->currentItem()->data(Qt::UserRole).value<QFileInfo>(), 
				QFileInfo(m_importer->targetPath(), fileName()).absoluteFilePath(),
				m_xmlView->toPlainText());
			break;
		case H3DResTypes::Pipeline:
			m_importer->importPipeline(m_fileList->currentItem()->data(Qt::UserRole).value<QFileInfo>(),
				QFileInfo(m_importer->targetPath(), fileName()).absoluteFilePath(),
				m_xmlView->toPlainText());
			break;
		default:
			qWarning("Unresolved type in HordeFileDialog");
			break;
		}
	}	
	if( !m_importer->errorLog().isEmpty() )
	{
		QMessageBox::warning(this, tr("The following errors occured during the import process"), m_importer->errorLog().join("\n"));
	}
	QWidgetList widgets = qApp->allWidgets();
	foreach(QWidget* widget, widgets) QApplication::postEvent( widget, new QEvent((QEvent::Type) SceneFile::UpdateFileWatcherEvent) );
	QDialog::accept();
}

void HordeFileDialog::reject()
{
	QDialog::reject();
}

void HordeFileDialog::initTextureView(H3DResTypes::List type)
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("Repository");
	m_currentFilter = "*.jpg;*.png;*.tga;*.bmp;*.psd";
	populateList( m_sceneResourcePath.absolutePath(), m_sceneResourcePath, m_currentFilter, false);
	populateList( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString(), QDir( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString() ), m_currentFilter, true);
	m_stackedWidget->setCurrentWidget(m_imageView);
}

void HordeFileDialog::initShaderView()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("Repository");
	m_currentFilter = "*.shader";
	populateList( m_sceneResourcePath.absolutePath(), m_sceneResourcePath, m_currentFilter, false);
	populateList( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString(), QDir( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString() ), m_currentFilter, true);
	m_stackedWidget->setCurrentWidget(m_xmlView);
}

void HordeFileDialog::initMaterialView()
{
	QHordeSceneEditorSettings settings;
	settings.beginGroup("Repository");
	m_currentFilter = "*.material.xml";
	populateList(m_sceneResourcePath.absolutePath(), m_sceneResourcePath, m_currentFilter, false);
	populateList( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString(), QDir( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString() ), m_currentFilter, true);
	m_stackedWidget->setCurrentWidget(m_xmlView);
}

void HordeFileDialog::initEffectView()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("Repository");
	m_currentFilter = "*.effect.xml";
	populateList(m_sceneResourcePath.absolutePath(), m_sceneResourcePath.absolutePath(), m_currentFilter, false);
	populateList( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString(), QDir( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString() ), m_currentFilter, true);
	m_stackedWidget->setCurrentWidget(m_xmlView);
}

void HordeFileDialog::initPipelineView()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("Repository");
	m_currentFilter = "*.pipeline.xml";
	populateList(m_sceneResourcePath.absolutePath(), m_sceneResourcePath.absolutePath(), m_currentFilter, false);
	populateList( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString(), QDir( settings.value("repositoryDir", DefaultRepoPath.absolutePath()).toString() ), m_currentFilter, true);
	m_stackedWidget->setCurrentWidget(m_xmlView);
}

void HordeFileDialog::populateList(const QString& path, const QDir& baseDir, const QString& filter, bool repo)
{
	// get all files in the given directory
	QFileInfoList files = QDir(path).entryInfoList(filter.split(';', QString::SkipEmptyParts), QDir::Files | QDir::Readable);
	m_fileList->blockSignals(true);
	foreach(QFileInfo file, files)
	{
		QListWidgetItem* item = 0;
		if (repo)
		{
			item = new QListWidgetItem(QIcon(QString::fromUtf8(":/Icons/Resources/Repository.png")), baseDir.relativeFilePath(file.absoluteFilePath()), m_fileList, 1);
			item->setToolTip("Repository file");		
		}
		else
			item = new QListWidgetItem(baseDir.relativeFilePath(file.absoluteFilePath()), m_fileList, 0);
		item->setData(Qt::UserRole, QVariant::fromValue<QFileInfo>(file));		
	}
	QFileInfoList subDirs = QDir(path).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable);
	foreach(QFileInfo dir, subDirs)
		populateList(dir.absoluteFilePath(), baseDir, filter, repo);
	m_fileList->blockSignals(false);
}

void HordeFileDialog::itemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	setCursor(Qt::BusyCursor);
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("Repository");
	if (current)
	{
		switch(m_type)
		{		
		case H3DResTypes::Texture:
			loadTexture(current->data(Qt::UserRole).value<QFileInfo>());
			break;		
		case H3DResTypes::ParticleEffect:
		case H3DResTypes::Material:
		case H3DResTypes::Shader:
		case H3DResTypes::Pipeline:
			// Load shader
			loadXmlFile(current->data(Qt::UserRole).value<QFileInfo>());
			break;					
		default:
			qWarning("Unresolved type in HordeFileDialog");
			break;
		}
		// indicate current selection
		m_file->setText(current->text());
	}
	else // no valid selection
		m_file->setText("");
	settings.endGroup();
	unsetCursor();
}



void HordeFileDialog::loadXmlFile(const QFileInfo& fileName)
{
	QFile file(fileName.absoluteFilePath());
	if (file.open(QIODevice::ReadOnly))				
	{
		m_xmlView->setDocumentTitle(fileName.fileName());
		m_xmlView->setPlainText(file.readAll());				
	}
	else
		m_xmlView->setPlainText(tr("Couldn't open file: %1").arg(file.fileName()));
	file.close();			
}

void HordeFileDialog::loadTexture(const QFileInfo& fileName)
{
	QImageReader imgReader;
	imgReader.setFileName(fileName.absoluteFilePath());
	if (imgReader.canRead())
	{
		QImage img = imgReader.read();
		if (img.isNull())
			m_imageView->setText(imgReader.errorString());
		else
			m_imageView->setPixmap(QPixmap::fromImage(img.scaledToWidth(240, Qt::SmoothTransformation)));
	}
	else
		m_imageView->setText("Image format not supported for preview");
}



QString HordeFileDialog::getResourceFile( H3DResTypes::List resourceType, const QString& targetPath, QWidget *parent, const QString& caption)
{	
	HordeFileDialog dlg( resourceType, targetPath, parent);
	dlg.setWindowTitle(caption);
	if (dlg.exec() == QDialog::Accepted)
		return dlg.fileName();
	else
		return QString();	
}
