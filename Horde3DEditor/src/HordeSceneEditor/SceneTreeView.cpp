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

#include "SceneTreeView.h"
#include "SceneTreeModel.h"
#include "QReferenceNode.h"
#include "SceneWizard.h"
#include "HordeSceneEditor.h"
#include "PlugInManager.h"
#include "AttachmentPlugIn.h"
#include "GLWidget.h"
#include "CustomTypes.h"

#include <QtCore/QFile>
#include <QAction>
#include <QApplication>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtGui/QClipboard>
#include <QtCore/QMimeData>
#include <QMessageBox>
#include <QtCore/QEvent>
#include <QAction>
#include <QMenu>
#include <QUndoStack>

#include <QXmlTree/QXmlTreeUndoCommands.h>

#include <Horde3D.h>
#include <Horde3DUtils.h>


class RemoveNodeUndoCommand : public QRemoveXmlNodeUndoCommand
{
public:
	RemoveNodeUndoCommand(QXmlTreeNode* node, const QString& text) : QRemoveXmlNodeUndoCommand(node, text)
	{
	}

	void undo()
	{
		QRemoveXmlNodeUndoCommand::undo();
		h3dutLoadResourcesFromDisk(".");
	}	

};

class AddNodeUndoCommand : public QAddXmlNodeUndoCommand
{
public:
	AddNodeUndoCommand(const QDomElement& node, const QDomElement& parent, QXmlTreeModel* model, const QString& text) : QAddXmlNodeUndoCommand(node, parent, model, text)
	{
	}

	void redo()
	{
		qApp->setOverrideCursor(Qt::BusyCursor);
		QModelIndex parentIndex = m_model->index(m_parentNode);
		QModelIndex index = m_model->addNode(m_model->rowCount(parentIndex), m_addNode, parentIndex);

		if ( !m_addNode.hasAttribute("tx") && !m_addNode.hasAttribute("ty") && !m_addNode.hasAttribute("tz"))
		{
			QSceneNode* newNode = static_cast<QSceneNode*>(index.internalPointer());
			
			const float* camera = 0;	
			unsigned int cameraID = HordeSceneEditor::instance()->glContext()->activeCam();
			h3dGetNodeTransMats(cameraID, 0, &camera);
			if ( camera )
			{
				newNode->blockSignals(true);
				const float* node = 0;
				H3DNode parentNode = h3dGetNodeParent(newNode->hordeId());
				h3dGetNodeTransMats(parentNode, 0, &node);
				if ( node ) 
				{
					QVec3f t, r, s;
					(QMatrix4f(node).inverted() * QMatrix4f(camera)).decompose(t, r, s);															

					// Get bounding box
					float minX, minY, minZ, maxX, maxY, maxZ;
					h3dGetNodeAABB( newNode->hordeId(), &minX, &minY, &minZ, &maxX, &maxY, &maxZ );
					
					if( fabs(maxY - minY) > 0.0001f ) 
					{
						float frustumHeight = h3dGetNodeParamF(cameraID, H3DCamera::TopPlaneF, 0) - h3dGetNodeParamF(cameraID, H3DCamera::BottomPlaneF, 0);
						float x = h3dGetNodeParamF(cameraID, H3DCamera::NearPlaneF, 0);
						float scale = (frustumHeight * 10 / x) / fabs(maxY - minY) ;					
						newNode->setScale( QVec3f( scale, scale, scale ) );
					}
					s = QMatrix4f(node).getScale();
					newNode->setPosition( t - QMatrix4f::RotMat( r ) * QVec3f(0, 0, 10 / s.Z) );					
				}
				newNode->blockSignals(false);								
			}
		}
		h3dutLoadResourcesFromDisk(".");
		qApp->restoreOverrideCursor();
	}

};


SceneTreeView::SceneTreeView(QWidget* parent /*= 0*/) : QXmlTreeView(parent), m_rootNode(0), m_contextCall(false)
{
	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(nodeActivated(const QModelIndex&)));
	connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(resizeColumns(const QModelIndex&)));
}


SceneTreeView::~SceneTreeView()
{
	closeTree();
}

