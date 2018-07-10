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
#include "SoundWidget.h"

#include <QXmlTree/QXmlTreeNode.h>
#include <QXmlTree/QXmlTreeModel.h>

#include <QtWidgets/qfiledialog.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qmessagebox.h>
#include <QtGui/qdesktopservices.h>

#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEngine_Sound.h>

#include "PhonemeEditorWidget.h"

Q_DECLARE_METATYPE(QDomElement)

SoundWidget::SoundWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags), m_currentNode(0)
{
	setupUi(this);
	connect(m_importSound, SIGNAL(clicked()), this, SLOT(addFiles()));	
	connect(m_soundFile, SIGNAL(currentIndexChanged(const QString& )), this, SLOT(updateSoundFile( const QString& )));
	connect(m_gain, SIGNAL(valueChanged(double)), this, SLOT(gainChanged(double)));
	connect(m_maxDist, SIGNAL(valueChanged(double)), this, SLOT(maxDistChanged(double)));
	connect(m_refDist, SIGNAL(valueChanged(double)), this, SLOT(refDistChanged(double)));
	connect(m_pitch, SIGNAL(valueChanged(double)), this, SLOT(pitchChanged(double)));
	connect(m_rolloff, SIGNAL(valueChanged(double)), this, SLOT(rollOffChanged(double)));
	connect(m_loop, SIGNAL(stateChanged(int)), this, SLOT(loopChanged()));
	connect(m_importPhoneme, SIGNAL(clicked()), this, SLOT(addPhonemeFiles()));
	connect(m_phonemeFile, SIGNAL(currentIndexChanged(const QString& )), this, SLOT(updatePhonemeFile( const QString& )));
	connect(m_playButton, SIGNAL(clicked()), this, SLOT(playSound()) );
	connect(m_stopButton, SIGNAL(clicked()), this, SLOT(stopSound()) );
	connect(m_phonemeEditorButton, SIGNAL(clicked()), this, SLOT(openPhonemeEditor()) );
	connect(m_offset, SIGNAL(valueChanged(double)), this, SLOT(offsetChanged(double)));
	connect(m_visemeBlendSpeed, SIGNAL(valueChanged(double)), this, SLOT(visemeBlendSpeedChanged(double)));
	connect(m_forceNoStream, SIGNAL(stateChanged(int)), this, SLOT(forceNoStreamChanged()));
	connect(m_autoStart, SIGNAL(stateChanged(int)), this, SLOT(autoStartChanged()));
}


SoundWidget::~SoundWidget()
{
}


bool SoundWidget::setCurrentNode(QXmlTreeNode *node)
{
	if (node && node->xmlNode().tagName() == "Sound3D")
	{				
		m_currentNode = 0;
		m_soundFile->clear();
		m_phonemeFile->clear();
		scanMediaDir( GameEngine::soundResourceDirectory() );
		scanPhonemeFiles( GameEngine::soundResourceDirectory() );
		// Last Item empty for clearing the phonemefile
		m_phonemeFile->addItem("");

		m_soundFile->setCurrentIndex( m_soundFile->findText( node->xmlNode().attribute("file") ) );
		m_phonemeFile->setCurrentIndex( m_phonemeFile->findText( node->xmlNode().attribute("phonemes") ) );
		m_phonemeEditorButton->setEnabled( !node->xmlNode().attribute("phonemes").isEmpty() );
		m_gain->setValue( node->xmlNode().attribute("gain", "0.5").toFloat() );
		m_loop->setChecked( 
			node->xmlNode().attribute("loop", "false").compare("true", Qt::CaseInsensitive) == 0 ||
			node->xmlNode().attribute("loop", "false").compare("1", Qt::CaseInsensitive) == 0 );
		m_pitch->setValue( node->xmlNode().attribute("pitch", "1.0").toFloat() );
		m_rolloff->setValue( node->xmlNode().attribute("rolloff", "1.0").toFloat() );
		m_maxDist->setValue( node->xmlNode().attribute("maxdist", "30.0").toFloat() );
		m_refDist->setValue( node->xmlNode().attribute("maxrefdist", "15.0").toFloat() );
		m_visemeBlendSpeed->setValue( node->xmlNode().attribute("visemeBlendSpeed", "8.0").toFloat() );
		m_offset->setValue( node->xmlNode().attribute("offset", "0").toFloat() );
		m_autoStart->setChecked( 
			node->xmlNode().attribute("autoStart", "false").compare("true", Qt::CaseInsensitive) == 0 ||
			node->xmlNode().attribute("autoStart", "false").compare("1", Qt::CaseInsensitive) == 0 );
		m_forceNoStream->setChecked( 
			node->xmlNode().attribute("stream", "true").compare("false", Qt::CaseInsensitive) == 0 ||
			node->xmlNode().attribute("stream", "true").compare("0", Qt::CaseInsensitive) == 0 );
		m_currentNode = node;
		return true;
	}
	else
		return false;
}

