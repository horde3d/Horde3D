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
#ifndef PHONEMEEDITORWIDGET_H_
#define PHONEMEEDITORWIDGET_H_

#include "Ui_PhonemeEditorWidget.h"

#include <QtXml/qdom.h>
#include <QtCore/qstringlist.h>

class QXmlTreeNode;

class PhonemeEditorWidget : public QWidget, protected Ui_PhonemeEditorWidget
{
	Q_OBJECT
public:
	PhonemeEditorWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~PhonemeEditorWidget();
	
	void loadPhonemeFile( unsigned int id, const QString& path );

public slots:
	void closePhonemeEditor();

signals:
	void closeEditor();

private slots:
	void playSound();
	void stopSound();
	void save();
	void parseXmlFile();
	void enableSaveButton();

private:
	unsigned int				m_entityWorldID;
	QString						m_phonemeFileName;
	QDomDocument				m_phonemeXml;

	QXmlTreeNode*				m_currentNode;

	static const QStringList	m_phonemeList;


};
#endif