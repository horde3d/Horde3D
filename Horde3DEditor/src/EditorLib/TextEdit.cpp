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

#include "TextEdit.h"
#include "HighLighter.h"

#include <QCompleter>
#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QMenu>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>

TextEdit::TextEdit(QWidget* parent /*= 0*/) : QTextEdit(parent), m_fontSize(10)
{	
	setFontPointSize(m_fontSize);
	m_highLighter = new HighLighter(document());
	m_autoComplete = new QStringListModel(this);
	m_completer = new QCompleter(m_autoComplete, this);
	m_completer->setWidget(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	QObject::connect(m_completer, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));
	m_zoomOut = new QShortcut(QKeySequence::ZoomOut,this);
	m_zoomIn = new QShortcut(QKeySequence::ZoomIn,this);
	connect(m_zoomOut, SIGNAL(activated()),this, SLOT(decreaseFont()));
	connect(m_zoomIn, SIGNAL(activated()),this, SLOT(increaseFont()));
}


TextEdit::~TextEdit()
{
}

void TextEdit::contextMenuEvent( QContextMenuEvent* /*e*/ )
{
	QMenu *menu = createStandardContextMenu();
	QString currentWord = textUnderCursor();
	if (!currentWord.isEmpty()) 
		menu->addAction(tr( "Append word \"%1\" to completion" ).arg(currentWord), this , SLOT( appendCurrentWord() ) );	 
	menu->addAction(tr( "Open..." ), this , SLOT( open() ));
	menu->addAction(tr( "Save..." ), this , SLOT( saveAs() ));
	//menu->addAction(tr( "Find text CTR+F" ), this , SLOT( QueryFind() ));
	menu->addAction(tr( "Zoom in CTRL+" ), this , SLOT( increaseFont() ));
	menu->addAction(tr( "Zoom out CTRL-" ), this , SLOT( decreaseFont() ));
	//menu->addAction(tr( "Close file F10" ), this , SLOT( closetabulatorhere() ));
	//menu->addAction(tr( "New File CTR+N" ), this , SLOT( NewFileMake() ));
	menu->exec( QCursor::pos() ); 
	delete menu;
} 

void TextEdit::increaseFont()
{
	if (m_fontSize < 55)
	{
		m_fontSize++;
	}
	textrepaint();
}

void TextEdit::decreaseFont()
{
	if (m_fontSize > 5) 
	{
		m_fontSize --;
	}
	textrepaint();
}

void TextEdit::textrepaint()
{
	selectAll();
	setFontPointSize(m_fontSize);
	toPlainText ();
	QTextCursor tc = textCursor();
	tc.movePosition(QTextCursor::Start);
	setTextCursor(tc);
}

void TextEdit::open()
{
	QString file = QFileDialog::getOpenFileName(this,tr("Select script file"), QDir::currentPath() ,tr("Lua file (*.lua);;All files (*.*)"));
	if ( !file.isEmpty() ) 
	{
		QFile f(file);
		if ( f.open( QFile::ReadOnly | QFile::Text ) )
		{
			QTextStream stream( &f );
			setPlainText(stream.readAll());
			f.close();						
		}
	}
}

void TextEdit::saveAs()
{	
	QString file = QFileDialog::getSaveFileName(this,tr("Save As ..."), QDir::currentPath() ,tr("Lua file (*.lua);;All files (*.*)"));
	if ( !file.isEmpty() ) 
	{
		QFile f(file);
		if ( f.open( QFile::WriteOnly | QFile::Text ) )
		{
			QTextStream stream( &f );
			stream << toPlainText();
			f.close();						
		}
	}
}

void TextEdit::appendCurrentWord()
{
	QString currentWord = textUnderCursor();
	QStringList phrases = m_autoComplete->stringList();
	if (!phrases.contains(currentWord))
	{
		phrases.push_back(currentWord);
		m_autoComplete->setStringList(phrases);
	}
}

void TextEdit::setCompleterPhrases(const QStringList& phrases)
{
	m_autoComplete->setStringList(phrases);
}

void TextEdit::insertCompletion(const QString& completion)
{
	QTextCursor tc = textCursor();
	int extra = completion.length() - m_completer->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion.right(extra));
	setTextCursor(tc);
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{

	if (m_completer && m_completer->popup()->isVisible()) 
	{
		// The following keys are forwarded by the completer to the widget
		switch (e->key()) 
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Backtab:
			e->ignore(); 
			return; // let the completer do default behavior			
		default:
			break;
		}
	}

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+E
	if (!m_completer || !isShortcut) // dont process the shortcut when we have a completer
		QTextEdit::keyPressEvent(e);
	
	// don't process event in parent widgets
	e->accept();
	
	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!m_completer || (ctrlOrShift && e->text().isEmpty()))
		return;

	// Add new words to db
	//QTextCursor tc(textCursor());
	//tc.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
	//tc.select(QTextCursor::WordUnderCursor);
	//QString previous(tc.selectedText());
	//qDebug("Previous Word: %s", qPrintable(previous));
	//QStringList& db(m_autoComplete->stringList());
	//if (previous.length() > 5 && !db.contains(previous))
	//{
	//	db.append(previous);
	//	qDebug("Word added: %s", qPrintable(previous));
	//	m_autoComplete->setStringList(db);
	//}


	static QString eow("~!@#$%^&*()_+{}|:\"<>?,/;'[]\\-="); // end of word
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 1	|| eow.contains(e->text().right(1)))) 
	{
		m_completer->popup()->hide();
		return;
	}

	if (completionPrefix != m_completer->completionPrefix()) 
	{
		m_completer->setCompletionPrefix(completionPrefix);
		m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
	}

	QRect cr = cursorRect();
	cr.setWidth(m_completer->popup()->sizeHintForColumn(0) + m_completer->popup()->verticalScrollBar()->sizeHint().width());
	m_completer->complete(cr); // popup it up!
}

QString TextEdit::textUnderCursor() const
{
	static QString eow("~!@#$%^&*()_+{}|:\"<>?,/;'[]\\-=\\n"); // end of word

	QTextCursor tc(textCursor());
	tc.select(QTextCursor::WordUnderCursor);
	QString word(tc.selectedText());

	// check if there is a dot connection
	tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
	tc.select(QTextCursor::WordUnderCursor);
	QString previous(tc.selectedText());
	QString prevResult;

	int count = 1;
	while (!previous.isEmpty() && !eow.contains(previous))
	{
		if (prevResult == previous) break;
		else prevResult = previous;

		if (previous == ".")
		{
			tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
			tc.select(QTextCursor::WordUnderCursor);
			previous = tc.selectedText();
			return previous + "." + word;
		}

		count++;
		tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, count);
		tc.select(QTextCursor::WordUnderCursor);
		previous = tc.selectedText();
	}
	
	return word;
}
