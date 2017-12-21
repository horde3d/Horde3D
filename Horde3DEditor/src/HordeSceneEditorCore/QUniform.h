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
#ifndef QUNIFORM_H_
#define QUNIFORM_H_

#include <QtCore/QObject>
#include <QtXml/qdom.h>

#include "CustomTypes.h"

class QUniform : public QObject
{
	Q_OBJECT

	Q_CLASSINFO("QUniform", "Uniform");
	
	Q_PROPERTY(double a READ a WRITE setA DESIGNABLE true USER true)
	Q_CLASSINFO("a", "decimals=6;singleStep=0.01;");
	Q_PROPERTY(double b READ b WRITE setB DESIGNABLE true USER true)
	Q_CLASSINFO("b", "decimals=6;singleStep=0.01;");
	Q_PROPERTY(double c READ c WRITE setC DESIGNABLE true USER true)
	Q_CLASSINFO("c", "decimals=6;singleStep=0.01;");
	Q_PROPERTY(double d READ d WRITE setD DESIGNABLE true USER true)
	Q_CLASSINFO("d", "decimals=6;singleStep=0.01;");

public:
	QUniform( const QDomElement& uniformNodeNode, bool shaderUniform, QObject* parent = 0);
	virtual ~QUniform();

	QString name() const;
	void setName(const QString& name);

	double a() const;
	void setA(const double& a);
	
	double b() const;
	void setB(const double& b);
	
	double c() const;
	void setC(const double& c);
	
	double d() const;
	void setD(const double& d);
	
	QDomElement& xmlNode() {return m_uniformNode;}

	bool isShaderUniform() const { return m_shaderUniform; }
private:
	QDomElement		m_uniformNode;
        // Indicates if this uniform belongs to a shader ( or a material otherwise )
        bool			m_shaderUniform;

};
#endif
