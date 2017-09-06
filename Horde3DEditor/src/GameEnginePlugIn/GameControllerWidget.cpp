// ****************************************************************************************
//
// GameEngine PlugIn
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the GameEngine of the University of Augsburg
// 
// You are not allowed to redistribute the code, if not explicitly authorized by the author
//
// ****************************************************************************************
#include "GameControllerWidget.h"

#include "AttachmentTreeModel.h"
#include "QAttachmentNode.h"

#include <QXmlTree/QXmlTreeNode.h>
#include <GameEngine/GameEngine.h>

#include <QSceneNode.h>

#include <QtWidgets/qmenu.h>
#include <QtXml/qdom.h>

#include <Horde3D/Horde3D.h>

GameControllerWidget::GameControllerWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags), m_currentNode(0), m_currentSceneNode(0)
{	
	setupUi(this);
	connect(m_entityName, SIGNAL(textChanged(const QString&)), this, SLOT(entityNameChanged(const QString&)));
	connect(m_addComponent, SIGNAL(clicked()), this, SLOT(addComponent()));
	connect(m_removeComponent, SIGNAL(clicked()), this, SLOT(removeComponent()));
	connect(m_attachmentTreeView, SIGNAL(currentNodeChanged(QXmlTreeNode*)), this, SLOT(componentSelected(QXmlTreeNode*)));

	connect(m_physicsWidget, SIGNAL(modified(bool)), this, SLOT(emitModified(bool)));
	connect(m_crowdVisibilityWidget, SIGNAL(modified(bool)), this, SLOT(emitModified(bool)));
	connect(m_crowdParticleWidget, SIGNAL(modified(bool)), this, SLOT(emitModified(bool)));
	connect(m_keyframeAnimWidget, SIGNAL(modified(bool)), this, SLOT(emitModified(bool)));
	connect(m_soundWidget, SIGNAL(modified(bool)), this, SLOT(emitModified(bool)));
	connect(m_ttsWidget, SIGNAL(modified(bool)), this, SLOT(emitModified(bool)));
	connect(m_soundWidget, SIGNAL(activatePhonemeEditor(bool, unsigned int, const QString&)), this, SLOT(switchPhonemeEditor(bool, unsigned int, const QString&)));
	connect(m_phonemeEditorWidget, SIGNAL(closeEditor()), this, SLOT(switchPhonemeEditor()));

	connect(m_actionAddPhysicsComponent, SIGNAL(triggered()), m_attachmentTreeView, SLOT(addPhysicsComponent()));
	connect(m_actionAddTTSComponent, SIGNAL(triggered()), m_attachmentTreeView, SLOT(addTTSComponent()));
	connect(m_actionAddKeyframeAnimComponent, SIGNAL(triggered()), m_attachmentTreeView, SLOT(addKeyframeAnimComponent()));
	connect(m_actionAddCrowdParticleComponent, SIGNAL(triggered()), m_attachmentTreeView, SLOT(addCrowdParticleComponent()));
	connect(m_actionAddCrowdVisNodeComponent, SIGNAL(triggered()), m_attachmentTreeView, SLOT(addCrowdVisNodeComponent()));
	connect(m_actionAddSound3DComponent, SIGNAL(triggered()), m_attachmentTreeView, SLOT(addSound3DComponent()));

	connect(this, SIGNAL(phonemeFileChanged()), m_phonemeEditorWidget, SLOT(closePhonemeEditor())); 
	connect(this, SIGNAL(updateFinished()), m_physicsWidget, SLOT(updateFinished()));
}


GameControllerWidget::~GameControllerWidget()
{
}

void GameControllerWidget::emitModified(bool changed)
{
	emit modified(changed);
}

void GameControllerWidget::emitUpdateFinished()
{
	emit updateFinished();
}

