// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the GameEngine developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// The GameEngine is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The GameEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************


// ****************************************************************************************
//
// GameEngine Horde3D Editor Plugin of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// ****************************************************************************************
#ifndef EXTRATREEMODEL_H_
#define EXTRATREEMODEL_H_

#include <QXmlTree/QXmlTreeModel.h>

class PlugInManager;

/**
 * The scene tree model manages the extra nodes for the attachment plugin
 */
class ExtraTreeModel : public QXmlTreeModel
{

	Q_OBJECT
public:
	/**
	 * \brief Constructor
	 */
	ExtraTreeModel(PlugInManager* factory, const QDomElement& extrasRoot, QObject* parent = 0);
	virtual ~ExtraTreeModel();

	virtual QVariant data(const QModelIndex &index, int role) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	PlugInManager* nodeFactory() const {return m_extraNodeFactory;}

private:

	PlugInManager*	m_extraNodeFactory;

};
#endif
