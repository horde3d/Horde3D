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
#include "CrowdParticleWidget.h"
#include <QXmlTree/QXmlTreeNode.h>

#include <GameEngine/GameEngine.h>

#ifdef UNI_AUGSBURG
#include <GameEngine/GameEngine_CrowdSim.h>
#endif

CrowdParticleWidget::CrowdParticleWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags)
{
	setupUi(this);

#ifdef UNI_AUGSBURG
	m_simSpeed->setValue( GameEngine::crowdSimSpeed() );
	m_minTargetDist->setValue( GameEngine::crowdSimMinTargetDist() );
	m_crowdSim->setChecked( GameEngine::crowdSimEnabled() );
#endif

	connect(m_d1, SIGNAL(valueChanged ( double )), this, SLOT(updateZone( double )));
	connect(m_d2, SIGNAL(valueChanged ( double )), this, SLOT(updateZone( double )));
	connect(m_d3, SIGNAL(valueChanged ( double )), this, SLOT(updateZone( double )));
	connect(m_f1, SIGNAL(valueChanged ( double )), this, SLOT(updateZone( double )));
	connect(m_f2, SIGNAL(valueChanged ( double )), this, SLOT(updateZone( double )));
	connect(m_f3, SIGNAL(valueChanged ( double )), this, SLOT(updateZone( double )));

	connect(m_fixedParticle, SIGNAL(toggled(bool)), this, SLOT(updateFixedState(bool)));

	connect(m_injectorTag, SIGNAL(valueChanged(int)), this, SLOT(updateTags(int)));
	connect(m_targetTag, SIGNAL(valueChanged(int)), this, SLOT(updateTags(int)));

	connect(m_crowdSim, SIGNAL(toggled(bool)), this, SLOT(updateCrowdSim()));
	connect(m_simSpeed, SIGNAL(valueChanged(double)), this, SLOT(updateCrowdSim()));
	connect(m_minTargetDist, SIGNAL(valueChanged(double)), this, SLOT(updateCrowdSim()));

}


CrowdParticleWidget::~CrowdParticleWidget()
{
}

void CrowdParticleWidget::updateZone(double value)
{
	if( m_currentNode == 0x0 ) return;
	
	m_currentNode->xmlNode().setAttribute("d1", m_d1->value() );
	m_currentNode->xmlNode().setAttribute("d2", m_d2->value() );
	m_currentNode->xmlNode().setAttribute("d3", m_d3->value() );
	m_currentNode->xmlNode().setAttribute("f1", m_f1->value() );
	m_currentNode->xmlNode().setAttribute("f2", m_f2->value() );
	m_currentNode->xmlNode().setAttribute("f3", m_f3->value() );

	QString entityName = m_currentNode->xmlNode().parentNode().toElement().attribute("name");

	unsigned int entityID = GameEngine::entityWorldID( qPrintable( entityName ) );
#ifdef UNI_AUGSBURG
	GameEngine::setCrowdParticleZone( entityID, m_d1->value(), m_d2->value(), m_d3->value(), m_f1->value(), m_f2->value(), m_f3->value() ); 
#endif
}

void CrowdParticleWidget::updateTags(int value)
{
	if( m_currentNode == 0x0 ) return;

	m_currentNode->xmlNode().setAttribute("targetTag", m_targetTag->value() );
	m_currentNode->xmlNode().setAttribute("injTag", m_injectorTag->value() );

	QString entityName = m_currentNode->xmlNode().parentNode().toElement().attribute("name");
	unsigned int entityID = GameEngine::entityWorldID( qPrintable( entityName ) );
#ifdef UNI_AUGSBURG
	GameEngine::setCrowdParticleTargetTag( entityID, m_targetTag->value() ); 
	GameEngine::setCrowdParticleInjectorTag( entityID, m_injectorTag->value() ); 
#endif
}

void CrowdParticleWidget::updateFixedState( bool fixed )
{
	if( m_currentNode == 0x0 ) return;

	m_currentNode->xmlNode().setAttribute("fixed", m_fixedParticle->isChecked());
	
	QString entityName = m_currentNode->xmlNode().parentNode().toElement().attribute("name");
	unsigned int entityID = GameEngine::entityWorldID( qPrintable( entityName ) );
#ifdef UNI_AUGSBURG
	GameEngine::setCrowdParticleFixedState( entityID, m_fixedParticle->isChecked() ); 
#endif
}

void CrowdParticleWidget::updateCrowdSim()
{
	if( m_currentNode ) 
	{
#ifdef UNI_AUGSBURG
		GameEngine::enableCrowdSim( m_crowdSim->isChecked() );
		GameEngine::setCrowdSimSpeed( m_simSpeed->value() );
		GameEngine::setCrowdSimMinTargetDist( m_minTargetDist->value() );
#endif
	}
}

bool CrowdParticleWidget::setCurrentNode(QXmlTreeNode* node)
{	
#ifdef UNI_AUGSBURG
	if (node && (node->xmlNode().tagName() == "CrowdParticle") )
	{
		m_currentNode = 0;

		m_fixedParticle->setChecked(
			node->xmlNode().attribute("fixed", "false").compare("true", Qt::CaseInsensitive) == 0 ||
			node->xmlNode().attribute("fixed", "false").compare("1", Qt::CaseInsensitive) == 0 );

		m_targetTag->setValue(node->xmlNode().attribute("targetTag").toInt());
		m_injectorTag->setValue(node->xmlNode().attribute("injTag").toInt());

		m_d1->setValue(node->xmlNode().attribute("d1").toDouble());
		m_d2->setValue(node->xmlNode().attribute("d2").toDouble());
		m_d3->setValue(node->xmlNode().attribute("d3").toDouble());

		m_f1->setValue(node->xmlNode().attribute("f1").toDouble());
		m_f2->setValue(node->xmlNode().attribute("f2").toDouble());
		m_f3->setValue(node->xmlNode().attribute("f3").toDouble());

		m_crowdSim->setChecked( GameEngine::crowdSimEnabled() );
		m_simSpeed->setValue( GameEngine::crowdSimSpeed() );
		m_minTargetDist->setValue( GameEngine::crowdSimMinTargetDist() );
		
		m_currentNode = node;
		return true;
	}
	else
#endif
		return false;
}


