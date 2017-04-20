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
#include "PipelineTreeView.h"

#include "PipelineTreeModel.h"
#include "QPipelineNode.h"
#include "RenderTargetView.h"

#include "RenderTargetView.h"


#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QMessageBox>

#include <Horde3D.h>
#include <Horde3DUtils.h>

PipelineTreeView::PipelineTreeView(QWidget* parent /*= 0*/) : QTreeView(parent), m_pipelineID(0)
{
	connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(showRenderTarget(const QModelIndex&)));
}


PipelineTreeView::~PipelineTreeView()
{
	closeTree();
}




void PipelineTreeView::currentNodeChanged(const QModelIndex& current, const QModelIndex& previous)
{
	if (current.isValid() && current.row() != previous.row())
	{
		QXmlTreeNode* node(static_cast<QXmlTreeNode*>(current.internalPointer()));
		emit currentNodeChanged(node);
	}
	else if (!current.isValid() && previous.isValid())
	{
		emit currentNodeChanged(0);
		clearSelection();
	}
}

void PipelineTreeView::showRenderTarget(const QModelIndex& index)
{
	if (index.isValid())
	{
		QXmlTreeNode* renderTarget = static_cast<QXmlTreeNode*>(index.internalPointer());
		if (renderTarget->property("Type").toInt() == QPipelineNode::PIPELINERENDERTARGETNODE)
		{
			RenderTargetView* view = new RenderTargetView(this, Qt::Window);
			view->setRenderTarget(renderTarget->xmlNode(), m_pipelineID);
			view->setVisible(true);
			connect(view, SIGNAL(closed(RenderTargetView*)), this, SLOT(removeView(RenderTargetView*)));
			m_renderTargetViews.push_back(view);
		}
	}
}

bool PipelineTreeView::loadPipeline(const QString& fileName, unsigned int pipelineID)
{
	closeTree();
	m_pipelineID = pipelineID;

	QFile file( QDir::current().absoluteFilePath(fileName));
	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(this, tr("Error"), tr("Error opening pipeline file %1!\n%2").arg(fileName).arg(file.errorString()));
		return false;
	}

	QString errorMsg;
	int errorLine, errorColumn;
	QDomDocument doc;
	if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
	{
		QMessageBox::warning(
			this, 
			tr("Error"), 
			tr("Error in line %1, column %2 when parsing pipeline file %3!\n%4").arg(errorLine).arg(errorColumn).arg(fileName).arg(errorMsg)
		);
		return false;
	}
	setModel(new PipelineTreeModel(pipelineID, doc.documentElement(), this));
	expandToDepth(1);
	resizeColumnToContents(0);	
	return true;
}

void PipelineTreeView::closeTree()
{
	qDeleteAll(m_renderTargetViews);
	m_renderTargetViews.clear();
	emit currentNodeChanged(0);
	setModel(0);	
	m_pipelineID = 0;
	// Deleting the root node will delete all sub nodes 	
}

void PipelineTreeView::removeView(RenderTargetView *view)
{
	int index = m_renderTargetViews.indexOf(view);
	if (index != -1)
		delete m_renderTargetViews.takeAt(index);
}
