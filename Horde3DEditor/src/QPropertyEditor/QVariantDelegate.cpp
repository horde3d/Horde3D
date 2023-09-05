// *************************************************************************************************
//
// QPropertyEditor v 0.3
//   
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// Acknowledgements to Roman alias banal from qt-apps.org for the Enum enhancement
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************



#include "QVariantDelegate.h"

#include "Property.h"

#include <QAbstractItemView>
#include <QSignalMapper>

#include <QtGlobal>
#if QT_VERSION >= 0x060000
    #include "QRegExp.h"
#else
    #include <QtCore/QRegExp>
#endif


QVariantDelegate::QVariantDelegate(QObject* parent) : QItemDelegate(parent)
{
	m_finishedMapper = new QSignalMapper(this);
	connect(m_finishedMapper, &QSignalMapper::mappedObject, this, &QVariantDelegate::onEditorFinished);
//	connect(m_finishedMapper, &QSignalMapper::mappedObject, this, &QVariantDelegate::closeEditor);
	
	//	connect(m_finishedMapper, SIGNAL(mappedObject(QWidget*)), this, SIGNAL(closeEditor(QWidget*)));
}

QVariantDelegate::~QVariantDelegate()
{
}

QWidget *QVariantDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& option , const QModelIndex & index ) const
{
	QWidget* editor = 0;
	Property* p = static_cast<Property*>(index.internalPointer());
	int propType = p->value().typeId();

	if (propType >= QMetaType::User)
	{
		// handle user properties
		editor = p->createEditor(parent, option);
		if (editor)
		{
			if (editor->metaObject()->indexOfSignal("editFinished()") != -1)
			{
				connect(editor, SIGNAL(editFinished()), m_finishedMapper, SLOT(map()));
				m_finishedMapper->setMapping(editor, editor);
			}
		}
		else
			if (!editor) editor = QItemDelegate::createEditor(parent, option, index);
	}
	else
	{
		switch (propType)
		{
		case QMetaType::QColor:
		case QMetaType::Int:
		case QMetaType::Float:
		case QMetaType::Double:
			editor = p->createEditor(parent, option);
			if (editor)
			{
				if (editor->metaObject()->indexOfSignal("editFinished()") != -1)
				{
					connect(editor, SIGNAL(editFinished()), m_finishedMapper, SLOT(map()));
					m_finishedMapper->setMapping(editor, editor);
				}
				break; // if no editor could be created take default case
			}
		default:
			if (!editor) editor = QItemDelegate::createEditor(parent, option, index);
		}
	}
	
	parseEditorHints(editor, p->editorHints());
	return editor;
}

void QVariantDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{		
	m_finishedMapper->blockSignals(true);
    editor->blockSignals(true);
	QVariant data = index.model()->data(index, Qt::EditRole);	
	
	if (data.typeId() >= QMetaType::User)
	{
		if ( !static_cast<Property*>(index.internalPointer())->setEditorData(editor, data) ) // if editor couldn't be recognized use default
			QItemDelegate::setEditorData(editor, index);
	}
	else
	{
		switch (data.typeId())
		{
		case QMetaType::QColor:
		case QMetaType::Int:
		case QMetaType::Float:
		case QMetaType::Double:
			if (static_cast<Property*>(index.internalPointer())->setEditorData(editor, data)) // if editor couldn't be recognized use default
				break;
		default:
			QItemDelegate::setEditorData(editor, index);
			break;
		}
	}

    editor->blockSignals(false);
	m_finishedMapper->blockSignals(false);    
}

void QVariantDelegate::onEditorFinished( QObject *obj )
{
	QWidget *editor = static_cast< QWidget * >( obj );
	emit commitData( editor );
	emit closeEditor( editor );
}

void QVariantDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{	
	QVariant data = index.model()->data(index, Qt::EditRole);	
    unsigned int type = data.typeId();

	if ( type < QMetaType::User )
	{
		if (type != QMetaType::QColor ||
			type != QMetaType::Int ||
			type != QMetaType::Float ||
			type != QMetaType::Double)
		{
			QItemDelegate::setModelData(editor, model, index);
			return;
		}
	}	

    data = static_cast<Property*>(index.internalPointer())->editorData(editor);
    if (data.isValid())
        model->setData(index, data , Qt::EditRole);
}

void QVariantDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& index ) const
{
	return QItemDelegate::updateEditorGeometry(editor, option, index);
}

void QVariantDelegate::parseEditorHints(QWidget* editor, const QString& editorHints) const
{
	if (editor && !editorHints.isEmpty())
	{
		editor->blockSignals(true);
		// Parse for property values
		QRegExp rx("(.*)(=\\s*)(.*)(;{1})");
		rx.setMinimal(true);
		int pos = 0;
		while ((pos = rx.indexIn(editorHints, pos)) != -1) 
		{
			//qDebug("Setting %s to %s", qPrintable(rx.cap(1)), qPrintable(rx.cap(3)));
			editor->setProperty(qPrintable(rx.cap(1).trimmed()), rx.cap(3).trimmed());				
			pos += rx.matchedLength();
		}
		editor->blockSignals(false);
	}
}
