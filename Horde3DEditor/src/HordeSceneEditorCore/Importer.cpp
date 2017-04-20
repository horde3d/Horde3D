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

#include "OverwriteFilesDialog.h"
#include "QHordeSceneEditorSettings.h"

#include <QtXml/qdom.h>
#include <QMessageBox>

#include "ShaderData.h"

#include "Importer.h"
#include <Horde3DUtils.h>

Importer::Importer()
{
	QHordeSceneEditorSettings settings;
	settings.beginGroup("Repository");
	m_sourceResourcePath = settings.value("repositoryDir", QApplication::applicationDirPath()+QDir::separator()+"Repository").toString();
	settings.endGroup();

}

void Importer::setTargetPath( const QDir& path )
{
	m_targetResourcePath = path;
	if( !m_targetResourcePath.exists() )
		m_targetResourcePath.mkpath( m_targetResourcePath.absolutePath() );

}


void Importer::setSourcePath(H3DResTypes::List type, const QDir& path)
{
	m_sourceResourcePath = path;
	if( !m_sourceResourcePath.exists() )
		m_sourceResourcePath.mkpath( m_sourceResourcePath.absolutePath() );

}

void Importer::importScene(const QFileInfo& file, const QString& targetFileName)
{
	m_alreadyCopied.clear();
	m_filesToOverwrite.clear();

	importSceneFile(file, targetFileName);	
	if (!m_filesToOverwrite.isEmpty())
	{
		OverwriteFilesDialog dlg(m_filesToOverwrite);
		if( dlg.numFilesToOverwrite() > 0 )
			dlg.exec();
	}
}

void Importer::importShader(const QFileInfo& file, const QString& targetFileName, const QString& customData /*= QString()*/)
{
	m_alreadyCopied.clear();
	m_filesToOverwrite.clear();
	importShaderFile(file, targetFileName, customData);
	if (!m_filesToOverwrite.isEmpty())
	{
		OverwriteFilesDialog dlg(m_filesToOverwrite);
		if( dlg.numFilesToOverwrite() > 0 )
			dlg.exec();
	}
}

void Importer::importTexture( const QFileInfo& texture, const QString& targetFileName )
{
	m_filesToOverwrite.clear();
	m_alreadyCopied.clear();
	importTextureFile( texture, targetFileName );
	if (!m_filesToOverwrite.isEmpty())
	{
		OverwriteFilesDialog dlg(m_filesToOverwrite);
		if( dlg.numFilesToOverwrite() > 0 )
			dlg.exec();
	}
}

void Importer::importMaterial(const QFileInfo& materialFile, const QString& targetFileName, const QString& customData /*= QString()*/)
{
	m_alreadyCopied.clear();
	m_filesToOverwrite.clear();
	importMaterialFile(materialFile, targetFileName, customData);
	if (!m_filesToOverwrite.isEmpty())
	{
		OverwriteFilesDialog dlg(m_filesToOverwrite);
		if( dlg.numFilesToOverwrite() > 0 )
			dlg.exec();
	}
}

void Importer::importEffect(const QFileInfo& effect, const QString& targetFileName, const QString& customData /*= QString()*/)
{
	m_filesToOverwrite.clear();
	m_alreadyCopied.clear();
	importEffectFile(effect, targetFileName, customData);
	if (!m_filesToOverwrite.isEmpty())
	{
		OverwriteFilesDialog dlg(m_filesToOverwrite);
		if( dlg.numFilesToOverwrite() > 0 )
			dlg.exec();
	}
}