void GameControllerWidget::init()
{
	size_t size = GameEngine::componentNames(0, 0);
	char* components = new char[size];
	GameEngine::componentNames(components, size);
	QStringList availableNodes = QString(components).split(';', QString::SkipEmptyParts);
	delete[] components;	

	while( !m_attachmentTreeView->actions().isEmpty() )
		m_attachmentTreeView->removeAction( m_attachmentTreeView->actions().first() );

	if( availableNodes.contains("BulletPhysics") ) m_attachmentTreeView->addAction(m_actionAddPhysicsComponent);
	if( availableNodes.contains("TTS") ) m_attachmentTreeView->addAction(m_actionAddTTSComponent);
	if( availableNodes.contains("KeyframeAnimation") ) m_attachmentTreeView->addAction(m_actionAddKeyframeAnimComponent);
	if( availableNodes.contains("CrowdParticle") ) m_attachmentTreeView->addAction(m_actionAddCrowdParticleComponent);
	if( availableNodes.contains("CrowdVisNode") ) m_attachmentTreeView->addAction(m_actionAddCrowdVisNodeComponent);
	if( availableNodes.contains("Sound3D") ) m_attachmentTreeView->addAction(m_actionAddSound3DComponent);

}

void GameControllerWidget::release()
{
	m_attachmentTreeView->actions().clear();
}

void GameControllerWidget::setCurrentNode(QXmlTreeNode *node)
{
	emit phonemeFileChanged();
	if (node)
	{
		AttachmentTreeModel* model = static_cast<AttachmentTreeModel*>(node->property("__AttachmentModel").value<void*>());
		m_componentWidgets->setCurrentWidget(m_emptyWidget);			
		m_attachmentTreeView->setModel(model);		
		m_attachmentTreeView->selectionModel()->setCurrentIndex(model->index(0,0), QItemSelectionModel::SelectCurrent);
		connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(entityChanged(const QModelIndex&, const QModelIndex&)));
		m_currentNode = model->rootNode();
		m_currentSceneNode = node;
		m_entityName->blockSignals(true);
		m_entityName->setText(m_currentNode->property("Name").toString());
		m_entityName->blockSignals(false);
		updateActions();		
		setEnabled(true);
	}
	else
	{
		m_currentSceneNode = 0;
		if (m_attachmentTreeView->model())
			disconnect(m_attachmentTreeView->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(entityChanged(const QModelIndex&, const QModelIndex&)));
		m_attachmentTreeView->setModel(0);
		m_entityName->blockSignals(true);
		m_entityName->setText("");
		m_entityName->blockSignals(false);
		m_currentNode = 0;
		setEnabled(false);
	}
}


void GameControllerWidget::addComponent()
{
	QMenu popup;
	popup.addActions(m_attachmentTreeView->actions());
	popup.exec(mapToGlobal(m_addComponent->pos()));
	updateActions();
}

void GameControllerWidget::removeComponent()
{
	m_attachmentTreeView->removeCurrentNode(false);
	updateActions();
}

void GameControllerWidget::componentSelected(QXmlTreeNode *node)
{
	emit phonemeFileChanged();
	if (m_physicsWidget->setCurrentNode(node))
		m_componentWidgets->setCurrentWidget(m_physicsWidgetContainer);
	else if (m_ttsWidget->setCurrentNode(node))
		m_componentWidgets->setCurrentWidget(m_ttsWidgetContainer);
	else if (m_keyframeAnimWidget->setCurrentNode(node))
		m_componentWidgets->setCurrentWidget(m_keyframeAnimWidgetContainer);
	else if (m_crowdParticleWidget->setCurrentNode(node))
		m_componentWidgets->setCurrentWidget(m_crowdParticleWidgetContainer);
	else if (m_crowdVisibilityWidget->setCurrentNode(node))
		m_componentWidgets->setCurrentWidget(m_crowdVisibilityWidgetContainer);
	else if (m_soundWidget->setCurrentNode(node))
		m_componentWidgets->setCurrentWidget(m_soundWidgetContainer);
	else
	{
		m_componentWidgets->setVisible(false);
		return;
	}
	m_componentWidgets->setVisible(true);

}

void GameControllerWidget::switchPhonemeEditor(bool activate /*=false*/, unsigned int id /*=0*/, const QString& path /*=0*/)
{
	if(activate)
	{
		m_componentWidgets->setCurrentWidget(m_phonemeEditorWidgetContainer);
		m_phonemeEditorWidget->loadPhonemeFile(id, path);
	}
	else
		m_componentWidgets->setCurrentWidget(m_soundWidgetContainer);
	m_componentWidgets->setVisible(true);
}

