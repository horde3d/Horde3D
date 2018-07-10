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

#ifndef COLORSELECTIONBUTTON_H_
#define COLORSELECTIONBUTTON_H_

#include <QItemDelegate>

class QSignalMapper;

/**
 * This class is used to create the editor widgets for datatypes encapsulated in QVariant variables
 */
class QVariantDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	/**
	 * Constructor 
	 * @param parent optional parent object
	 */
	QVariantDelegate(QObject* parent = 0);
	/// Destructor
	virtual ~QVariantDelegate();

	/**
	 * Creates an editor widget as child of a given widget for a specific QModelIndex
	 * 
	 * @param parent the parent widget for the editor
	 * @param option some style options that the editor should use
	 * @param index the index of the item the editor will be created for
	 * @return QWidget the editor widget
	 */
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	/**
	 * Tries to set the editor data based on the value stored at a specific QModelIndex
	 * @param editor the editor widget
	 * @param index the model index of the value that should be used in the editor
	 */
	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;

	/**
	 * Sets the data of a specific QModelIndex to tha value of the editor widget
	 * @param editor the editor widget that contains the new value
	 * @param model the model that contains the index
	 * @param index the index within the model whose data value should be set to the data value of the editor
	 */
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

	/// QItemDelegate implementation
	virtual void updateEditorGeometry(QWidget *editor,  const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	void parseEditorHints(QWidget* editor, const QString& editorHints) const;

	QSignalMapper*	m_finishedMapper;
};
#endif
