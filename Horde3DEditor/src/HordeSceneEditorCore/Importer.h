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

#ifndef IMPORTER_H_
#define IMPORTER_H_

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QVariant>

#include <Horde3D.h>


struct CopyJob
{
	CopyJob() {}
	CopyJob(const QString& source, const QString& target) : Source(source), Target(target) {}
	CopyJob(const QFileInfo& source, const QFileInfo& target) : Source(source), Target(target) {}
	QFileInfo Source;
	QFileInfo Target;
	bool exec() const 
	{
		if ( Source.absoluteFilePath() == Target.absoluteFilePath() )
			return true;
		else
			return  QFile::copy(Source.absoluteFilePath(), Target.absoluteFilePath());
	}

	bool operator == (const CopyJob& other) const
	{
		return (Source == other.Source) && (Target == other.Target);
	}
};
Q_DECLARE_METATYPE(CopyJob)

class QDomElement;

class Importer
{
public:
	Importer();

	const QDir& targetPath()  {return m_targetResourcePath;}
	void setTargetPath(const QDir& path );

	const QDir& sourcePath(H3DResTypes::List type) {return m_sourceResourcePath;}
	void setSourcePath(H3DResTypes::List type, const QDir& path );

	void importScene(const QFileInfo& file, const QString& targetFileName);
	
	void importMaterial(const QFileInfo& file, const QString& targetFileName, const QString& customData = QString());

	void importShader(const QFileInfo& shader, const QString& targetFileName, const QString& customData = QString());

	void importTexture(const QFileInfo& texture, const QString& targetFileName );

	void importEffect(const QFileInfo& effect, const QString& targetFileName, const QString& customData = QString());

	void importPipeline(const QFileInfo& pipeline, const QString& targetFileName, const QString& customData = QString());

	void clearErrorLog() { m_errorLog.clear(); }

	const QStringList& errorLog() { return m_errorLog; }

private:
	void importSceneFile(const QFileInfo& file, const QString& targetFileName);
	void importSceneElement(const QDomElement& element);

	void importShaderFile(const QFileInfo& shader, const QString& targetFileName, const QString& customData );

	void importMaterialFile(const QFileInfo& materialFile, const QString& targetFileName, const QString& customData );
	void importMaterialElement(const QDomElement& element);

	void importEffectFile(const QFileInfo& effectFile, const QString& targetFileName, const QString& customData);
	void importEffectElement(const QDomElement& element);

	void importTextureFile( const QFileInfo& textureFile, const QString& targetFileName );

	void importGeometrieElement(const QDomElement& element);

	QList<CopyJob>	m_alreadyCopied;
	QList<CopyJob>  m_filesToOverwrite;

	QStringList		m_errorLog;

	QDir			m_targetResourcePath;
	QDir			m_sourceResourcePath;
};

#endif