void Importer::importPipeline(const QFileInfo& pipeline, const QString& targetFileName, const QString& customData /*= QString()*/)
{
	m_filesToOverwrite.clear();
	m_alreadyCopied.clear();
	QFileInfo target(m_targetResourcePath, targetFileName);
	if (!target.absoluteDir().exists())
		m_targetResourcePath.mkpath(target.absolutePath());		
	const CopyJob job(pipeline, target);

	QFile file(pipeline.absoluteFilePath());
	if (!file.copy(target.absoluteFilePath()))
	{
		if (!customData.isEmpty())
		{
			if (QMessageBox::question(0, QObject::tr("Overwrite file?"), QObject::tr("Do you want to overwrite the existing pipeline\n%1").arg(target.absoluteFilePath()), QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape) != QMessageBox::Yes)
				return;
		}
		else if (!m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job))
			m_filesToOverwrite.append(job);	
	}
	else
		m_alreadyCopied.append(job);

	if ( !customData.isEmpty() )
		file.setFileName(target.absoluteFilePath());

	if (!file.open(QIODevice::ReadWrite))
	{
		m_errorLog.push_back(QObject::tr("Couldn't open pipeline file %1: %2").arg(file.fileName()).arg(file.errorString()));
		return;
	}
	if ( !customData.isEmpty() )
	{		
		file.resize(customData.toLocal8Bit().size());
		file.write(customData.toLocal8Bit());
		file.seek(0);
	}
	QDomDocument root;
	root.setContent(&file);
	file.close();
	QDomNodeList materialNodes = root.documentElement().elementsByTagName("Stage");
	for( int i=0; i<materialNodes.size(); ++i )
		if( materialNodes.at(i).toElement().hasAttribute("link") )
			importMaterialFile(
				QFileInfo(m_sourceResourcePath, materialNodes.at(i).toElement().attribute("link")),
				materialNodes.at(i).toElement().attribute("link"),
				QString());

	materialNodes = root.documentElement().elementsByTagName("DrawQuad");
	for (int i=0; i<materialNodes.size(); ++i)
		if (materialNodes.at(i).toElement().hasAttribute("material"))
			importMaterialFile(
				QFileInfo(m_sourceResourcePath, materialNodes.at(i).toElement().attribute("material")),
				materialNodes.at(i).toElement().attribute("material"),
				QString());
	materialNodes = root.documentElement().elementsByTagName("SetUniform");
	for (int i=0; i<materialNodes.size(); ++i)
		if (materialNodes.at(i).toElement().hasAttribute("material"))
			importMaterialFile(
				QFileInfo(m_sourceResourcePath, materialNodes.at(i).toElement().attribute("material")),
				materialNodes.at(i).toElement().attribute("material"),
				QString());

	if (!m_filesToOverwrite.isEmpty())
	{
		OverwriteFilesDialog dlg(m_filesToOverwrite);
		if( dlg.numFilesToOverwrite() > 0 )
			dlg.exec();
	}
}

void Importer::importSceneFile(const QFileInfo& file, const QString& targetFileName)
{
	QFile sceneFile(file.absoluteFilePath());
	if (!sceneFile.open(QIODevice::ReadOnly))
	{
		m_errorLog << QObject::tr("Error opening file %1").arg(sceneFile.fileName());
		return;
	}
	QDomDocument root;
	root.setContent(&sceneFile);
	sceneFile.close();
	QFileInfo target(m_targetResourcePath, targetFileName);
	if( !target.absoluteDir().exists() )
		m_targetResourcePath.mkpath(target.absolutePath());

	if (!sceneFile.copy(target.absoluteFilePath()))
	{
		CopyJob job(file, target);
		if (!m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job))
			m_filesToOverwrite.append(job);	
	}
	else
		m_alreadyCopied.append(CopyJob(file, target));
	importEffectElement(root.documentElement());
	importMaterialElement(root.documentElement());
	importGeometrieElement(root.documentElement());	
	importSceneElement(root.documentElement());
}

void Importer::importSceneElement(const QDomElement& element)
{
	if (element.hasAttribute("sceneGraph"))
		importSceneFile(QFileInfo(m_sourceResourcePath, element.attribute("sceneGraph")), element.attribute("sceneGraph"));
	QDomNodeList childs = element.childNodes();
	for (int i=0; i<childs.count(); ++i)
		importSceneElement(childs.at(i).toElement());
}