void GameControllerWidget::entityChanged(const QModelIndex& first, const QModelIndex& last)
{
	if (first.row() == -1 && first.column() == 0 && first.internalPointer())
	{
		QAttachmentNode* node = static_cast<QAttachmentNode*>(first.internalPointer());
		m_entityName->blockSignals(true);
		m_entityName->setText(node->name());
		m_entityName->blockSignals(false);
	}
}

void GameControllerWidget::entityNameChanged(const QString& name)
{
	QString tip;
	if (name.isEmpty())
	{
		m_entityName->setStyleSheet("background-color: red");
		tip = tr("You have to specify an unique entity name");				
	}
	else if ( GameEngine::entityWorldID(qPrintable(name)) != 0 )
	{
		m_entityName->setStyleSheet("background-color: yellow");
		tip = tr("Another entity with this name already exists");				
	}
	else
	{
		m_entityName->setStyleSheet("");
		m_currentNode->setProperty("Name", name);		
	}
	m_entityName->setStatusTip(tip);
	m_entityName->setToolTip(tip);	
}

void GameControllerWidget::updateActions()
{
	m_actionAddPhysicsComponent->setEnabled( m_currentNode->xmlNode().firstChildElement("BulletPhysics").isNull() );
	m_actionAddPhysicsComponent->setStatusTip( m_actionAddPhysicsComponent->isEnabled() ?
		tr("Adds a physics component based on the Bullet Physics Engine") : 
		tr("There is already another BulletPhysics component in this entity"));

	m_actionAddTTSComponent->setEnabled( m_currentNode->xmlNode().firstChildElement("TTS").isNull() && m_currentNode->xmlNode().firstChildElement("Sapi").isNull());	
	m_actionAddTTSComponent->setStatusTip( m_actionAddTTSComponent->isEnabled() ?
		tr("Adds a text to speech component based on the Microsoft Speech API") : 
		tr("There is already another Sapi component in this entity"));

	m_actionAddCrowdParticleComponent->setEnabled( m_currentNode->xmlNode().firstChildElement("CrowdParticle").isNull() );	
	m_actionAddCrowdParticleComponent->setStatusTip( m_actionAddCrowdParticleComponent->isEnabled() ?
		tr("Adds a crowd particle to the crowd simulation") : 
		tr("There is already another crowd particle component in this entity"));

	m_actionAddCrowdVisNodeComponent->setEnabled( m_currentNode->xmlNode().firstChildElement("CrowdVisNode").isNull() );	
	m_actionAddCrowdVisNodeComponent->setStatusTip( m_actionAddCrowdVisNodeComponent->isEnabled() ?
		tr("Adds a visibility node to the crowd simulation") : 
		tr("There is already another visibility node component in this entity"));

	
	m_actionAddSound3DComponent->setEnabled( m_currentNode->xmlNode().firstChildElement("Sound3D").isNull() );	
	m_actionAddSound3DComponent->setStatusTip( m_actionAddSound3DComponent->isEnabled() ?
		tr("Adds a Sound3D node to the scene") : 
		tr("There is already another Sound3D component in this entity"));

	int nodeID = m_currentSceneNode->property("ID").toInt();
	m_actionAddKeyframeAnimComponent->setEnabled(false);
	if( nodeID == 0 || h3dGetNodeType( nodeID ) != H3DNodeTypes::Model)
		m_actionAddKeyframeAnimComponent->setStatusTip("Disabled because the entity contains no ModelNode scengraph component ");	
	else if( !m_currentNode->xmlNode().firstChildElement("KeyframeAnimation").isNull() )
		m_actionAddKeyframeAnimComponent->setStatusTip("There is already another KeyframeAnim component");	
	else
	{
		m_actionAddKeyframeAnimComponent->setEnabled(true);
		m_actionAddKeyframeAnimComponent->setStatusTip("Adds a component to control keyframe animations");
	}
}