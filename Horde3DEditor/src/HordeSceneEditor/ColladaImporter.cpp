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

#include "ColladaImporter.h"

#include "OverwriteFilesDialog.h"
#include "QHordeSceneEditorSettings.h"
#include "HordeSceneEditor.h"

#include <QtXml/qdom.h>
#include <QtCore/QProcess>
#include <QMessageBox>
#include <QtCore/QDir>
#include <QFileDialog>
#include <QProgressDialog>
#include <QtCore/QFileInfo>

ColladaImporter::ColladaImporter(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QDialog(parent, flags)
{
	setupUi(this);
    m_editorInstance = static_cast<HordeSceneEditor*>(qApp->property("SceneEditorInstance").value<void*>());
	connect(m_setColladaFile, SIGNAL(clicked()), this, SLOT(setColladaFiles()));
	connect(m_importButton, SIGNAL(clicked()), this, SLOT(importFiles()));
	connect(m_lod, SIGNAL(currentIndexChanged(int)), this, SLOT(lodLevelChanged(int)));
	connect(m_lodDist, SIGNAL(valueChanged(int)), this, SLOT(lodChanged(int)));
	m_convertProcess = new QProcess(this);
    m_editorInstance->pauseRendering( true );
}


ColladaImporter::~ColladaImporter()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("ColladaImporter");
	settings.setValue("noOptimizeGeometry", m_noOptimizeGeometry->isChecked());
	settings.setValue("lodDist1", m_lod->itemData(0).toInt() );
	settings.setValue("lodDist2", m_lod->itemData(1).toInt() );
	settings.setValue("lodDist3", m_lod->itemData(2).toInt() );
	settings.setValue("lodDist4", m_lod->itemData(3).toInt() );
	settings.endGroup();
    m_editorInstance->pauseRendering( false );
}


void ColladaImporter::initImportPath(const QString& resourcePath)
{
	m_resourceDir = resourcePath;
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("ColladaImporter");
	m_lod->setItemData(0, settings.value("lodDist1", 0).toInt() );
	m_lod->setItemData(1, settings.value("lodDist2", 0).toInt() );
	m_lod->setItemData(2, settings.value("lodDist3", 0).toInt() );
	m_lod->setItemData(3, settings.value("lodDist4", 0).toInt() );
	m_noOptimizeGeometry->setChecked(settings.value("noOptimizeGeometry", false).toBool());
	settings.endGroup();
}

bool ColladaImporter::setColladaFiles()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("ColladaImporter");
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Select collada file(s)"), settings.value("LastDaeDir").toString(), tr("Collada files (*.collada *.dae);;All files (*.*)"));
	if (files.isEmpty()) return false;
	settings.setValue("LastDaeDir", QFileInfo(files[0]).absolutePath());
	settings.endGroup();
	m_colladaFilesLabel->setText( files.join(";") );
	m_colladaFiles = files;
	return true;
}