bool SceneTreeView::loadSceneGraph(const QString& fileName)
{
	closeTree();
	QDomDocument root;
	root.setContent( QString( "<Reference/>" ) );
	root.documentElement().setAttribute("name", fileName);	
	root.documentElement().setAttribute("sceneGraph", QDir(h3dutGetResourcePath(H3DResTypes::SceneGraph)).relativeFilePath(fileName));
	m_rootNode = new QReferenceNode(root.documentElement(), 0, 0, 0);
	if (!m_rootNode->model())
	{		
		delete m_rootNode;
		m_rootNode = 0;
		return false;
	}
	else
		pushSceneGraph(m_rootNode->model());	
	return true;
}

void SceneTreeView::save()
{
	m_rootNode->save();
}

void SceneTreeView::closeTree()
{
	if( model() )
	{
		emit QXmlTreeView::currentNodeChanged(0);
		setModel(0);	
	}
	// Deleting the root node will delete all sub nodes and undo stacks of sub tree models
	// The QUndoGroup of the HordeSceneEditor will be cleared automatically by the Destructor of the QUndoStack
	delete m_rootNode;
	m_activeModels.clear();
	m_rootNode = 0;
}



bool SceneTreeView::popSceneGraph()
{	
	if (m_activeModels.size() > 1)
	{
		SceneTreeModel* oldModel = m_activeModels.takeLast();
		pushSceneGraph( m_activeModels.takeLast() );
		selectNode( oldModel->parentNode()->property( "ID" ).toInt() );
		return true;
	}	
	return false;
}

QUndoStack* SceneTreeView::currentUndoStack()
{		
	if (!m_activeModels.isEmpty())
		return m_activeModels.last()->undoStack();	
	else
		return 0;
}

void SceneTreeView::nodeActivated(const QModelIndex& index /*=QModelIndex()*/)
{
	QModelIndex node = currentIndex();
	if (index.isValid()) node = index;
	// Renaming of name column should not activate the node
	if (index.column() == 0)
		return;
	
	QSceneNode* sceneNode = static_cast<QSceneNode*>(node.internalPointer());
	if( sceneNode )
	{
		// Enter reference node if Type Column of a Reference Node was activated 
		if (sceneNode->objectName() == "Reference" && (qApp->keyboardModifiers() & Qt::ControlModifier) == 0)
		{
			SceneTreeModel* model = static_cast<QReferenceNode*>(sceneNode)->model();
			pushSceneGraph(model);		
		}
		// Activate Node if ID column was selected
		else 
			sceneNode->activate();
	}
}


