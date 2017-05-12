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

///////////////////////////////////////////////////////////
//  CheckBoxList.cpp
//  Implementation of the Class CheckBoxList
//  Created on:      02-Jun-2008 6:53:56 PM
//  Original author: Nasser M. Al-Ansari (Da-Crystal)
///////////////////////////////////////////////////////////
//	Modification History:
//
//	Who(SNo)			Date			Description
//  Volker Wiendl	    3.5.2009        Added stateChanged signal
//	
///////////////////////////////////////////////////////////

#include "CheckBoxList.h"
#include <QItemDelegate>
#include <QApplication>
#include <QCheckBox>
#include <QAbstractItemView>
#include <QStylePainter>

// internal private delegate
class CheckBoxListDelegate : public QItemDelegate
{
public:
    CheckBoxListDelegate(QObject *parent) : QItemDelegate(parent)
	{
	}

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
	{
		//Get item data
		bool value = index.data(Qt::UserRole).toBool();
		QString text = index.data(Qt::DisplayRole).toString();
		
		// fill style options with item data
		const QStyle *style = QApplication::style();
		QStyleOptionButton opt;
		opt.state |= value ? QStyle::State_On : QStyle::State_Off;
		opt.state |= QStyle::State_Enabled;
		opt.text = text;
		opt.rect = option.rect;

		// draw item data as CheckBox
		style->drawControl(QStyle::CE_CheckBox,&opt,painter);
	}

    QWidget *createEditor(QWidget *parent,
		 const QStyleOptionViewItem & option ,
		 const QModelIndex & index ) const
	{
		// create check box as our editor
		 QCheckBox *editor = new QCheckBox(parent);
		 return editor;
	}

	 void setEditorData(QWidget *editor,
										 const QModelIndex &index) const
	 {
		 //set editor data
		 QCheckBox *myEditor = static_cast<QCheckBox*>(editor);
		 myEditor->setText(index.data(Qt::DisplayRole).toString());
		 myEditor->setChecked(index.data(Qt::UserRole).toBool());
		 myEditor->setProperty( "int", index.data( Qt::UserRole + 1 ) );
	 }

	 void setModelData(QWidget *editor, QAbstractItemModel *model,
										const QModelIndex &index) const
	 {
		 //get the value from the editor (CheckBox)
		 QCheckBox *myEditor = static_cast<QCheckBox*>(editor);
		 bool value = myEditor->isChecked();

		 //set model data
		 QMap<int,QVariant> data;
		 data.insert(Qt::DisplayRole,myEditor->text());
		 data.insert(Qt::UserRole,value);
		 data.insert(Qt::UserRole + 1, myEditor->property( "int" ) );
		 model->setItemData(index,data);
	 }

	 void updateEditorGeometry(QWidget *editor,
		 const QStyleOptionViewItem &option, const QModelIndex &index ) const
	 {
		 editor->setGeometry(option.rect);
	 }
 }; 



CheckBoxList::CheckBoxList(QWidget *widget )
:QComboBox(widget),m_DisplayText((const char *)0)
{
	CheckBoxListDelegate* itemDelegate = new CheckBoxListDelegate(this);
	//connect( itemDelegate, SIGNAL( commitData( QWidget* ) ), this, SLOT( dataEdited( QWidget* ) ) );
	connect( model(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT(dataChanged( const QModelIndex&, const QModelIndex& ) ) );
	// set delegate items view 
	view()->setItemDelegate( itemDelegate );
	
	// Enable editing on items view
	view()->setEditTriggers(QAbstractItemView::CurrentChanged);
	
	// set "CheckBoxList::eventFilter" as event filter for items view 
	view()->viewport()->installEventFilter(this);
	
	// it just cool to have it as defualt ;)
	view()->setAlternatingRowColors(true);
}


CheckBoxList::~CheckBoxList()
{
}


bool CheckBoxList::eventFilter(QObject *object, QEvent *event)
{
	// don't close items view after we release the mouse button
	// by simple eating MouseButtonRelease in viewport of items view
	if(event->type() == QEvent::MouseButtonRelease && object==view()->viewport()) 
	{
		return true;
	}
	return QComboBox::eventFilter(object,event);
}


void CheckBoxList::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);

	// if no display text been set , use "..." as default
	if(m_DisplayText.isNull())
		opt.currentText = "...";
	else
		opt.currentText = m_DisplayText;
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}


void CheckBoxList::setDisplayText( const QString& text )
{
	m_DisplayText = text;
}

QString CheckBoxList::getDisplayText() const
{
	return m_DisplayText;
}

void CheckBoxList::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	emit stateChanged( topLeft.row(), itemData( topLeft.row() ).toBool() );
}