void Importer::importMaterialFile(const QFileInfo& materialFile, const QString& targetFileName, const QString& customData )
{
	if ( !materialFile.exists() )
	{
		m_errorLog << QObject::tr("The material file %1 couldn't be found!").arg(materialFile.absoluteFilePath());		
		return;
	}

	QFileInfo target(m_targetResourcePath, targetFileName);
	if (!target.absoluteDir().exists())
		m_targetResourcePath.mkpath(target.absolutePath());


	QDomDocument material;
	const CopyJob job(materialFile, target);
		
	QFile file(materialFile.absoluteFilePath());
	if (!file.copy(target.absoluteFilePath()))
	{				
		if (!customData.isEmpty())
		{
			if (QMessageBox::question(0, QObject::tr("Overwrite file?"), QObject::tr("Do you want to overwrite the existing material\n%1").arg(target.absoluteFilePath()), QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape) != QMessageBox::Yes)
				return;
		}
		else if ( !m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job) )
			m_filesToOverwrite.append(job);	
	}
	else
		m_alreadyCopied.append(job);

	if ( !customData.isEmpty() )
		file.setFileName(target.absoluteFilePath());	
	
	// Linux problem?
	if( file.isOpen() ) file.close();

	if (!file.open(QIODevice::ReadWrite))
	{
		m_errorLog << QObject::tr("Couldn't open material file %1: %2").arg(file.fileName()).arg(file.errorString());
		return;
	}
	if ( !customData.isEmpty() )
	{		
		file.resize(customData.toLocal8Bit().size());
		file.write(customData.toLocal8Bit());
		file.seek(0);
	}
	material.setContent(&file);
	file.close();

	QString linkedMaterial = material.documentElement().attribute("link");
	if( !linkedMaterial.isEmpty() )
		importMaterialFile( QFileInfo(m_sourceResourcePath, linkedMaterial), linkedMaterial, QString() );

	QDomNodeList shaders = material.documentElement().elementsByTagName("Shader");
	for (int j=0; j<shaders.count(); ++j)
		importShaderFile(
			QFileInfo(m_sourceResourcePath, shaders.at(j).toElement().attribute("source")),
			shaders.at(j).toElement().attribute("source"),
			QString());
	QDomNodeList textures = material.documentElement().elementsByTagName("Sampler");
	for (int j=0; j<textures.count(); ++j)
	{
			importTextureFile(
				QFileInfo(m_sourceResourcePath, textures.at(j).toElement().attribute("map")),
				textures.at(j).toElement().attribute("map"));		
	}
}

void Importer::importMaterialElement(const QDomElement& element)
{
	if (element.hasAttribute("material"))		
		importMaterialFile(
			QFileInfo(m_sourceResourcePath, element.attribute("material")), 
			element.attribute("material"), 
			QString());		
	QDomNodeList childs = element.childNodes();
	for (int i=0; i<childs.count(); ++i)
		importMaterialElement(childs.at(i).toElement());		
}

void Importer::importShaderFile(const QFileInfo& shader, const QString& targetFileName, const QString& customData)
{			
	if ( !shader.exists() )
	{
		m_errorLog << QObject::tr("The shader file %1 couldn't be found!").arg(shader.absoluteFilePath());
		return;
	}

	QFileInfo target(m_targetResourcePath, targetFileName);
	if (!target.absoluteDir().exists())
		m_targetResourcePath.mkpath(target.absolutePath());

	const CopyJob job(shader, target);
	QFile file(shader.absoluteFilePath());
	if (!file.copy(target.absoluteFilePath()) && !m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job) )
	{			
		if (!customData.isEmpty())
		{
			if (QMessageBox::question(0, QObject::tr("Overwrite file?"), QObject::tr("Do you want to overwrite the existing shader\n%1").arg(shader.absoluteFilePath()), QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape) != QMessageBox::Yes)
				return;
		}
		else
			m_filesToOverwrite.append(job);
	}
	else
		m_alreadyCopied.append(job);

	file.setFileName(target.absoluteFilePath());	
	if (!file.open(QIODevice::ReadWrite))
	{
		m_errorLog.push_back(QObject::tr("Couldn't open shader file %1").arg(file.fileName()));
		return;
	}
	if ( !customData.isEmpty() )
	{		
		file.resize(customData.toLocal8Bit().size());
		file.write(customData.toLocal8Bit());
		file.seek(0);
	}
	
	ShaderData shaderData( file.readAll() );
	const QStringList includedFiles = shaderData.includeFiles();
	for (int i=0; i<includedFiles.count(); ++i)
	{		
		target = includedFiles.at(i);
		if (!target.absoluteDir().exists())
			m_targetResourcePath.mkpath(target.absolutePath());
		QFileInfo source(m_sourceResourcePath,includedFiles.at(i) );
		CopyJob job(source, target);
		if (!job.exec() && !m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job))
			m_filesToOverwrite.append(job);
		else
			m_alreadyCopied.append(job);
	}
	const QStringList textureFiles = shaderData.textureFiles();
	for (int i=0; i<textureFiles.count(); ++i)
	{		
		target = textureFiles.at(i);
		if (!target.absoluteDir().exists())
			m_targetResourcePath.mkpath(target.absolutePath());
		QFileInfo source(m_sourceResourcePath,textureFiles.at(i) );
		CopyJob job(source, target);
		if (!job.exec() && !m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job))
			m_filesToOverwrite.append(job);
		else
			m_alreadyCopied.append(job);
	}
}


