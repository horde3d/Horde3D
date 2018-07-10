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
#ifndef QTEXUNIT_H_
#define QTEXUNIT_H_

#include <QtCore/QObject>
#include <QtXml/qdom.h>

#include "CustomTypes.h"

class QTexUnit : public QObject
{
	Q_OBJECT

	Q_CLASSINFO("QTexUnit", "Sampler");

	Q_PROPERTY(Texture Map READ map WRITE setMap DESIGNABLE true USER true)
	Q_PROPERTY(bool Mipmaps READ mipmaps WRITE setMipmaps DESIGNABLE true USER true)
	Q_PROPERTY(bool Compressed READ compressed WRITE setCompressed DESIGNABLE true USER true)

public:
	QTexUnit( const QDomElement& texUnitNode, QObject* parent = 0 );
	virtual ~QTexUnit();
	
	Texture map() const;
	void setMap(const Texture& map);

	bool mipmaps() const;
	void setMipmaps(bool cube);

	bool compressed() const;
	void setCompressed(bool compressionAllowed);

	QDomElement& xmlNode() {return m_texUnitNode;}
private:

	QDomElement		m_texUnitNode;
	int				m_unit;	
};
#endif
