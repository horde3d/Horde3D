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
#include "QOneTimeDialog.h"


const int QOneTimeDialog::ApplyAlways = 0x10000000;

QOneTimeDialog::QOneTimeDialog(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QDialog(parent, flags)
{
	setupUi(this);
	connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonPressed(QAbstractButton*)));
}


QOneTimeDialog::~QOneTimeDialog()
{
}

void QOneTimeDialog::buttonPressed(QAbstractButton* button)
{
	if (m_buttonBox->buttonRole(button) != QDialogButtonBox::AcceptRole &&
		m_buttonBox->buttonRole(button) != QDialogButtonBox::RejectRole)
		accept();

	m_button = m_buttonBox->standardButton(button);
}



int QOneTimeDialog::question(QWidget *parent, const QString &title, const QString &text, const QDialogButtonBox::StandardButton &button1, const QDialogButtonBox::StandardButton &button2, const QDialogButtonBox::StandardButton &button3 /*= QDialogButtonBox::NoButton*/)
{
        QOneTimeDialog dlg( parent );
	dlg.setWindowTitle(title);
	dlg.m_label->setText(text);
	dlg.m_buttonBox->addButton(button1);
	dlg.m_buttonBox->addButton(button2);
	dlg.m_buttonBox->addButton(button3);
	dlg.resize(dlg.vboxLayout->sizeHint());
	dlg.exec();

	int result = (int) dlg.m_button;

	return result | ( dlg.m_applyForAll->isChecked() ? ApplyAlways : 0 );
}