void SoundWidget::scanMediaDir( const QString& path )
{
	QDir baseDir = GameEngine::soundResourceDirectory();
	QStringList fileFormats = QStringList("*.wav");
	fileFormats.append("*.ogg");
	QFileInfoList sounds = QDir(path).entryInfoList(fileFormats, QDir::Files | QDir::Readable );
	for( int i = 0; i < sounds.size(); ++i )
	{
		m_soundFile->addItem( baseDir.relativeFilePath( sounds[i].absoluteFilePath() ) );
	}
	QFileInfoList dirs = QDir(path).entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot );
	for( int i = 0; i < dirs.size(); ++i )
	{
		scanMediaDir( dirs[i].absoluteFilePath() );
	}	
}

void SoundWidget::addFiles()
{
	QStringList files = QFileDialog::getOpenFileNames(
		this, 
		tr("Select audio file"), 
		QDir::currentPath(), 
		tr("Wave files (*.wav);;Ogg Files (*.ogg);;All files (*.*)"));

	QStringList errorList;
	QDir baseDir( GameEngine::soundResourceDirectory() );
	for( int i = 0; i < files.size(); ++i )
	{
		if( !files[i].contains( baseDir.absolutePath() ) )
		{
			if( !QFile::copy( files[i], baseDir.absoluteFilePath( QFileInfo(files[i]).fileName() ) ) )
				errorList << files[i];
			else
				m_soundFile->addItem( QFileInfo(files[i]).fileName() );
		}
	}
	if( !errorList.isEmpty() )
		QMessageBox::warning(this, tr("Error"), 
		tr("The following files couldn't be copied to the\n"
		"sound directory: %1").arg(baseDir.absolutePath())+
		"\n\n"+errorList.join("\n"));
}


void SoundWidget::updateSoundFile( const QString& soundFile )
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute("file", soundFile );
		GameEngine::setSoundFile( entityWorldID(), qPrintable( soundFile ) );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::gainChanged(double value)
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "gain", value );
		GameEngine::setSoundGain( entityWorldID(), float(value) );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::pitchChanged(double value)
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "pitch", value );
		GameEngine::setSoundPitch( entityWorldID(), float(value) );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::refDistChanged(double value)
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "refdist", value );
		GameEngine::setSoundRefdist( entityWorldID(), float(value) );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::maxDistChanged(double value)
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "maxdist", value );
		GameEngine::setSoundMaxdist( entityWorldID(), float(value) );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::loopChanged()
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "loop", m_loop->isChecked() );
		GameEngine::setSoundLoop( entityWorldID(), m_loop->isChecked() );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::rollOffChanged(double value)
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "rolloff", value );
		GameEngine::setSoundRolloff( entityWorldID(), float(value) );
		emit modified(true); // TODO add undo functionality
	}
}

unsigned int SoundWidget::entityWorldID()
{
	if( m_currentNode )
	{
		return GameEngine::entityWorldID( qPrintable(m_currentNode->xmlNode().parentNode().toElement().attribute("name")) );
	}
	else
		return 0;
}

