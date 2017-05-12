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
#ifndef TIMELINECOMBOBOX_H_
#define TIMELINECOMBOBOX_H_

#include <QtXml/qdom.h>
#include <QtCore/qstringlist.h>
#include <QtWidgets/QWidget.h>
#include <QtWidgets/QComboBox.h>
#include "TimeLineFrame.h"

class TimeLineComboBox : public QComboBox
{
	Q_OBJECT

	Q_PROPERTY(QString value READ value WRITE setValue DESIGNABLE true USER true)

public:
	TimeLineComboBox(const QDomElement& xmlNode, int maxTime, const QStringList items = QStringList(), TimeLineFrame* parent = 0);
	virtual ~TimeLineComboBox();

	void setValue(const QString& value);
	QString value() const {return m_value;}
	QDomElement& xmlNode() {return m_xmlNode;}

public slots:
	void mouseMove(QMouseEvent* event, bool comboBoxEvent = false);
	void mousePress(QMouseEvent* event, bool comboBoxEvent = false);
	void mouseRelease(QMouseEvent* event, bool comboBoxEvent = false);

signals:
	void xmlNodeChanged();

private slots:
	void indexChanged(int index);

protected:
	void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	
	// Timeline values and content
	int				m_maxTime, m_start, m_end;
	QString			m_value;
	QDomElement		m_xmlNode;
	
	// Drag variables
	bool			m_dragging;
	int				m_dragStartX;
	int				m_dragStartGeoX;

	// Sizing variables#
	bool			m_sizingLeft;
	bool			m_sizingRight;
	int				m_sizingStartX;
	int				m_sizingStartGeoX;
	int				m_sizingStartGeoW;
	
};
#endif
