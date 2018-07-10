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


// ****************************************************************************************
//
// GameEngine Horde3D Editor Plugin of the University of Augsburg
// Phoneme Editor
// ---------------------------------------------------------
// Copyright (C) 2008 Felix Kistler
// 
// ****************************************************************************************
#ifndef TIMELINETEXTEDIT_H_
#define TIMELINETEXTEDIT_H_

#include <QtXml/qdom.h>
#include <QtCore/qstringlist.h>
#include <QtWidgets/QWidget.h>
#include <QtWidgets/QTextEdit.h>

class TimeLineTextEdit : public QTextEdit
{
	Q_OBJECT

	Q_PROPERTY(QString value READ value WRITE setValue DESIGNABLE true USER true)
	Q_PROPERTY(int start READ start WRITE setStart DESIGNABLE true USER true)
	Q_PROPERTY(int end READ end WRITE setEnd DESIGNABLE true USER true)

public:
	TimeLineTextEdit(const QDomElement& xmlNode, int maxTime, QWidget* parent = 0);
	virtual ~TimeLineTextEdit();

	void setValue(const QString& value);
	QString value() const {return m_value;}
	void setStart(int start);
	int start() const {return m_start;}
	void setEnd(int end);
	int end() const {return m_end;}

	QDomElement& xmlNode() {return m_xmlNode;}

signals:
	void xmlNodeChanged();

private slots:
	void currentTextChanged();
	void updateFromXml();


private:
	int				m_maxTime, m_start, m_end;
	QString			m_value;
	QDomElement		m_xmlNode;
	
};
#endif
