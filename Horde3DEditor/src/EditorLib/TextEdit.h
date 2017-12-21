// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor.
//
// The EditorLib of the Horde3D Scene Editor is based on the Kombine project by Matus Tomlain.
// You can find the project under http://kombine.sourceforge.net/
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

#ifndef TEXTEDIT_H_
#define TEXTEDIT_H_

#include <QTextEdit>

class HighLighter;
class QShortcut;
class QCompleter;
class QKeyEvent;
class QStringListModel;
class QContextMenuEvent;
class QShortcut;

/**
 * Provides an editor interface for lua scripts
 */
class TextEdit : public QTextEdit
{
	Q_OBJECT
public:
	TextEdit(QWidget* parent = 0);
	virtual ~TextEdit();

	void setCompleterPhrases(const QStringList& phrases);
protected:
	void keyPressEvent(QKeyEvent *e);
	void contextMenuEvent( QContextMenuEvent *e );

private slots:
	void insertCompletion(const QString& completition);
	void appendCurrentWord();
	void increaseFont();
	void decreaseFont();
	void saveAs();
	void open();

private:
	void textrepaint();

	QString textUnderCursor() const;

	HighLighter*		m_highLighter;
	QCompleter*			m_completer;
	QStringListModel*	m_autoComplete;

	QShortcut*			m_zoomOut;
	QShortcut*			m_zoomIn;

	int					m_fontSize;
};
#endif
