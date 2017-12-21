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

#include "TimeLineComboBox.h"

#include <QtCore/QPoint.h>
#include <QtGui/QCursor.h>
#include <QtGui/QEvent.h>

TimeLineComboBox::TimeLineComboBox(const QDomElement &xmlNode, int maxTime, 
	const QStringList items /*=QStringList()*/, TimeLineFrame * parent /*=0*/) : QComboBox(parent),
	m_xmlNode(xmlNode), m_maxTime(maxTime), m_dragging(false), m_sizingLeft(false), m_sizingRight(false)
{
	addItems(items);
	m_value = QString(m_xmlNode.attribute("value"));
	m_start = m_xmlNode.attribute("start", "0").toInt();
	m_end = m_xmlNode.attribute("end", "0").toInt();
	setCurrentIndex(findText(m_value));
	if( m_maxTime != 0 && parent )
	{
		setGeometry( (int)( ((float)m_start/m_maxTime) * (parent->width()-2) + 1.5f ), 1,
		(int)( ((float)(m_end-m_start)/m_maxTime) * (parent->width()-2) + 0.5f ), parent->height()-2 );
	}

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)) );
	connect(parent, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
	connect(parent, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
	connect(parent, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
	setMouseTracking(true);
}

TimeLineComboBox::~TimeLineComboBox()
{
}

void TimeLineComboBox::setValue(const QString& value)
{
	int index = findText(value);
	if( index != -1)
	{
		m_xmlNode.setAttribute("value", value);
		m_value = QString(value);
		setCurrentIndex(index);
		emit xmlNodeChanged();
	}
}

void TimeLineComboBox::indexChanged(int index)
{
	m_xmlNode.setAttribute("value", itemText(index));
	m_value = QString(itemText(index));
	emit xmlNodeChanged();
}

void TimeLineComboBox::mouseMoveEvent(QMouseEvent *event)
{
	mouseMove(event, true);
}

void TimeLineComboBox::mousePressEvent(QMouseEvent *event)
{
	mousePress(event, true);
}

void TimeLineComboBox::mouseReleaseEvent(QMouseEvent *event)
{
	mouseRelease(event, true);
}

// Mouse event callback from parent frame
void TimeLineComboBox::mouseMove(QMouseEvent* event, bool comboBoxEvent/* = false*/)
{
	QPoint pos = mapFromGlobal(event->globalPos());

	
	if( m_dragging )
	{
		int dist = event->globalPos().x()-m_dragStartX;
		if( m_dragStartGeoX+dist > 0 && m_dragStartGeoX+dist+width() < parentWidget()->width())
			setGeometry(m_dragStartGeoX+dist, y(), width(), height());
	}
	else if( m_sizingLeft)
	{
		int dist = event->globalPos().x()-m_sizingStartX;
		if( m_sizingStartGeoX+dist > 0 && m_sizingStartGeoW-dist >= 40)
			setGeometry(m_sizingStartGeoX+dist, y(), m_sizingStartGeoW-dist, height());
	}
	else if( m_sizingRight )
	{
		int dist = event->globalPos().x()-m_sizingStartX;
		if( m_sizingStartGeoW+dist >= 40 && m_sizingStartGeoW+dist+x() < parentWidget()->width())
			setGeometry(x(), y(), m_sizingStartGeoW+dist, height());
	}
	else
	{
		// Set cursor according to position
		if( (pos.x() < 4 && pos.x() >= -1)  ||  (pos.x() > width()-4 && pos.x() <= width()+1) ) 
			QWidget::setCursor(Qt::SizeHorCursor);
		else if( pos.x() < width()-10  &&  pos.x() > 10 )
			QWidget::setCursor(Qt::SizeAllCursor);
		else
			QWidget::unsetCursor();
	}
}
void TimeLineComboBox::mousePress(QMouseEvent* event, bool comboBoxEvent/* = false*/)
{
	if(event->button() == Qt::RightButton) {
		//TODO show context menu for deleting item
     }
	else if(event->button() == Qt::LeftButton) {
		QPoint pos = mapFromGlobal(event->globalPos());

		if( (pos.x() < 4 && pos.x() >= -1) )
		{
			// enable sizing left
			m_sizingLeft = true;
			m_sizingStartX = event->globalPos().x();
			m_sizingStartGeoX = x();
			m_sizingStartGeoW = width();
		}
		else if((pos.x() > width()-4 && pos.x() <= width()+1))
		{
			// enable sizing right
			m_sizingRight = true;
			m_sizingStartX = event->globalPos().x();
			m_sizingStartGeoX = x();
			m_sizingStartGeoW = width();
		}
		else if( pos.x() < width()-20  &&  pos.x() > 10 )
		{
			// enable dragging
			m_dragging = true;
			m_dragStartX = event->globalPos().x();
			m_dragStartGeoX = x();
		}
		else if(comboBoxEvent)
			// Needed for combobox interaction
			QComboBox::mousePressEvent(event);
	}
	else if(comboBoxEvent)
         QComboBox::mousePressEvent(event);
}
void TimeLineComboBox::mouseRelease(QMouseEvent* event, bool comboBoxEvent/* = false*/)
{
	if(m_dragging)
	{
		// Save geometry changes from dragging
		m_start = (int)((x() - 1.5f) / (float)(parentWidget()->width()-2) * m_maxTime);
		m_xmlNode.setAttribute("start", m_start);
		m_end = (int)(( (width()-0.5f) / (float)(parentWidget()->width()-2)* m_maxTime ) + m_start);
		m_xmlNode.setAttribute("end", m_end);
		
		emit xmlNodeChanged();
		m_dragging = false;
	}
	if(m_sizingLeft)
	{
		// Save geometry changes from sizing
		m_start = (int)((x() - 1.5f) / (float)(parentWidget()->width()-2) * m_maxTime);
		m_xmlNode.setAttribute("start", m_start);

		emit xmlNodeChanged();
		m_sizingLeft = false;
	}
	if(m_sizingRight)
	{
		// Save geometry changes from sizing
		m_end = (int)(( (width()-0.5f) / (float)(parentWidget()->width()-2)* m_maxTime ) + m_start);
		m_xmlNode.setAttribute("end", m_end);

		emit xmlNodeChanged();
		m_sizingRight = false;
	}
}