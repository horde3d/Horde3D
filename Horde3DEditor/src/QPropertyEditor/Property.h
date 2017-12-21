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

#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <QWidget>
#include <QStyleOption>
#include <QVariant>

/**
 * The Property class is the base class for all properties in the QPropertyEditor
 * You can implement custom properties inherited from this class to further enhence the
 * functionality of the QPropertyEditor
 */
class Property : public QObject
{
	Q_OBJECT

public:

	/**
	 * Constructor
	 *
	 * @param name the name of the property within the propertyObject (will be used in the QPropertyEditorWidget view too)
	 * @param propertyObject the object that contains the property
	 * @param parent optional parent object 
	 */
	Property(const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);
	
	/**
	 * The value stored by this property
	 * @return QVariant the data converted to a QVariant
	 */
	virtual QVariant value(int role = Qt::UserRole) const;
	/**
	 * Sets the value stored by this property
	 * @param value the data converted to a QVariant
	 */
	virtual void setValue(const QVariant& value);

	/**
	 * Returns the QObject which contains the property managed by this instance
	 * @return QObject* pointer to the QObject that contains user defined properties
	 */
	QObject* propertyObject() {return m_propertyObject;}

	/**
	 * Flag if property is used for indicating a group or really manages a property
	 * @return bool true if this property is only used to display a category in the QPropertyEditorWidget
	 */
	bool isRoot() {return m_propertyObject == 0;}
	
	/**
	 * Flag if the property can be set
	 * @return bool true if this property has no set method
	 */
	bool isReadOnly();

	/**
	 * Returns the row of this instance within the QPropertyModel
	 * @return int row within the QPropertyModel
	 */
	int row() {return parent()->children().indexOf(this);}
	
	/**
	 * returns optional settings for the editor widget that is used to manipulate the properties value
	 * @return QString a string that contains property settings for the editor widget (e.g. "minimum=1.0;maximum=10.0;")
	 */
	QString editorHints() {return m_hints;}
	
	/**
	 * Sets properties for the editor widget that is used to manipulate the data value managed by this instance
	 * @param hints a string containing property settings for the editor widget that manipulates this property
	 */
	virtual void setEditorHints(const QString& hints) {m_hints = hints;}
	
	/**
	 * Creates an editor for the data managed by this instance
	 * @param parent widget the newly created editor widget will be child of
	 * @param option currently not used
	 * @return QWidget* pointer to the editor widget
	 */
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option);
	
	/**
	 * Returns the data of the editor widget used to manipulate this instance
	 * @return QVariant the data converted to a QVariant
	 */
	virtual QVariant editorData(QWidget *editor);
	
	/**
	 * Changes the editor widget's data to a specific value
	 * @param editor the editor widget
	 * @param data the data to set in the editor widget
	 * @return bool true if editor widget was set to the given data successfully, false if the data can not be set in the editor (e.g. wrong datatype)
	 */
	virtual bool setEditorData(QWidget *editor, const QVariant& data);

	/**
	 * Tries to find the first property that manages the given propertyObject
	 * @param propertyObject
	 * @return Property
	 */
	Property*	findPropertyObject(QObject* propertyObject);

private slots:
	/**
	 * This slot is used to immediately set the properties when the editor widget's value of a double or float 
	 * property has changed
	 * @param value the new value
	 */
	void setValue(double value);
	/**
	 * This slot is used to immediately set the properties when the editor widget's value of an integer
	 * property has changed
	 * @param value the new value
	 */
	void setValue(int value);

protected:
	QObject*	m_propertyObject;
	QString		m_hints;	

};

#endif