void Importer::importTextureFile( const QFileInfo& textureFile, const QString& targetFileName )
{
	QFileInfo target = QFileInfo(m_targetResourcePath, targetFileName);
	if (!target.absoluteDir().exists())
		m_targetResourcePath.mkpath(target.absolutePath());

	const CopyJob job(textureFile, target);
	if (!job.exec() && !m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job))
		m_filesToOverwrite.append(job);		
	else
		m_alreadyCopied.append(job);
}




void Importer::importGeometrieElement(const QDomElement& element)
{
	if (element.hasAttribute("geometry"))
	{
		QFileInfo source = QFileInfo(m_sourceResourcePath, element.attribute("geometry"));
		QFile geometryFile(source.absoluteFilePath());
		QFileInfo target(m_targetResourcePath, element.attribute("geometry"));
		if (!target.absoluteDir().exists())
			m_targetResourcePath.mkpath(target.absolutePath());
		if (!geometryFile.copy(target.absoluteFilePath()))
		{
			CopyJob job(source, target);
			if (!m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job))
				m_filesToOverwrite.append(job);	
		}
		else
			m_alreadyCopied.append(CopyJob(source, target));
	}
	QDomNodeList childs = element.childNodes();
	for (int i=0; i<childs.count(); ++i)
		importGeometrieElement(childs.at(i).toElement());
}

void Importer::importEffectFile(const QFileInfo& effectFile, const QString& targetFileName, const QString& customData)
{
	QFileInfo target(m_targetResourcePath, targetFileName);
	if (!target.absoluteDir().exists())
		m_targetResourcePath.mkpath(target.absolutePath());		
	const CopyJob job(effectFile, target);
	if (!job.exec() && !m_filesToOverwrite.contains(job) && !m_alreadyCopied.contains(job))
	{
		if (!customData.isEmpty())
		{
			if (QMessageBox::question(0, QObject::tr("Overwrite file?"), QObject::tr("Do you want to overwrite the existing effect\n%1").arg(effectFile.absoluteFilePath()), QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape) != QMessageBox::Yes)
				return;
			QFile targetFile(target.absoluteFilePath());
			if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
				m_errorLog.push_back(QObject::tr("Error opening %1 for writing!").arg(target.absoluteFilePath()));
			else
				targetFile.write(customData.toLocal8Bit());
			targetFile.close();
		}
		else
			m_filesToOverwrite.append(job);		
	}
	else
		m_alreadyCopied.append(job);
}

void Importer::importEffectElement(const QDomElement& element)
{
	if (element.hasAttribute("effect"))
	{
		QFileInfo effectFile(m_sourceResourcePath, element.attribute("effect"));
		importEffectFile(effectFile, element.attribute("effect"), QString());
	}
	QDomNodeList childs = element.childNodes();
	for (int i=0; i<childs.count(); ++i)
		importEffectElement(childs.at(i).toElement());		
}


