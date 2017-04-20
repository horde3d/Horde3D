// *************************************************************************************************
//
// QPropertyEditor v 0.3
//   
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// Acknowledgements to Roman alias banal from qt-apps.org for the Enum enhancement
//
//
// The QPropertyEditor Library is free software; you can redistribute it and/or modify
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
// *************************************************************************************************
#ifndef QPROPERTYMODEL_H_
#define QPROPERTYMODEL_H_

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>

#include "QPropertyEditorWidget.h"

class Property;

/**
 * The QPropertyModel handles the user defined properties of QObjects
 */
class QPropertyModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	/**
	 * Constructor
	 * @param parent optional parent object
	 */
	QPropertyModel(QObject* parent = 0);
	/// Destructor
	virtual ~QPropertyModel();

	/// QAbstractItemModel implementation
	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;	

	/// QAbstractItemModel implementation
	QModelIndex parent ( const QModelIndex & index ) const;
	/// QAbstractItemModel implementation
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	/// QAbstractItemModel implementation
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	/// QAbstractItemModel implementation
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

	/// QAbstractItemModel implementation
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	/// QAbstractItemModel implementation
	Qt::ItemFlags flags ( const QModelIndex & index ) const;

	/// QAbstractItemModel implementation
	QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	
	/// QAbstractItemModel implementation
	QModelIndex buddy ( const QModelIndex & index ) const;

	/**
	 * Adds the user properties of the given class to the QPropertyModel instance
	 * 
	 * @param propertyObject the class inherited from QObject that contains user properties that should be 
	 *        managed by this instance
	 */
	void addItem(QObject* propertyObject);

	/**
	 * Creates a dataChanged signal for the given object
	 * @param propertyObject the instance of a QObject based class that should be updated 
	 * @param parent optional model index the propertyObject is child of
	 */
	void updateItem ( QObject* propertyObject, const QModelIndex& parent = QModelIndex() ) ;

	/**
	 * Removes all objects from the model
	 */
	void clear();

	/**
	 * Adds custom callback that will be used to create Property instances for custom datatypes
	 */
	void registerCustomPropertyCB(QPropertyEditorWidget::UserTypeCB callback);
		
	/**
	 * Adds custom callback that will be used to create Property instances for custom datatypes
	 */
	void unregisterCustomPropertyCB(QPropertyEditorWidget::UserTypeCB callback);
		

private:

	/// Adds dynamic properties to the model
	void addDynamicProperties( Property* parent, QObject* propertyObject );

	/// The Root Property for all objects
	Property*									m_rootItem;

	/// Custom callback
	QList<QPropertyEditorWidget::UserTypeCB>	m_userCallbacks;
	
};
#endif
