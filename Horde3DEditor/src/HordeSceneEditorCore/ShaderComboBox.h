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

#ifndef SHADERCOMBOBOX_H_
#define SHADERCOMBOBOX_H_

#include <QComboBox>

#include "CustomTypes.h"

class ShaderComboBox : public QComboBox
{
	Q_OBJECT

	Q_PROPERTY(QString shaderFile READ shaderFile);

public:
	ShaderComboBox(QWidget* parent = 0);
	virtual ~ShaderComboBox();

	void init(const QString& resourcePath);

	QString shaderFile() const {return currentText();}

	Shader shader() const;
	void setShader(Shader shader);

signals:
	void shaderChanged();
	void editFinished();

private slots:
	void currentChanged(int index);	

private:
	void addShader(const QString dir);

	QString	m_init;

	QString m_resourcePath;	
};
#endif

