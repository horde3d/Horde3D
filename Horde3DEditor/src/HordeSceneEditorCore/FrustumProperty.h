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

#ifndef FRUSTUMPROPERTY_H_
#define FRUSTUMPROPERTY_H_

#include <QPropertyEditor/Property.h>

class FrustumProperty : public Property
{
	Q_OBJECT
	Q_PROPERTY(float Left READ left WRITE setLeft DESIGNABLE true USER true)
	Q_PROPERTY(float Right READ right WRITE setRight DESIGNABLE true USER true)
	Q_PROPERTY(float Bottom READ bottom WRITE setBottom DESIGNABLE true USER true)
	Q_PROPERTY(float Top READ top WRITE setTop DESIGNABLE true USER true)
	Q_PROPERTY(float Near READ nearPlane WRITE setNearPlane DESIGNABLE true USER true)
	Q_PROPERTY(float Far READ farPlane WRITE setFarPlane DESIGNABLE true USER true)

public:
	FrustumProperty(const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);
	virtual ~FrustumProperty();

	QVariant value(int role = Qt::UserRole) const;
	void setValue(const QVariant& value);

	//QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option);
	//
	//QVariant editorData(QWidget *editor);
	//bool setEditorData(QWidget *editor, const QVariant& data);
	float left() const;
	void setLeft(float left);

	float right() const;
	void setRight(float right);

	float bottom() const;
	void setBottom(float bottom);

	float top() const;
	void setTop(float top);

	float nearPlane() const;
	void setNearPlane(float nearPlane);

	float farPlane() const;
	void setFarPlane(float farPlane);


private:
	Property*	m_left;
	Property*	m_right;
	Property*	m_bottom;
	Property*	m_top;
	Property*	m_near;
	Property*	m_far;
};
#endif

