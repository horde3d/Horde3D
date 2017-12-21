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
// Phoneme Editor
// ---------------------------------------------------------
// Copyright (C) 2008 Felix Kistler
// 
// ****************************************************************************************
#include "PhonemeEditorWidget.h"
#include "TimeLineComboBox.h"
#include "TimeLineTextEdit.h"

#include <QXmlTree/QXmlTreeNode.h>
#include <QXmlTree/QXmlTreeModel.h>

#include <QtWidgets/qfiledialog.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qmessagebox.h>
#include <QtGui/qdesktopservices.h>
#include <QtCore/QTextStream>
#include <QtWidgets/qlineedit.h>

#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEvent.h>
#include <GameEngine/GameEngine_Sound.h>
#include <math.h>

Q_DECLARE_METATYPE(QDomElement)

// List of available Phonemes
const QStringList PhonemeEditorWidget::m_phonemeList = (QStringList() << "x" << "AE" << "AY" << "AH" << "AA" << "AO" << "EY"  << "EH"  << "UH"
														<< "ER"	<< "y" << "IY" << "IH" << "w" << "UW" << "OW" << "AW" << "OY" << "AY" << "h" 
														<< "r" << "l" << "s" << "z" << "SH" << "CH" << "ZH" << "j" << "TH" << "DH"
														<< "f" << "v" << "d" << "t" << "n" << "k" << "g" << "NG" << "p" << "b" << "m");

PhonemeEditorWidget::PhonemeEditorWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags), m_entityWorldID(0)
{
	m_phonemeFileName = QString();
	setupUi(this);
	connect(m_playButton, SIGNAL(clicked()), this, SLOT(playSound()) );
	connect(m_stopButton, SIGNAL(clicked()), this, SLOT(stopSound()) );
	connect(m_closeButton, SIGNAL(clicked()), this, SLOT(closePhonemeEditor()) );
	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(save()) );
	m_saveButton->setEnabled(false);
	setEnabled(false);
}


PhonemeEditorWidget::~PhonemeEditorWidget()
{
}


void PhonemeEditorWidget::playSound()
{
	if( m_entityWorldID != 0)
		GameEngine::enableSound( m_entityWorldID, true );
}

void PhonemeEditorWidget::stopSound()
{
	if( m_entityWorldID != 0)
		GameEngine::enableSound( m_entityWorldID, false );
}

void PhonemeEditorWidget::loadPhonemeFile(unsigned int id, const QString& path)
{
	blockSignals(true);
	m_entityWorldID = id;
	m_phonemeFileName = QString(path);
	if( !m_phonemeFileName.isEmpty() )
	{
		// Open phoneme file
		QFile file( QDir(GameEngine::soundResourceDirectory()).absoluteFilePath(m_phonemeFileName) );
		if (file.open(QIODevice::ReadOnly))
		{	
			QString errorMsg;
			int errorLine, errorColumn;
			// read phoneme file
			if (!m_phonemeXml.setContent(&file, &errorMsg, &errorLine, &errorColumn))
				setStatusTip(tr("Error in line %1 column %2 when reading phoneme file %3: %4").arg(errorLine).arg(errorColumn).arg(m_phonemeFileName).arg(errorMsg));
			else
			{
				setStatusTip("");		
				//make Content visible
				parseXmlFile();
			}
			file.close();
		}
		else
			setStatusTip(file.errorString());
	}
	setEnabled(!m_phonemeFileName.isEmpty());
	blockSignals(false);
}

void PhonemeEditorWidget::closePhonemeEditor()
{
	if (m_saveButton->isEnabled())
	{
		if (QMessageBox::question(
			this, 
			tr("Save changes?"), 
			tr("Save previous changes to phoneme file %1?").arg(m_phonemeFileName), 
			QMessageBox::Save | QMessageBox::Default, 
			QMessageBox::Ignore | QMessageBox::Escape)==QMessageBox::Save)
			save();				
	}
	m_saveButton->setEnabled(false);
	m_phonemeXml = QDomDocument();
	emit closeEditor();
}

void PhonemeEditorWidget::save()
{
	blockSignals(true);
	//TODO: gaps between phonemes have to be interpreted as silence (phoneme "x")
	//Save changes to phoneme file
	if (!m_phonemeFileName.isEmpty() && m_saveButton->isEnabled())
	{
		QFile file( QDir(GameEngine::soundResourceDirectory()).absoluteFilePath(m_phonemeFileName) );
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			QTextStream stream(&file);
			m_phonemeXml.save(stream, 4);
			file.flush();
			file.close();
		}
		else
		{
			QMessageBox::warning(this, tr("Error"), tr("Error opening file %1 for writing:\n\n%2").arg(m_phonemeFileName).arg(file.errorString()));
			return;
		}
	}
	m_saveButton->setEnabled(false);
	GameEngine::sendEvent(m_entityWorldID, &GameEvent(GameEvent::E_SET_PHONEMES_FILE, &GameEventData(qPrintable(m_phonemeFileName)), 0));
	blockSignals(false);
}

