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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>
#include <QtCore/QHash>
#include <QtGui/QTextFormat>
#include <QtCore/QStringList>

#include <QtGlobal>
#if QT_VERSION >= 0x060000
    #include "QRegExp.h"
#else
    #include <QtCore/QRegExp>
#endif

class QTextDocument;

/**
 * Syntax Highlighter class for the Lua Script Editor
 */
class HighLighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    HighLighter(QTextDocument *parent = 0);
	
	void setKeywordPattern(QStringList keywordPatterns);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif
