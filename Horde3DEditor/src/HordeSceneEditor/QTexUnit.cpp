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
#include "QTexUnit.h"

#include <Horde3D.h>

QTexUnit::QTexUnit( const QDomElement& texUnitNode, QObject* parent /*= 0*/) : QObject(parent), m_texUnitNode(texUnitNode)
{
	
}


QTexUnit::~QTexUnit()
{
}


Texture QTexUnit::map() const
{
	return Texture(m_texUnitNode.attribute("map") );

}

void QTexUnit::setMap(const Texture& map)
{
	m_texUnitNode.setAttribute("map", map.FileName);
}

bool QTexUnit::compressed() const
{
	return m_texUnitNode.attribute("allowCompression", "true").compare("true", Qt::CaseInsensitive)==0 ||
		   m_texUnitNode.attribute("allowCompression").compare("1")==0;
}

void QTexUnit::setCompressed(bool compressionAllowed)
{
	m_texUnitNode.setAttribute("allowCompression", compressionAllowed);
}

bool QTexUnit::mipmaps() const
{
	return
		m_texUnitNode.attribute("mipmaps", "true").compare("true", Qt::CaseInsensitive) == 0 ||
		m_texUnitNode.attribute("mipmaps").compare("1") == 0 ;
}

void QTexUnit::setMipmaps( bool mipmaps )
{
	m_texUnitNode.setAttribute("mipmaps", mipmaps ? "true" : "false" );
}