void SoundWidget::scanPhonemeFiles( const QString& path )
{
	QDir baseDir = GameEngine::soundResourceDirectory();
	QStringList fileFormats = QStringList("*.phonemes.xml");
	QFileInfoList phonemes = QDir(path).entryInfoList(fileFormats, QDir::Files | QDir::Readable );
	for( int i = 0; i < phonemes.size(); ++i )
	{
		m_phonemeFile->addItem( baseDir.relativeFilePath( phonemes[i].absoluteFilePath() ) );
	}
	QFileInfoList dirs = QDir(path).entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot );
	for( int i = 0; i < dirs.size(); ++i )
	{
		scanPhonemeFiles( dirs[i].absoluteFilePath() );
	}	
}

void SoundWidget::addPhonemeFiles()
{
	QStringList files = QFileDialog::getOpenFileNames(
		this, 
		tr("Select phoneme file"), 
		QDir::currentPath(), 
		tr("Phoneme files (*.phonemes.xml);;All files (*.*)"));
	
	m_phonemeFile->removeItem(m_phonemeFile->findText(""));
	QStringList errorList;
	QDir baseDir( GameEngine::soundResourceDirectory() );
	for( int i = 0; i < files.size(); ++i )
	{
		if( !files[i].contains( baseDir.absolutePath() ) )
		{
			if( !QFile::copy( files[i], baseDir.absoluteFilePath( QFileInfo(files[i]).fileName() ) ) )
				errorList << files[i];
			else
				m_phonemeFile->addItem( QFileInfo(files[i]).fileName() );
		}
	}
	if( !errorList.isEmpty() )
		QMessageBox::warning(this, tr("Error"), 
		tr("The following files couldn't be copied to the\n"
		"sound directory: %1").arg(baseDir.absolutePath())+
		"\n\n"+errorList.join("\n"));

	// Last Item empty for clearing the phonemefile
	m_phonemeFile->addItem("");
}

void SoundWidget::updatePhonemeFile( const QString& phonemeFile )
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute("phonemes", phonemeFile );
		GameEngine::setSoundFile( entityWorldID(), qPrintable( m_currentNode->xmlNode().attribute("file") ),qPrintable( phonemeFile ) );
		emit modified(true); // TODO add undo functionality
		
		// Only enable phoneme editor button if a phoneme file is set
		if( !phonemeFile.isEmpty() )
			m_phonemeEditorButton->setEnabled(true);
		else
			m_phonemeEditorButton->setEnabled(false);
	}
	else
		m_phonemeEditorButton->setEnabled(false);
}

void SoundWidget::playSound()
{
	unsigned int id = entityWorldID();
	if( id != 0)
		GameEngine::enableSound( id, true );
}

void SoundWidget::stopSound()
{
	unsigned int id = entityWorldID();
	if( id != 0)
		GameEngine::enableSound( id, false );
}

void SoundWidget::openPhonemeEditor()
{
	if( !m_phonemeEditorButton->isEnabled() ) return;
	unsigned int id = entityWorldID();
	if( id != 0)
	{
		// Call PhonemeEditor with entityWorldID of current Object
		// and the selected phoneme file
		emit activatePhonemeEditor(true, id, m_currentNode->xmlNode().attribute("phonemes") );
	}
}

void SoundWidget::offsetChanged(double value)
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "offset", value );
		GameEngine::setSoundOffset( entityWorldID(), float(value) );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::visemeBlendSpeedChanged(double value)
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "visemeBlendSpeed", value );
		GameEngine::setVisemeBlendSpeed( entityWorldID(), float(value) );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::forceNoStreamChanged()
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "stream", !m_forceNoStream->isChecked() );
		GameEngine::setSoundForceNoStream( entityWorldID(), m_forceNoStream->isChecked() );
		emit modified(true); // TODO add undo functionality
	}
}

void SoundWidget::autoStartChanged()
{
	if( m_currentNode != 0 )
	{
		m_currentNode->xmlNode().setAttribute( "autoStart", m_autoStart->isChecked() );
		emit modified(true); // TODO add undo functionality
		if (m_autoStart->isChecked())
			GameEngine::playSound(entityWorldID());
		else
			GameEngine::stopSound(entityWorldID());
	}
}