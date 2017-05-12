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


#ifndef COLORCOMBO_H_
#define COLORCOMBO_H_

#include <QComboBox>

class ColorCombo : public QComboBox
{
	Q_OBJECT
public:
	ColorCombo(QWidget* parent = 0);
	virtual ~ColorCombo();

    QColor color() const;
    void setColor(QColor c);

private slots:
	void currentChanged(int index);	

private:
	QColor	m_init;

};
#endif
