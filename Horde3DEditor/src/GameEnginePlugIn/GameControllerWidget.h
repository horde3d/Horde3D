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
#ifndef GAMECONTROLLERWIDGET_H_
#define GAMECONTROLLERWIDGET_H_

#include "ui_GameControllerWidget.h"


class QXmlTreeNode;

/**
 * This class handles the different GameController widgets that will be displayed depending on the current Horde3D node type
 */
class GameControllerWidget : public QWidget, protected Ui_GameControllerWidget
{
	Q_OBJECT
public:
	GameControllerWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~GameControllerWidget();

	void setCurrentNode(QXmlTreeNode* node);

	void init();
	void release();

signals:
	void modified(bool);
	void phonemeFileChanged();
	void updateFinished();

private slots:
	void addComponent();
	void removeComponent();

	void componentSelected(QXmlTreeNode* node);
	void entityChanged(const QModelIndex& first, const QModelIndex& last);
	void entityNameChanged(const QString& name);

	void switchPhonemeEditor(bool activate=false, unsigned int id=0, const QString& path=0);

	void emitModified(bool changed);

	void emitUpdateFinished();

private:
	void updateActions();

	QXmlTreeNode*	m_currentNode;
	QXmlTreeNode*	m_currentSceneNode;

};
#endif