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
#include "KeyframeAnimationWidget.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtWidgets/QMessageBox>

#include <QXmlTree/QXmlTreeNode.h>
#include <GameEngine/GameEngine.h>
#include <GameEngine/GameEngine_Animations.h>

#include <Horde3D/Horde3DUtils.h>

Q_DECLARE_METATYPE(QDomElement)

KeyframeAnimationWidget::KeyframeAnimationWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags)
{
	setupUi(this);
	m_addAnim->setDefaultAction(m_actionAddAnim);
	m_removeAnim->setDefaultAction(m_actionRemoveAnim);
	m_playButton->setDefaultAction(m_actionPlayAnim);
	connect(m_actionAddAnim, SIGNAL(triggered()), this, SLOT(addAnimation()));
	connect(m_actionRemoveAnim, SIGNAL(triggered()), this, SLOT(removeAnimation()));
	connect(m_actionPlayAnim, SIGNAL(triggered()), this, SLOT(playAnimation()));
	connect(m_animations, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(updateAnimation(QTreeWidgetItem*, int)));
	connect(m_animations, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
}

KeyframeAnimationWidget::~KeyframeAnimationWidget()
{
}

bool KeyframeAnimationWidget::setCurrentNode(QXmlTreeNode *node)
{
	m_animations->clear();
	if (node && node->xmlNode().tagName() == "KeyframeAnimation")
	{				
		m_currentNode = 0;
		QDomNodeList animations = node->xmlNode().elementsByTagName("StaticAnimation");
		for (int i=0; i<animations.size(); ++i)
		{
			QDomElement anim = animations.at(i).toElement();
			QStringList attributes;
			attributes << anim.attribute("name") << anim.attribute("file")  << anim.attribute("fps", "30");			
			QTreeWidgetItem* item = new QTreeWidgetItem(m_animations, attributes);		
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			item->setData(0, Qt::UserRole, QVariant::fromValue<QDomElement>(anim));
		}
		m_currentNode = node;
		return true;
	}
	else
		return false;
}

void KeyframeAnimationWidget::addAnimation()
{
	QStringList animations = QFileDialog::getOpenFileNames(
		this, 
		tr("Select animation(s)"), 
		QDir::currentPath(), 
		tr("Animation files (*.anim);;All files (*.*)"));	

	QStringList errorList;
	QDir baseDir = QDir::current();
	for( int i = 0; i < animations.size(); ++i )
	{
		if( !animations[i].contains( baseDir.absolutePath() ) )
		{
			if( !QFile::copy( animations[i], baseDir.absoluteFilePath( QFileInfo(animations[i]).fileName() ) ) )
			{
				errorList << animations[i];
				continue;
			}
			else
				animations[i] = baseDir.absoluteFilePath( QFileInfo(animations[i]).fileName() );
		}
		QDomElement anim = m_currentNode->xmlNode().appendChild(QDomDocument().createElement("StaticAnimation")).toElement();		
		QStringList attributes;
		attributes << QFileInfo(animations[i]).baseName() << baseDir.relativeFilePath( QFileInfo(animations[i]).fileName() ) << "30";
		anim.setAttribute("name", attributes[0]);
		anim.setAttribute("file", attributes[1]);
		anim.setAttribute("fps", attributes[2]);
		QTreeWidgetItem* item = new QTreeWidgetItem(m_animations, attributes);		
		item->setData(0, Qt::UserRole, QVariant::fromValue<QDomElement>(anim));
		item->setFlags(item->flags() | Qt::ItemIsEditable);
	}
	if( !errorList.isEmpty() )
		QMessageBox::warning(this, tr("Error"), 
		tr("The following files couldn't be copied to the\n"
		"animation directory: %1").arg(baseDir.absolutePath())+
		"\n\n"+errorList.join("\n"));

	if ( !animations.empty() )
	{
		emit modified(true);	
		updateGameEngine();
	}
}

void KeyframeAnimationWidget::removeAnimation()
{
	QList<QTreeWidgetItem*> items = m_animations->selectedItems();
	foreach(QTreeWidgetItem* item, items)
		m_currentNode->xmlNode().removeChild(item->data(0, Qt::UserRole).value<QDomElement>());
	qDeleteAll(items);
	updateGameEngine();
	emit modified(true);
}

void KeyframeAnimationWidget::updateGameEngine()
{
	QString data;
	QTextStream stream(&data);
	m_currentNode->xmlNode().save(stream, 4);
	unsigned int entityID = GameEngine::entityWorldID( qPrintable(m_currentNode->xmlNode().parentNode().toElement().attribute("name")) );
	GameEngine::setComponentData( entityID, "KeyframeAnimation", qPrintable(data));
}

void KeyframeAnimationWidget::updateAnimation(QTreeWidgetItem* item, int column)
{
	switch (column)
	{
	case 0:
		item->data(0, Qt::UserRole).value<QDomElement>().setAttribute("name", item->text(0));
		break;
	case 1:
		item->data(0, Qt::UserRole).value<QDomElement>().setAttribute("file", item->text(1));
		break;
	case 2:
		item->data(0, Qt::UserRole).value<QDomElement>().setAttribute("fps", item->text(2));
		break;
	}
	if (m_currentNode)
	{
		updateGameEngine();
		emit modified(true);
	}

}

void KeyframeAnimationWidget::playAnimation()
{
	if( GameEngine::isPlaying( entityWorldID(), qPrintable(m_animations->selectedItems()[0]->text(0)) ) )
	{
		updateGameEngine();
	}
	else
	{
		GameEngine::playAnim( entityWorldID(), qPrintable(m_animations->selectedItems()[0]->text(0)), 0, 1, -1, m_animations->selectedItems()[0]->text(2).toInt() );
	}
	selectionChanged();
}

void KeyframeAnimationWidget::selectionChanged()
{
	m_actionRemoveAnim->setEnabled( !m_animations->selectedItems().empty() );
	if( m_animations->selectedItems().size() == 1 )
	{
		m_actionPlayAnim->setEnabled( true );
		if( GameEngine::isPlaying( 
			entityWorldID(),
			qPrintable(m_animations->selectedItems()[0]->text(0))) )
			m_actionPlayAnim->setText("Stop");
		else
			m_actionPlayAnim->setText("Play");
	}
	else
	{
		m_actionPlayAnim->setEnabled(false);
		m_actionPlayAnim->setText("Play");
	}
}

unsigned int KeyframeAnimationWidget::entityWorldID()
{
	if( m_currentNode )
		return GameEngine::entityWorldID( qPrintable(m_currentNode->xmlNode().parentNode().toElement().attribute("name")) );
	else
		return 0;
}

