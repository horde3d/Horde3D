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
#include "TTSWidget.h"

#include <QXmlTree/QXmlTreeNode.h>
#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEngine_SAPI.h>

#include <QtWidgets/qmessagebox.h>

#ifdef _WIN32
#include <sapi.h>
#include <sphelper.h>
#include <atlbase.h>
#endif

TTSWidget::TTSWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags)
{
	setupUi(this);

#ifdef _WIN32
	CoInitialize(NULL);
	HRESULT                             hr = S_OK;
	CComPtr<ISpObjectToken>             cpVoiceToken;
	CComPtr<IEnumSpObjectTokens>        cpEnum;
	CComPtr<ISpVoice>                   cpVoice;
	ULONG                               ulCount = 0;
	
	// Create the SAPI voice
	if(SUCCEEDED(hr))
		hr = cpVoice.CoCreateInstance( CLSID_SpVoice ); 
	//Enumerate the available voices 
	if(SUCCEEDED(hr))
		hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
	//Get the number of voices
	if(SUCCEEDED(hr))
		hr = cpEnum->GetCount(&ulCount);
	// Obtain a list of available voice tokens
	while (SUCCEEDED(hr) && ulCount-- )
	{
		cpVoiceToken.Release();
		if(SUCCEEDED(hr))
			hr = cpEnum->Next( 1, &cpVoiceToken, NULL );
		WCHAR* voiceName = 0;
		if (SUCCEEDED(SpGetDescription(cpVoiceToken, &voiceName)))
		{
			m_voice->addItem(QString::fromUtf16((const ushort*) voiceName));
			CoTaskMemFree(voiceName);
		}
	}
	CoUninitialize();
#endif

	connect(m_voice, SIGNAL(currentIndexChanged(int)), this, SLOT(voiceChanged(int)));
	connect(m_speakButton, SIGNAL(clicked()), this, SLOT(speak()));	
}


TTSWidget::~TTSWidget()
{
}

bool TTSWidget::setCurrentNode(QXmlTreeNode* node)
{
	m_currentNode = 0;
	if (node && (node->xmlNode().tagName() == "TTS" || node->xmlNode().tagName() == "Sapi"))
	{						
		if (node->xmlNode().attribute("voice").isEmpty())
			node->xmlNode().setAttribute("voice", "Microsoft Sam");
		int index = m_voice->findText(node->xmlNode().attribute("voice"));
		/*// TODO: Message box produces and error-state for the node selection --> transformation changes won't work
		if (index == -1)
			QMessageBox::information(this, tr("Error"), tr("The associated voice %1 couldn't be found").arg(node->xmlNode().attribute("voice")));*/
		m_voice->setCurrentIndex(index);
		m_currentNode = node;
	}
	else
		return false;
	return true;
}


void TTSWidget::voiceChanged(int index)
{
	if (m_currentNode && index != -1)
	{
		GameEngine::setVoice(
			GameEngine::entityWorldID(qPrintable(m_currentNode->xmlNode().parentNode().toElement().attribute("name"))), 
			qPrintable(m_voice->currentText()));
		m_currentNode->xmlNode().setAttribute("voice", m_voice->currentText());
		emit modified(true);
	}
}

void TTSWidget::speak()
{
	if (m_currentNode)
	{
		GameEngine::speak(
			GameEngine::entityWorldID(qPrintable(m_currentNode->xmlNode().parentNode().toElement().attribute("name"))),
			qPrintable(m_sentence->toPlainText()));
	}
}


