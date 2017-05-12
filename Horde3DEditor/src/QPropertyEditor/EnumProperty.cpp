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
#include "EnumProperty.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////////////////////
EnumProperty::EnumProperty(const QString &name /* = QString()*/, 
						   QObject *propertyObject /* = 0*/, QObject *parent /* = 0*/)
: Property(name, propertyObject, parent)
{
	// get the meta property object
	const QMetaObject* meta = propertyObject->metaObject();
	QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(name)));

	// if it is indeed an enum type, fill the QStringList member with the keys
	if(prop.isEnumType()){
		QMetaEnum qenum = prop.enumerator();
		for(int i=0; i < qenum.keyCount(); i++){
			m_enum << qenum.key(i);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// value
/////////////////////////////////////////////////////////////////////////////////////////////
QVariant EnumProperty::value(int role /* = Qt::UserRole */) const {
	if(role == Qt::DisplayRole){
		if (m_propertyObject){
			// resolve the value to the corresponding enum key
			int index = m_propertyObject->property(qPrintable(objectName())).toInt();

			const QMetaObject* meta = m_propertyObject->metaObject();
			QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(objectName())));
			return QVariant(prop.enumerator().valueToKey(index));
		} else{
			return QVariant();
		}
	} else {
		return Property::value(role);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// createEditor
/////////////////////////////////////////////////////////////////////////////////////////////
QWidget* EnumProperty::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/){
	// create a QComboBox and fill it with the QStringList values
	QComboBox* editor = new QComboBox(parent);
	editor->addItems(m_enum);
	
	connect(editor, SIGNAL(currentIndexChanged(const QString)), 
		this, SLOT(valueChanged(const QString)));
	return editor;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// setEditorData
/////////////////////////////////////////////////////////////////////////////////////////////
bool EnumProperty::setEditorData(QWidget *editor, const QVariant &data)
{
	QComboBox* combo = 0;
        if( (combo = qobject_cast<QComboBox*>(editor) ) ){
		int value = data.toInt();
		const QMetaObject* meta = m_propertyObject->metaObject();
		QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(objectName())));
		
		int index = combo->findText(prop.enumerator().valueToKey(value));
		if(index == -1)
			return false;

		combo->setCurrentIndex(index);
	} else {
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// editorData
/////////////////////////////////////////////////////////////////////////////////////////////
QVariant EnumProperty::editorData(QWidget *editor)
{
	QComboBox* combo = 0;
        if( (combo = qobject_cast<QComboBox*>(editor) ) )
        {
		return QVariant(combo->currentText());
        }
        else
        {
		return QVariant();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// valueChanged
/////////////////////////////////////////////////////////////////////////////////////////////
void EnumProperty::valueChanged(const QString item){
	setValue(QVariant(item));
}
