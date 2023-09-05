// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Vogelhuber
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

#include "FrustumProperty.h"
#include "CustomTypes.h"

#include <QtGlobal>
#if QT_VERSION >= 0x060000
    #include "../EditorLib/QRegExp.h"
#else
    #include <QtCore/QRegExp>
#endif

FrustumProperty::FrustumProperty(const QString& name /*= QString()*/, QObject* propertyObject /*= 0*/, QObject* parent /*= 0*/) : Property(name, propertyObject, parent)
{
	m_left = new Property("Left", this, this);
	m_left->setEditorHints("minimum=-2147483647;maximum=2147483647;decimals=7;singleStep=0.0001;");
	m_right = new Property("Right", this, this);
	m_right->setEditorHints("minimum=-2147483647;maximum=2147483647;decimals=7;singleStep=0.0001;");
	m_bottom = new Property("Bottom", this, this);
	m_bottom->setEditorHints("minimum=-2147483647;maximum=2147483647;decimals=7;singleStep=0.0001;");
	m_top = new Property("Top", this, this);
	m_top->setEditorHints("minimum=-2147483647;maximum=2147483647;decimals=7;singleStep=0.0001;");
	m_near = new Property("Near", this, this);
	m_near->setEditorHints("minimum=0.0001;maximum=2147483647;decimals=3;singleStep=1.0;");
	m_far = new Property("Far", this, this);
	m_far->setEditorHints("minimum=0.001;maximum=2147483647;decimals=3;singleStep=1.0;");
		
}


FrustumProperty::~FrustumProperty()
{
}

QVariant FrustumProperty::value(int role) const
{
	QVariant data = Property::value();
	Frustum frustum = data.value<Frustum>();
	if (data.isValid() && role != Qt::UserRole)
	{
		switch (role)
		{
		case Qt::DisplayRole: // Show frustum in QPropertyEditorWidget
			if (frustum.isSymetric())
				return tr("[ %1%2, %3, %4, %5]").arg(frustum.Fov()).arg(QChar(176)).arg(frustum.Aspect()).arg(frustum.Near).arg(frustum.Far);
			else
				return tr("[ %1, %2, %3, %4, %5, %6]").arg(frustum.Left).arg(frustum.Right).arg(frustum.Bottom).arg(frustum.Top).arg(frustum.Near).arg(frustum.Far);
		case Qt::EditRole: // Show frustum in Lineedit
			if (frustum.isSymetric())
				return tr("%1%2, %3, %4, %5").arg(frustum.Fov()).arg(QChar(176)).arg(frustum.Aspect()).arg(frustum.Near).arg(frustum.Far);
			else
				return tr("%1 %2 %3 %4 %5 %6").arg(frustum.Left).arg(frustum.Right).arg(frustum.Bottom).arg(frustum.Top).arg(frustum.Near).arg(frustum.Far);
		};
	}
	return data;
}

void FrustumProperty::setValue(const QVariant& value)
{
	if (value.typeId() == QMetaType::QString)
	{
		QString v = value.toString();				
		QRegExp rx("([+-]?([0-9]*[\\.,])?[0-9]+(e[+-]?[0-9]+)?)");
		rx.setCaseSensitivity(Qt::CaseInsensitive);
		int count = 0;
		int pos = 0;
		float frustum[6];		
		while ((pos = rx.indexIn(v, pos)) != -1) 
		{
			if (count < 6)
				frustum[count++] = rx.cap(1).toDouble();						
			else
				break;
			pos += rx.matchedLength();			
		}
		if (count == 4) // Symetric frustum
		{						
			float ymax = frustum[2] * tanf( frustum[0] * 3.1415926 / 360.0f ) ;
			float xmax = ymax * frustum[1];
			Property::setValue(QVariant::fromValue(Frustum(-xmax, xmax, -ymax, ymax, frustum[2], frustum[3])));
		}
		else if (count == 7)
			Property::setValue(QVariant::fromValue(Frustum(frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5])));		
	}
	else
		Property::setValue(value);
}

float FrustumProperty::left() const
{
	return value().value<Frustum>().Left;
}

void FrustumProperty::setLeft(float left)
{
	Property::setValue(QVariant::fromValue(Frustum(left, right(), bottom(), top(), nearPlane(), farPlane())));
}

float FrustumProperty::right() const
{
	return value().value<Frustum>().Right;
}

void FrustumProperty::setRight(float right)
{
	Property::setValue(QVariant::fromValue(Frustum(left(), right, bottom(), top(), nearPlane(), farPlane())));
}

float FrustumProperty::bottom() const
{
	return value().value<Frustum>().Bottom;
}

void FrustumProperty::setBottom(float bottom)
{
	Property::setValue(QVariant::fromValue(Frustum(left(), right(), bottom, top(), nearPlane(), farPlane())));
}


float FrustumProperty::top() const
{
	return value().value<Frustum>().Top;
}

void FrustumProperty::setTop(float top)
{
	Property::setValue(QVariant::fromValue(Frustum(left(), right(), bottom(), top, nearPlane(), farPlane())));
}

float FrustumProperty::nearPlane() const
{
	return value().value<Frustum>().Near;
}

void FrustumProperty::setNearPlane(float nearPlane)
{
	Property::setValue(QVariant::fromValue(Frustum(left(), right(), bottom(), top(), nearPlane, farPlane())));
}

float FrustumProperty::farPlane() const
{
	return value().value<Frustum>().Far;
}

void FrustumProperty::setFarPlane(float farPlane)
{
	Property::setValue(QVariant::fromValue(Frustum(left(), right(), bottom(), top(), nearPlane(), farPlane)));
}
