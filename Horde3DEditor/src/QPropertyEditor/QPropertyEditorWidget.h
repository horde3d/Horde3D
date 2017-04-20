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

#ifndef QPROPERTYEDITORWIDGET_H_
#define QPROPERTYEDITORWIDGET_H_

#include <QTreeView>

class QPropertyModel;
class Property;

/**
 * \mainpage QPropertyEditor
 *
 * \section intro_sec Introduction
 *
 * The main purpose for the QPropertyEditor is the visualization and manipulation of properties defined via the Q_PROPERTY macro in
 * QObject based classes.
 */
 
/**
 * \brief The QPropertyEditorWidget offers an easy to use mechanism to visualize properties of a class inherited from QObject. 
 *
 * Qt provides a nice way to define class properties by using the Q_PROPERTY macro. The purpose of the QPropertyEditor
 * is to visualize these properties in an easy way. 
 *
 * To use the property editor, all you have to do is to create a class that defines it's properties by using Q_PROPERTY
 * and to add this class by using the addObject() method of this QPropertyEditorWidget class.
 * The QPropertyEditorWidget is inherited from QTreeView and will display the properties in a tree with two columns: Name and Value
 *
 * For basic data types the build in editor widgets of Qt will be used. The QPropertyEditor itself only defines an additional
 * editor for QColor (based on the Color Editor Factory Example from Trolltech). But it can easily be extended by yourself
 * either within the library or for special datatypes also outside of the library in your application.
 */
class QPropertyEditorWidget : public QTreeView
{
	Q_OBJECT
public:

	/**
	 * A typedef for a callback used to create user defined properties for custom datatypes
	 */
	typedef Property* (*UserTypeCB)(const QString& name, QObject* propertyObject, Property* parent);

	/**
	 * \brief Constructor 
	 *
	 * Creates a new editor widget based on QTreeView
	 * @param parent optional parent widget
	 */
	QPropertyEditorWidget(QWidget* parent = 0);
	
	/// Destructor
	virtual ~QPropertyEditorWidget();

	/**
	 * Adds the user properties of the given class to the QPropertyModel associated with this view
	 * 
	 * @param propertyObject the class inherited from QObject that contains user properties that should be 
	 *        managed by the QPropertyModel associated with this view
	 */
	void addObject(QObject* propertyObject);

	/**
	 * Similar to the addObject() method this method adds the properties of the given class to the QPropertyModel
	 * associated with this view. But in contrast to addObject() it will clear the model before, removing all
	 * previously added objects.
	 * 
	 * @param propertyObject  the class inherited from QObject that contains user properties that should be 
	 *        managed by the QPropertyModel associated with this view
	 */
	void setObject(QObject* propertyObject);

	/**
	 * Updates the view for the given object. This can be usefull if a property was changed programmatically instead
	 * of using the view. In this case the view normally will display the new property values only after the user clicked
	 * on it. To overcome this problem you can call updateObject with the object whose property was changed.
	 */
	void updateObject(QObject* propertyObject);

	/**
	 * If you define custom datatypes outside of this library the QPropertyModel will check if you
	 * also defined a callback that is responsible to create custom property classes inherited from Property to handle 
	 * these datatypes. With this method you can register such a callback that will create custom properties for custom datatypes.
	 */
	void registerCustomPropertyCB(UserTypeCB callback);

	/**
	 * You can register more than one callback. If one of those callbacks are not used any longer, you can unregister
	 * it with this method
	 */
	void unregisterCustomPropertyCB(UserTypeCB callback);

private:
	/// The Model for this view
	QPropertyModel*			m_model;

};
#endif