void PhonemeEditorWidget::parseXmlFile()
{
	// Clear old text boxes
	QList<TimeLineTextEdit *> oldText= m_text->findChildren<TimeLineTextEdit *>();
	while(!oldText.isEmpty())
		delete oldText.takeFirst();

	// Clear old phoneme comboboxes
	QList<TimeLineComboBox *> oldPhoneme = m_phonemes->findChildren<TimeLineComboBox *>();
	while(!oldPhoneme.isEmpty())
		delete oldPhoneme.takeFirst();
	
	// Get new items from phoneme xml
	QDomElement timings = m_phonemeXml.firstChildElement("PhonemeTimings");

	// Get length of phoneme timings
	QDomElement last = timings.lastChildElement("word");
	int sentenceLength = 0;
	if(!last.isNull()) sentenceLength = last.attribute("end", "0").toInt();
	last = timings.lastChildElement("phn");
	if( !last.isNull() ) sentenceLength = max(sentenceLength, last.attribute("end", "0").toInt() );
	
	// Get minimal phoneme width
	QDomNodeList phnlist = timings.elementsByTagName("phn");
	int minPhnWidth = 999999;
	for(int i=0; i<phnlist.length(); i++)
	{
		QDomElement phn = phnlist.item(i).toElement();
		// Skip silence
		if( phn.attribute("value").compare("x") == 0 ) continue;
		int start = phn.attribute("start", "0").toInt();
		int end = phn.attribute("end", "999999").toInt();
		if( end-start < minPhnWidth ) minPhnWidth = end-start;
	}
	int minWidth =  (int)( 39.5f / minPhnWidth * sentenceLength );
	
	// Get length of panels and enlarge them if too small
	if( m_text->width()-2 < minWidth)
	{
		m_text->setFixedWidth(minWidth+2);
		m_phonemes->setFixedWidth(minWidth+2);
		m_phonemeScrollArea->setFixedWidth(m_text->x()+minWidth+2);
	}
	
	// Add wordBoxes in reverse direction, because first nodes have higher priority
	// and should be printed above of the others
	QDomNode wordNode = timings.lastChild();
	while(!wordNode.isNull())
	{
		// Get words
		if( wordNode.nodeName().compare("word") == 0)
		{
			QDomElement word = wordNode.toElement();
			TimeLineTextEdit* wordBox = new TimeLineTextEdit(word, sentenceLength, m_text);
			connect(wordBox, SIGNAL(xmlNodeChanged()), this, SLOT(enableSaveButton()) );
			wordBox->setVisible(true);

			// Get phonemes of a word
			QDomNodeList phonemeList = word.elementsByTagName("phn");
			// also reverse direction
			for(int i = phonemeList.size()-1; i >=0; i--)
			{
				QDomElement phoneme = phonemeList.item(i).toElement();
				// Skip silence
				if( phoneme.attribute("value").compare("x") == 0 ) continue;
				TimeLineComboBox* phonemeBox = new TimeLineComboBox(phoneme, sentenceLength, m_phonemeList, m_phonemes);
				connect(phonemeBox, SIGNAL(xmlNodeChanged()), this, SLOT(enableSaveButton()) );
				connect(phonemeBox, SIGNAL(xmlNodeChanged()), wordBox, SLOT(updateFromXml()));
				phonemeBox->setVisible(true);
			}
		}
		// Get phonemes which don't belong to words
		else if( wordNode.nodeName().compare("phn") == 0 )
		{
			QDomElement phoneme = wordNode.toElement();
			// Skip silence
			if( phoneme.attribute("value").compare("x") != 0 )
			{
				TimeLineComboBox* phonemeBox = new TimeLineComboBox(phoneme, sentenceLength, m_phonemeList, m_phonemes);
				connect(phonemeBox, SIGNAL(xmlNodeChanged()), this, SLOT(enableSaveButton()) );
				phonemeBox->setVisible(true);
			}
		}

		wordNode = wordNode.previousSibling();
	}
}

void PhonemeEditorWidget::enableSaveButton()
{
	// TODO: Give SoundComponent a temporary phoneme file, so we don't have to save to see our changes

	// Enable save button
	m_saveButton->setEnabled(true);
}