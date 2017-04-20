// *************************************************************************************************
//
// QPropertyEditor v 0.3
//   
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// Acknowledgements to Roman alias banal from qt-apps.org for the Enum enhancement
//
//
// The QPropertyEditor Library is free software; you can redistribute it and/or modify
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
// *************************************************************************************************
#include "Property.h"
#include "ColorCombo.h"

#include <QMetaProperty>
#include <QSpinBox>

#include <limits.h>

Property::Property(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) : QObject(parent), 
m_propertyObject(propertyObject)
{
	setObjectName(name);
}

QVariant Property::value(int /*role = Qt::UserRole*/) const
{
	if (m_propertyObject)
		return m_propertyObject->property(qPrintable(objectName()));
	else
		return QVariant();
}

void Property::setValue(const QVariant &value)
{
	if (m_propertyObject)
		m_propertyObject->setProperty(qPrintable(objectName()), value);
}

bool Property::isReadOnly()
{
	if( m_propertyObject->dynamicPropertyNames().contains( objectName().toLocal8Bit() ) )
		return false;
	if (m_propertyObject && m_propertyObject->metaObject()->property(m_propertyObject->metaObject()->indexOfProperty(qPrintable(objectName()))).isWritable())
		return false;
	else
		return true;
}

QWidget* Property::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/)
{
	QWidget* editor = 0;
	switch(value().type())
	{
	case QVariant::Color:
		editor = new ColorCombo(parent);
		break;
	case QVariant::Int:
		editor = new QSpinBox(parent);
		editor->setProperty("minimum", -INT_MAX);
		editor->setProperty("maximum", INT_MAX);
		connect(editor, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
		break;
	case QMetaType::Float:	
	case QVariant::Double:	
		editor = new QDoubleSpinBox(parent);
		editor->setProperty("minimum", -INT_MAX);
		editor->setProperty("maximum", INT_MAX);
		connect(editor, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
		break;			
	default:
		return editor;
	}
	return editor;
}

bool Property::setEditorData(QWidget *editor, const QVariant &data)
{
	switch(value().type())
	{
	case QVariant::Color:
		static_cast<ColorCombo*>(editor)->setColor(data.value<QColor>());
		return true;;
	case QVariant::Int:
		editor->blockSignals(true);
		static_cast<QSpinBox*>(editor)->setValue(data.toInt());
		editor->blockSignals(false);
		return true;			
	case QMetaType::Float:	
	case QVariant::Double:	
		editor->blockSignals(true);
		static_cast<QDoubleSpinBox*>(editor)->setValue(data.toDouble());
		editor->blockSignals(false);
		return true;		
	default: 
		return false;	
	}
	return false;
}

QVariant Property::editorData(QWidget *editor)
{
	switch(value().type())
	{
	case QVariant::Color:
		return QVariant::fromValue(static_cast<ColorCombo*>(editor)->color());
	case QVariant::Int:
		return QVariant(static_cast<QSpinBox*>(editor)->value());
	case QMetaType::Float:	
	case QVariant::Double:	
		return QVariant(static_cast<QDoubleSpinBox*>(editor)->value());
		break;			
	default:
		return QVariant();
	}
}

Property* Property::findPropertyObject(QObject* propertyObject)
{
	if (m_propertyObject == propertyObject)
		return this;
	for (int i=0; i<children().size(); ++i)
	{
		Property* child = static_cast<Property*>(children()[i])->findPropertyObject(propertyObject);
		if (child)
			return child;
	}
	return 0;
}

void Property::setValue(double value)
{
	setValue(QVariant(value));
}

void Property::setValue(int value)
{
	setValue(QVariant(value));
}
