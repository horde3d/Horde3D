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

#include "QCameraNodePage.h"

#include <math.h>
#include <Horde3DUtils.h>

#include "HordeSceneEditor.h"


QCameraNodePage::QCameraNodePage(/*const QDomElement& init = QDomElement(),*/ QWidget* parent /*= 0*/) : QWizardPage(parent)
//m_init(init)
{
	setupUi(this);
	setTitle(tr("Add new camera"));
	setSubTitle(tr("You can define several cameras within one scene. But only one can be active at the same time."));
	m_leftPlane->setValidator(new QDoubleValidator(m_leftPlane));
	m_rightPlane->setValidator(new QDoubleValidator(m_rightPlane));
	m_bottomPlane->setValidator(new QDoubleValidator(m_bottomPlane));
	m_topPlane->setValidator(new QDoubleValidator(m_topPlane));
	registerField("cameraname", m_name);
	registerField("asymFrustum", m_asymFrustum, "checked", SIGNAL(toggled(bool)));
	registerField("symFrustum", m_symFrustum, "checked", SIGNAL(toggled(bool)));
	registerField("camerafov", m_fov, "value", SIGNAL(valueChanged(double)));
	registerField("quo", m_aspQuo, "value", SIGNAL(valueChanged(double)));
	registerField("div", m_aspDiv);
	registerField("symNear", m_symNearClipping, "value", SIGNAL(valueChanged(double)));
	registerField("symFar", m_symFarClipping, "value", SIGNAL(valueChanged(double)));
	registerField("leftPlane", m_leftPlane);
	registerField("rightPlane", m_rightPlane);
	registerField("topPlane", m_topPlane);
	registerField("bottomPlane", m_bottomPlane);
	registerField("asymNear", m_asymNearClipping, "value", SIGNAL(valueChanged(double)));
	registerField("asymFar", m_asymFarClipping, "value", SIGNAL(valueChanged(double)));
	registerField("pipeline", m_pipeline, "pipelineFile", SIGNAL(pipelineChanged()));
	connect( m_pipeline, SIGNAL( pipelineChanged() ), this, SIGNAL( completeChanged() ) );
	connect(m_importButton, SIGNAL(clicked()), this, SLOT(importCamera()));
	connect(m_asymFrustum, SIGNAL(toggled(bool)), this, SLOT(switchPage(bool)));
}


QCameraNodePage::~QCameraNodePage()
{
}

void QCameraNodePage::initializePage()
{
	// init from pathpage wizard if available
	m_pipeline->init( QDir::currentPath() );
		
}
//void QCameraNodePage::initializePage()
//{
//	if (!m_init.isNull())
//	{
//		m_name->setText(m_init.attribute("name"));
//		m_symFarClipping->setValue(m_init.attribute("farPlane", "1000").toFloat());
//		m_symNearClipping->setValue(m_init.attribute("nearPlane", "0.1").toFloat());
//		m_asymFarClipping->setValue(m_symFarClipping->value());
//		m_asymNearClipping->setValue(m_symNearClipping->value());
//		m_leftPlane->setText(m_init.attribute("leftPlane", "-0.055228457"));
//		m_rightPlane->setText(m_init.attribute("rightPlane", "0.055228457"));
//		m_topPlane->setText(m_init.attribute("topPlane", "0.041421354"));
//		m_bottomPlane->setText(m_init.attribute("bottomPlane", "-0.041421354"));
//		if (m_leftPlane->text().toFloat() == -m_rightPlane->text().toFloat() &&
//			m_topPlane->text().toFloat() == -m_bottomPlane->text().toFloat())
//		{
//			m_symFrustum->setChecked(true);			
//			m_fov->setValue(atanf( m_topPlane->text().toFloat() / m_asymNearClipping->text().toFloat() ) * 360.0f / 3.1415926);			
//			m_aspQuo->setValue(m_rightPlane->text().toFloat() / m_topPlane->text().toFloat());
//			m_aspDiv->setValue(1.0);
//		}
//		else
//			m_asymFrustum->setChecked(true);			
//	}
//}

bool QCameraNodePage::isComplete() const
{
	return !field("pipeline").toString().isEmpty();
}

void QCameraNodePage::switchPage(bool asym)
{
	if( asym )
		m_stackedWidget->setCurrentIndex(1);
	else
		m_stackedWidget->setCurrentIndex(0);
}

void QCameraNodePage::importCamera()
{
}