bool SceneTreeView::selectNode( int H3DNode )
{
	if (H3DNode == 0)
	{
		while (popSceneGraph())	{}		
		setCurrentIndex(QModelIndex());
		return false;
	}
	// First check the trees already loaded 
	for (int i = m_activeModels.count(); i>0; --i)
	{
		QModelIndexList items(m_activeModels[i-1]->match(m_activeModels[i-1]->index(0,2), Qt::DisplayRole, H3DNode, 1, Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
		if (!items.isEmpty())
		{
			while (m_activeModels.count() > i && popSceneGraph()) {}
			// Little hack, otherwise the scrollTo command won't be executed successfully
			qApp->processEvents();
			setCurrentIndex(items.first());
			scrollTo(items.first(), QAbstractItemView::PositionAtCenter);
			return true;
		}
	}
	// Not found? Then we have to take the hard way	and search through all models
	QApplication::setOverrideCursor(Qt::BusyCursor);
	QStringList fileCache;
	QList<SceneTreeModel*> models = findSceneGraph(m_activeModels.first(), H3DNode);
	QApplication::restoreOverrideCursor();
	if (models.isEmpty())
		return false;
	else
	{
		m_activeModels.clear();
		while (!models.isEmpty())
			pushSceneGraph(models.takeFirst());
		selectNode(H3DNode);
		return true;
	}
}

void SceneTreeView::addNode()
{
	QAction* source = qobject_cast<QAction*>(sender());
	if( !source ) return;

	SceneTreeModel* treeModel = qobject_cast<SceneTreeModel*>(model());
	QSceneNode* sceneNode = static_cast<QSceneNode*>(treeModel->rootNode());
	// If the slot was called from the context menu...
	if( m_contextCall && currentIndex().isValid() )
		// ... select the currently selected node as root
		sceneNode = static_cast<QSceneNode*>(currentIndex().internalPointer());	
	QDomElement newNode = HordeSceneEditor::instance()->pluginManager()->createNode( source->data().toString(), this );
	if( !newNode.isNull() )
		treeModel->undoStack()->push(createAddUndoCommand(newNode, sceneNode->xmlNode(), treeModel, source->text()));		
}


void SceneTreeView::addAttachmentNode()
{
	QSceneNode* node = static_cast<QSceneNode*>(currentIndex().internalPointer());
	if( node->xmlNode().firstChildElement("Attachment").isNull() )
	{
		HordeSceneEditor::instance()->pluginManager()->attachmentPlugIn()->createNodeAttachment();
		qobject_cast<QAction*>(sender())->setText(tr("Remove Attachment"));
	}
	else
	{
		HordeSceneEditor::instance()->pluginManager()->attachmentPlugIn()->removeNodeAttachment();
		qobject_cast<QAction*>(sender())->setText(tr("Add Attachment"));
	}
}


void SceneTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	setCurrentIndex(indexAt(event->pos()));
	QModelIndex index(currentIndex());
	if (!index.isValid())
		clearSelection();
	QList<QAction*> actions = this->actions();
	for (int i = 0; i<actions.size();)
		if (!actions[i]->isEnabled())
			actions.takeAt(i);
		else 
			++i;
	if( !actions.isEmpty() )
	{
		m_contextCall = true;
		QMenu::exec(actions, mapToGlobal(event->pos()));
		m_contextCall = false;
	}
	event->accept();
}

void SceneTreeView::dropEvent(QDropEvent* event)
{
	QTreeView::dropEvent(event);
	if (event->isAccepted())
	{
		setCursor(Qt::BusyCursor);
		QModelIndexList sources = selectionModel()->selectedRows();
		// Get target node, that the selected nodes should use as parent
		QSceneNode* target = static_cast<QSceneNode*>(indexAt(event->pos()).internalPointer()); 
		if ( !target ) 
			target = static_cast<QSceneNode*>(m_activeModels.last()->rootNode());
		if ( !sources.empty() )
		{
			Qt::DropAction action = event->proposedAction();
			if (action & Qt::CopyAction || action & Qt::MoveAction)
			{
				foreach( QModelIndex index, sources )
				{
					QSceneNode* node = static_cast<QSceneNode*>(index.internalPointer());
					currentUndoStack()->push(createAddUndoCommand( node->xmlNode().cloneNode().toElement(), target->xmlNode(), m_activeModels.last(), tr("Add Node '%1'").arg(node->name())));
					if (action & Qt::MoveAction)
					{
						currentUndoStack()->push(createRemoveUndoCommand(node, tr("Remove Node '%1'").arg(node->name())));
					}
				}				
			}
			if (action == Qt::MoveAction)
				clearSelection(); // Avoid call to removeRows by QAbstractItemView			
		}
		unsetCursor();
	}
}

QAddXmlNodeUndoCommand* SceneTreeView::createAddUndoCommand(const QDomElement& node, const QDomElement& parent, QXmlTreeModel* model, const QString& text) const
{
	return new AddNodeUndoCommand(node, parent, model, text);
}

QRemoveXmlNodeUndoCommand* SceneTreeView::createRemoveUndoCommand(QXmlTreeNode* node, const QString& text) const
{
	return new RemoveNodeUndoCommand(node, text);
}

void SceneTreeView::currentNodeChanged(const QModelIndex& current, const QModelIndex& previous)
{
	if (current.isValid() && current.internalPointer())
	{
		QSceneNode* node = static_cast<QSceneNode*>(current.internalPointer());
		if (h3dGetNodeType(node->hordeId()) == H3DNodeTypes::Undefined)
			HordeSceneEditor::instance()->updateLog(new QListWidgetItem(tr("The selected node has no valid scenegraph ID.\nMaybe it was removed from the scene graph by a plugin or failed to load properly!"), 0, 2));
	}
	if ( current.internalPointer() != previous.internalPointer() )
		emit nodeAboutToChange();
	QXmlTreeView::currentNodeChanged(current, previous);
}

void SceneTreeView::useCameraTransformation()
{
	if (currentIndex().isValid())
	{
		QSceneNode* object = static_cast<QSceneNode*>(currentIndex().internalPointer());
		const float* camera = 0;	
		unsigned int cameraID = HordeSceneEditor::instance()->glContext()->activeCam();
		// TODO create a useCameraTransformation( cameraID ) method in QSceneNode to handle setting of the transformation indivdually
		h3dGetNodeTransMats(cameraID, 0, &camera);
		if ( camera )
		{			
			const float* node = 0;
			H3DNode parentNode = h3dGetNodeParent(object->hordeId());
			h3dGetNodeTransMats(parentNode, 0, &node);
			if ( node ) 
			{
				QVec3f t, r, s;
				(QMatrix4f(node).inverted() * QMatrix4f(camera)).decompose(t, r, s);
				// Only apply position and rotation, because normally we don't want to rescale an object 
				// but only adjust its position and rotation
				object->setPosition(t);
				object->setRotation(r * (180.0f / 3.1415926f));								
			}
		}
	}
}

void SceneTreeView::setCameraTransformation()
{
	if (currentIndex().isValid())
	{
		QSceneNode* object = static_cast<QSceneNode*>(currentIndex().internalPointer());

		unsigned int cameraID = HordeSceneEditor::instance()->glContext()->activeCam();
		const float* cameraParent = 0;
		H3DNode parentNode = h3dGetNodeParent(cameraID);
		h3dGetNodeTransMats(parentNode, 0, &cameraParent); 
		if ( !cameraParent ) return;

		if( cameraID == object->hordeId() )
		{			
			QVec3f p = object->position();
			QVec3f r = object->rotation();
			QVec3f s = object->scale();
			h3dSetNodeTransform( cameraID, p.X, p.Y, p.Z, r.X, r.Y, r.Z, s.X, s.Y, s.Z );	
		}
		else
		{
			const float* node;
			h3dGetNodeTransMats(object->hordeId(), 0, &node);
			if( node ) 
				h3dSetNodeTransMat(cameraID, (QMatrix4f(cameraParent).inverted() * QMatrix4f(node)).x);	
		}
	}
}

void SceneTreeView::nodeDataChanged(const QModelIndex & index)
{
	if (index.isValid() && index.row() == currentIndex().row())
	{
		QSceneNode* object = static_cast<QSceneNode*>(currentIndex().internalPointer());
		emit nodePropertyChanged(object);
	}
}

void SceneTreeView::resizeColumns( const QModelIndex& index )
{
	for (int i=0; i < model()->columnCount(); ++i)
		resizeColumnToContents(i);
}

void SceneTreeView::pushSceneGraph(SceneTreeModel* model)
{		
	if( model )
	{
		connect(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(nodeDataChanged(const QModelIndex&)));
		setModel(model);
		m_activeModels.push_back(model);	
		// This will activate the correct undostack
		emit modelChanged(model);	
		//emit QXmlTreeView::currentNodeChanged(0);
		if (m_activeModels.size() > 1)
			emit rootLayer(m_activeModels[m_activeModels.size() - 2]->parentNode()->property("Scene_Graph_File").toString());
		else
			emit rootLayer(QString());
		for (int i=0; i<model->columnCount(); ++i)
			resizeColumnToContents(i);
	}
}

QList<SceneTreeModel*> SceneTreeView::findSceneGraph(SceneTreeModel* model, unsigned int H3DNode)
{	
	QList<SceneTreeModel*> retVal;
	QModelIndexList list = model->match(model->index(0,2), Qt::DisplayRole, H3DNode, 1, Qt::MatchExactly | Qt::MatchRecursive);
	if (!list.isEmpty())
	{
		retVal.push_back(model);
		return retVal;
	}
	list = model->match(model->index(0,1), Qt::DisplayRole, QString("Reference"), -1, Qt::MatchExactly | Qt::MatchRecursive);
	for (int i=0; i<list.size(); ++i)
	{
		QReferenceNode* refNode = static_cast<QReferenceNode*>(list.at(i).internalPointer());
		retVal = findSceneGraph(refNode->model(), H3DNode);
		if (!retVal.isEmpty())
		{
			retVal.push_front(model);
			return retVal;
		}
	}
	return retVal;
}

