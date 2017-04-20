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

#ifndef VEC3FPROPERTY_H_
#define VEC3FPROPERTY_H_

#include <QPropertyEditor/Property.h>

class Vec3fProperty : public Property
{
	Q_OBJECT
	Q_PROPERTY(float x READ x WRITE setX DESIGNABLE true USER true)
	Q_PROPERTY(float y READ y WRITE setY DESIGNABLE true USER true)
	Q_PROPERTY(float z READ z WRITE setZ DESIGNABLE true USER true)

public:
	Vec3fProperty(const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);

	QVariant value(int role = Qt::UserRole) const;
	virtual void setValue(const QVariant& value);

	void setEditorHints(const QString& hints);

	float x() const;
	void setX(float x);

	float y() const;
	void setY(float y);

	float z() const;
	void setZ(float z);

private:
	QString parseHints(const QString& hints, const QChar component);

	Property*	m_x;
	Property*	m_y;
	Property*	m_z;
};
#endif
