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
#ifndef SOUNDWIDGET_H_
#define SOUNDWIDGET_H_

#include "Ui_SoundWidget.h"

class QXmlTreeNode;

class SoundWidget : public QWidget, protected Ui_SoundWidget
{
	Q_OBJECT
public:
	SoundWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~SoundWidget();

	bool setCurrentNode(QXmlTreeNode* node);


signals:
	void modified(bool);
	void activatePhonemeEditor(bool activate, unsigned int id, const QString& path);

private slots:
	void scanMediaDir( const QString& path );
	void addFiles();
	void updateSoundFile( const QString& soundFile );
	void gainChanged(double value);
	void pitchChanged(double value);
	void refDistChanged(double value);
	void maxDistChanged(double value);
	void loopChanged();
	void rollOffChanged(double value);
	void addPhonemeFiles();
	void scanPhonemeFiles( const QString& path  );
	void updatePhonemeFile( const QString& phonemeFile );
	void playSound();
	void stopSound();
	void openPhonemeEditor();
	void offsetChanged(double value);
	void visemeBlendSpeedChanged(double value);
	void forceNoStreamChanged();
	void autoStartChanged();

private:
	unsigned int entityWorldID();

	QXmlTreeNode*	m_currentNode;

};
#endif