void ColladaImporter::importFiles()
{
	if( m_colladaFiles.isEmpty() )	return;
	// Collada to Horde Converter executable
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("ColladaImporter");
	QFileInfo converter(settings.value("Converter", QApplication::applicationDirPath()+QDir::separator()+"ColladaConv.exe").toString());	
	if (!converter.exists())
	{
		QString path;
#ifdef _WIN32
		path = QFileDialog::getOpenFileName(this, tr("Select collada converter"), QDir::currentPath(), tr("Executables (*.exe)"));
		converter = QFileInfo( path );
#else
		path = QFileDialog::getOpenFileName(this, tr("Select collada converter"), QDir::currentPath(), tr("Executables (*)"));
		converter = QFileInfo( path );
#endif
		if (!converter.exists())
			return;
		settings.setValue("Converter", converter.absoluteFilePath());
	}
	settings.endGroup();
	
	// Create temporary directory
	QString tempDirBase = "HordeSceneEditorConversion";
	while( QDir::temp().exists(tempDirBase) )
		tempDirBase = "HordeSceneEditorConversion" + QString::number( rand() % 999 );	

	if (!QDir::temp().mkpath(tempDirBase))
	{
		QMessageBox::warning(this, tr("Error"), tr("Couldn't create temporary directory for conversion"));
		return;
	}

	m_outputDir = QDir( QFileInfo(QDir::temp(), tempDirBase).absoluteFilePath() );

	QDir textureDir(QFileInfo(m_colladaFiles[0]).absolutePath());

	QProgressDialog progress(tr("Converting files"), tr("Cancel"), 0, m_colladaFiles.size(), this);
	progress.setMinimumDuration(0);
	// Copy collada file for conversion to temporary directory
	for( int c = 0; c < m_colladaFiles.size(); ++c )
	{
		QFileInfo colladaFile( m_colladaFiles[c] );
		m_outputDir.mkpath( QString("models") + QDir::separator() + colladaFile.baseName() );
		QDir modelOutputDir = m_outputDir;
		modelOutputDir.cd( "models" );
		modelOutputDir.cd( colladaFile.baseName() );
		QFileInfo tempColladaFile( modelOutputDir, QFileInfo(m_colladaFiles[c]).fileName());
		QFile inputFile(m_colladaFiles[c]);
		if (!inputFile.copy(tempColladaFile.absoluteFilePath()))
		{
			QMessageBox::warning(this, tr("Error"), tr("Couldn't copy collada file %1 to temporary directory!\n%2").arg(m_colladaFiles[c]).
				arg(inputFile.errorString()));
			return;
		}

		// Prepare conversion
		QStringList arguments;
		arguments << QDir::toNativeSeparators( m_outputDir.relativeFilePath( tempColladaFile.absoluteFilePath() ) ); 
		arguments << "-base ";
		arguments << QDir::toNativeSeparators( m_outputDir.absolutePath() );
		arguments << "-dest ";
		arguments << QDir::toNativeSeparators( m_outputDir.absolutePath() );

		//arguments << tempColladaFile.baseName();
		
		if (m_noOptimizeGeometry->isChecked())
			arguments << " -noGeoOpt ";
		arguments << " -overwriteMats ";
		if( m_lod->itemData(0).toInt() > 0 )
			arguments << " -lodDist1 " << QString::number( m_lod->itemData(0).toInt() );
		if( m_lod->itemData(1).toInt() > 0 )
			arguments << " -lodDist2 " << QString::number( m_lod->itemData(1).toInt() );
		if( m_lod->itemData(2).toInt() > 0 )
			arguments << " -lodDist3 " << QString::number( m_lod->itemData(2).toInt() );
		if( m_lod->itemData(3).toInt() > 0 )
			arguments << " -lodDist4 " << QString::number( m_lod->itemData(3).toInt() );
		// Convert
		m_logWidget->append( converter.absoluteFilePath() + " " + arguments.join(" ") );
		setCursor(Qt::BusyCursor);
		progress.setValue(c);
		m_convertProcess->start(converter.absoluteFilePath(), arguments);
		while( m_convertProcess->waitForFinished( 250 ) == false  )
		{
			QApplication::processEvents();
			if( progress.wasCanceled() )
			{
				m_convertProcess->close();
				cleanUp();
				unsetCursor();
				return;
			}
			if( m_convertProcess->state() == QProcess::NotRunning && m_convertProcess->error() != QProcess::Timedout )
			{
				QMessageBox::warning(this, tr("Error"), tr("Collada converter exited with error \"%1\" when converting %2!").arg(m_convertProcess->errorString()).arg(m_colladaFiles[c]));
				cleanUp();
				unsetCursor();
				return;
			}
		}
		unsetCursor();
		// Show converter status messages
		m_logWidget->append(m_convertProcess->readAllStandardOutput());
		// Remove the copied collada file
		QFile::remove(tempColladaFile.absoluteFilePath());

		// Create ouput directories
		QDir outDir = m_resourceDir;
		if (!outDir.exists("models"))
			outDir.mkdir("models");
		outDir.cd("models");
		if (!outDir.exists(tempColladaFile.baseName()))
			outDir.mkdir(tempColladaFile.baseName());
		outDir.cd(tempColladaFile.baseName());
		QDir animOutDir = m_resourceDir;
		if (!animOutDir.exists("animations"))
			animOutDir.mkdir("animations");
		animOutDir.cd("animations");
	
		// Search created files and copy them to ouput directory
		QList<CopyJob> filesToOverwrite;
		QList<CopyJob> alreadyCopied;
		QStringList createdFiles;
		QDir searchDir = m_outputDir;	
		searchDir.cd( "models" );
		searchDir.cd( tempColladaFile.baseName() );
		QStringList sceneGraphFiles = searchDir.entryList(QStringList("*.scene.xml"), QDir::Files | QDir::Readable);
		for (int i=0; i<sceneGraphFiles.size(); ++i)
		{
			QString input(searchDir.absoluteFilePath(sceneGraphFiles[i]));
			createdFiles.append(input);
			QString output(QFileInfo(outDir, sceneGraphFiles[i]).absoluteFilePath());
			CopyJob job(input, output);
			if (!job.exec() && !filesToOverwrite.contains(job) && !alreadyCopied.contains(job))
				filesToOverwrite.push_back(job);
			else
			{
				alreadyCopied.push_back(job);
				QFile::remove(input);
			}
		}	

		QStringList geoFiles = searchDir.entryList(QStringList("*.geo"), QDir::Files | QDir::Readable);
		for (int i=0; i<geoFiles.size(); ++i)
		{
			QString input(searchDir.absoluteFilePath(geoFiles[i]));
			createdFiles.append(input);
			QString output(QFileInfo(outDir, geoFiles[i]).absoluteFilePath());
			CopyJob job(input, output);
			if (!job.exec() && !filesToOverwrite.contains(job) && !alreadyCopied.contains(job))
				filesToOverwrite.push_back(job);
			else
			{
				alreadyCopied.push_back(job);
				QFile::remove(input);
			}
		}

		QStringList animationFiles = searchDir.entryList(QStringList("*.anim"), QDir::Files | QDir::Readable);
		for (int i=0; i<animationFiles.size(); ++i)
		{
			QString input(searchDir.absoluteFilePath(animationFiles[i]));
			createdFiles.append(input);
			QString output(QFileInfo(animOutDir, animationFiles[i]).absoluteFilePath());
			CopyJob job(input, output);
			if (!job.exec() && !filesToOverwrite.contains(job) && !alreadyCopied.contains(job))
				filesToOverwrite.push_back(job);
			else
			{
				alreadyCopied.push_back(job);
				QFile::remove(input);
			}
		}

		// Store texture filenames used by the materials
		QStringList textureFiles;
		// Search for materials
		QStringList materialFiles = searchDir.entryList(QStringList("*.material.xml"), QDir::Files | QDir::Readable);	
		// now parse the materials for textures and copy them to the repository
		for (int i=0; i<materialFiles.size(); ++i)
		{			
			QString input(searchDir.absoluteFilePath(materialFiles[i]));
			createdFiles.append(input);
			QFile material(input);
			QDomDocument materialXml;
			if (!material.open(QIODevice::ReadOnly) || !materialXml.setContent(&material))
				QMessageBox::warning( 
					this, 
					tr("Error"), 
					tr("Couldn't open material file '%1' to extract texture information!").arg(searchDir.absoluteFilePath(materialFiles[i])) 
				);		
			else
			{
				QDomNodeList textures = materialXml.elementsByTagName("Sampler");
				for (int j=0; j<textures.size(); ++j)
					textureFiles << textures.at(j).toElement().attribute("map");
			}
			QString output(QFileInfo(outDir, materialFiles[i]).absoluteFilePath());
			CopyJob job(input, output);
			if (!job.exec() && !filesToOverwrite.contains(job) && !alreadyCopied.contains(job))
				filesToOverwrite.push_back(job);
			else
			{
				alreadyCopied.push_back(job);
				QFile::remove(input);
			}
		}

		for (int i=0; i<textureFiles.size(); ++i)
		{	
			QString output(QFileInfo(m_resourceDir, textureFiles[i]).absoluteFilePath());
			QString input(QFileInfo(m_resourceDir, textureFiles[i]).absoluteFilePath());
			if (!QFile::exists(input))
			{
				QFileInfo textureFile(m_resourceDir, textureFiles[i]);				
				input = QFileDialog::getOpenFileName(
					this, 
					tr("Where is the texture file %1").arg(textureFiles[i]), 
					textureDir.absolutePath(), 
					tr("Texture (%1)").arg(textureFile.fileName()));
			}
			if (!input.isEmpty())
			{
				textureDir = QFileInfo(input).absoluteDir();
				CopyJob job(input, output);
				if (!job.exec() && !filesToOverwrite.contains(job) && !alreadyCopied.contains(job))
					filesToOverwrite.push_back(job);
				else		
					alreadyCopied.push_back(job);		
			}
		}

		if (!filesToOverwrite.isEmpty())
		{
			OverwriteFilesDialog dlg(filesToOverwrite, this);
			dlg.exec();
		}
	
		// Remove temporary files and dirs
		for (int i=0; i<createdFiles.size(); ++i)
			QFile::remove(createdFiles[i]);
		QDir( m_outputDir.absolutePath() + QDir::separator() + "models" ).rmdir(tempColladaFile.baseName());
	}
	m_outputDir.rmdir( "models" );
	cleanUp();
	m_logWidget->append("\nImport finished!");
}

void ColladaImporter::lodLevelChanged( int index )
{
	m_lodDist->blockSignals( true );
	m_lodDist->setValue( m_lod->itemData( index ).toInt() );
	m_lodDist->blockSignals( false );

}

void ColladaImporter::lodChanged( int value )
{
	m_lod->setItemData( m_lod->currentIndex(), value );
}

void ColladaImporter::cleanUp()
{
	QStringList modelDirs = m_outputDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks );
	for( int i = 0; i < modelDirs.size(); ++i )
	{
		QFileInfoList files = QDir(modelDirs[i]).entryInfoList( QDir::Files | QDir::NoSymLinks );
		foreach( QFileInfo file, files )
			QFile::remove( file.absoluteFilePath() );
		m_outputDir.rmdir(modelDirs[i]);
	}

	QStringList files = m_outputDir.entryList( QDir::Files | QDir::NoSymLinks );
	foreach( QString file, files )
		QFile::remove(file);

	QDir::temp().rmdir(m_outputDir.absolutePath());

}

