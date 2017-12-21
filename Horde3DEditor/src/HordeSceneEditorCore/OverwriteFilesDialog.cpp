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

#include "OverwriteFilesDialog.h"

#include <QtCore/QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QPushButton>

OverwriteFilesDialog::OverwriteFilesDialog(const QList<CopyJob>& filesToOverwrite, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QDialog(parent, flags)
{
	setupUi(this);
	QStringList notFound;
	for (int i=0; i<filesToOverwrite.size(); ++i)
	{
		if (filesToOverwrite[i].Source.exists() && filesToOverwrite[i].Target.exists())
		{
			QListWidgetItem* item = new QListWidgetItem(filesToOverwrite.at(i).Source.fileName(), m_fileList);
			item->setData(Qt::UserRole, QVariant::fromValue<CopyJob>(filesToOverwrite.at(i)));
		}
		else if (!filesToOverwrite[i].Source.exists())		
			notFound.push_back(filesToOverwrite[i].Source.absoluteFilePath());		
	}
	if (!notFound.isEmpty())	
		QMessageBox::warning(parent, tr("Attention"), tr("The following files couldn't be found:\n\n")+notFound.join("\n"));
	m_buttonBox->setFocus();
	m_renameButton = new QPushButton(tr("Rename"), m_buttonBox);
	m_renameButton->setDefault(false);
	m_overwriteButton = new QPushButton(tr("Overwrite"), m_buttonBox);
	m_overwriteButton->setDefault(false);
	m_buttonBox->addButton(m_renameButton, QDialogButtonBox::ActionRole);
	m_buttonBox->addButton(m_overwriteButton, QDialogButtonBox::ActionRole);
	m_buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
	m_buttonBox->button(QDialogButtonBox::Close)->setAutoDefault(true);
	connect(m_renameButton, SIGNAL(clicked()), this, SLOT(renameSelectedFiles()));
	connect(m_overwriteButton, SIGNAL(clicked()), this, SLOT(overwriteSelectedFiles()));
}


OverwriteFilesDialog::~OverwriteFilesDialog()
{
}

void OverwriteFilesDialog::renameSelectedFiles()
{	
	QList<QListWidgetItem*> items = m_fileList->selectedItems();
	if ( items.empty() )
	{
		QMessageBox::information(this, tr("No item selected"), tr("You have to select the files to rename first"));
		return;
	}	
	for (int i=0; i<items.count(); ++i)
	{
		CopyJob job = items.at(i)->data(Qt::UserRole).value<CopyJob>();
		bool ok = false;
		QString newFileName = QInputDialog::getText(this, tr("Select new filename"), tr("New name"), QLineEdit::Normal, job.Target.fileName(), &ok);
		if (!ok) return;
		if ( newFileName == job.Target.fileName() || QFile::exists(job.Target.absoluteDir().absoluteFilePath(newFileName)) )
		{
			if (QMessageBox::warning(
				this, 
				tr("Warning"), 
				tr("The file %1 already exists! Do you want to overwrite it?").arg(newFileName), 
				QMessageBox::Yes | QMessageBox::Default, 
				QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
				return;
			QFile::remove(job.Target.absoluteDir().absoluteFilePath(newFileName));
		}
		job.Target.setFile(job.Target.absoluteDir(), newFileName);
		QFile::copy(job.Source.absoluteFilePath(), job.Target.absoluteFilePath());
		delete m_fileList->takeItem(m_fileList->row(items.at(i)));
	}		
}

void OverwriteFilesDialog::overwriteSelectedFiles()
{
	QList<QListWidgetItem*> items = m_fileList->selectedItems();
	if ( items.empty() )
	{
		QMessageBox::information(this, tr("No item selected"), tr("You have to select the files to overwrite first"));
		return;
	}
	QProgressDialog dlg(tr("Overwriting files...           "), tr("Cancel"), 0, items.size(), this);
	dlg.setMinimumDuration(0);	
	for (int i=0; i<items.count(); ++i)
	{			
		CopyJob job = items.at(i)->data(Qt::UserRole).value<CopyJob>();
		dlg.setLabelText(tr("Overwriting %1").arg(job.Target.fileName()));
		dlg.setValue(i);
		if (dlg.wasCanceled())
			return;
		QFile::remove(job.Target.absoluteFilePath());
		QFile::copy(job.Source.absoluteFilePath(), job.Target.absoluteFilePath());
		delete m_fileList->takeItem(m_fileList->row(items.at(i)));		
	}	
	dlg.setValue(items.size());
}
