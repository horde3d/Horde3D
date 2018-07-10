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

#include "MaterialProperty.h"
#include "MaterialComboBox.h"
#include "CustomTypes.h"

#include <QtCore/QDir>

MaterialProperty::MaterialProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) : Property(name, propertyObject, parent)
{	
}


MaterialProperty::~MaterialProperty()
{
}

QVariant MaterialProperty::value(int role) const
{
	QVariant data = Property::value();
	if (data.isValid() && role == Qt::DisplayRole)	
		return data.value<Material>().FileName;	
	return data;
}

QWidget* MaterialProperty::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/)
{
	MaterialComboBox* editor = new MaterialComboBox(parent);	
	editor->init( QDir::currentPath() );
	return editor;
}

QVariant MaterialProperty::editorData(QWidget *editor)
{
	return QVariant::fromValue(static_cast<MaterialComboBox*>(editor)->material());
}

bool MaterialProperty::setEditorData(QWidget *editor, const QVariant &data)
{
	MaterialComboBox* ed = qobject_cast<MaterialComboBox*>(editor);
	if (ed)
		static_cast<MaterialComboBox*>(editor)->setMaterial(data.value<Material>());
	return true;
}
