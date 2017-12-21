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
#include "QUniform.h"


QUniform::QUniform(const QDomElement& uniformNode, bool shaderUniform, QObject* parent /*= 0*/) : QObject(parent), m_uniformNode(uniformNode),
m_shaderUniform( shaderUniform )
{
	
}


QUniform::~QUniform()
{
}

QString QUniform::name() const
{
	if( m_uniformNode.hasAttribute( "name" ) )
		return m_uniformNode.attribute("name");
	else if( m_uniformNode.hasAttribute( "id" ) )
		return m_uniformNode.attribute( "id" );
	else
		return tr("No Uniform Identifier");
}

void QUniform::setName(const QString& name)
{
	if( m_uniformNode.hasAttribute( "name" ) )
		m_uniformNode.setAttribute("name", name);
	else if( m_uniformNode.hasAttribute( "id" ) )
		m_uniformNode.setAttribute( "id", name );
}

double QUniform::a() const
{	
	return roundIt( m_uniformNode.attribute("a", "0.0").toDouble(), 6 );
}


void QUniform::setA( const double& a) 
{
	m_uniformNode.setAttribute("a", QString::number( a, 'f' ) );
}

double QUniform::b() const
{
	return roundIt( m_uniformNode.attribute("b", "0.0").toDouble(), 6 );
}


void QUniform::setB( const double& b ) 
{
	m_uniformNode.setAttribute("b", QString::number( b, 'f' ));
}

double QUniform::c() const
{
	return roundIt( m_uniformNode.attribute("c", "0.0").toDouble(), 6 );
}


void QUniform::setC( const double& c ) 
{
	m_uniformNode.setAttribute("c", QString::number( c, 'f' ));
}

double QUniform::d() const
{
	return roundIt( m_uniformNode.attribute("d", "0.0").toDouble(), 6 );
}


void QUniform::setD( const double& d ) 
{
	m_uniformNode.setAttribute("d", QString::number( d, 'f' ));
}




