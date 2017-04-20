// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Vogelhuber
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
#include "CameraToolBar.h"

#include "QCameraNode.h"

#include <Horde3D.h>

CameraToolBar::CameraToolBar(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags), m_orthoScale(10)
{
	setupUi(this);
	connect(m_activeCam, SIGNAL(currentIndexChanged(int)), this, SLOT(currentCamChanged(int)));
	connect(m_sceneToolButton, SIGNAL(toggled(bool)), this, SLOT(activateSpecialCam(bool)));
	connect(m_frontToolButton, SIGNAL(toggled(bool)), this, SLOT(activateSpecialCam(bool)));
	connect(m_topToolButton, SIGNAL(toggled(bool)), this, SLOT(activateSpecialCam(bool)));
	connect(m_rightToolButton, SIGNAL(toggled(bool)), this, SLOT(activateSpecialCam(bool)));
}


CameraToolBar::~CameraToolBar()
{
}

void CameraToolBar::setActiveCamera(const QCameraNode* camera)
{	
	int index = m_activeCam->findData(QVariant::fromValue<void*>(const_cast<QCameraNode*>(camera)));
	int oldIndex = m_activeCam->currentIndex();
	m_activeCam->setCurrentIndex(index);
	if( oldIndex == index )
		currentCamChanged(index);
}

void CameraToolBar::setActiveCamera(const QString& name)
{
	int index = m_activeCam->findText(name);
	int oldIndex = m_activeCam->currentIndex();
	m_activeCam->setCurrentIndex(index);
	if( oldIndex == index )
		currentCamChanged(index);
	
}

int CameraToolBar::currentPipelineID() const
{
	const QCameraNode* camera = static_cast<const QCameraNode*>(m_activeCam->itemData(m_activeCam->currentIndex()).value<void*>());
	if( camera )
		return camera->pipeline().ResourceID;
	else
		return 0;
}

const QString CameraToolBar::currentPipelineFile() const
{
	const QCameraNode* camera = static_cast<const QCameraNode*>(m_activeCam->itemData(m_activeCam->currentIndex()).value<void*>());
	if( camera )
		return camera->pipeline().FileName;
	else
		return QString();
}

QString CameraToolBar::activeCameraName() const
{
	return m_activeCam->currentText();
}

void CameraToolBar::pipelineChanged(QCameraNode* camera)
{
	if (camera == static_cast<QCameraNode*>(m_activeCam->itemData(m_activeCam->currentIndex()).value<void*>()) || m_activeCam->currentIndex() == -1)
		emit activeCamChanged(camera); // Will result in a reload of the camera and in a call to setActiveCamera
}


void CameraToolBar::viewportResized(int width, int height)
{
	m_viewportWidth = width;
	m_viewportHeight = height;
	QCameraNode* camera = static_cast<QCameraNode*>(m_activeCam->itemData(m_activeCam->currentIndex()).value<void*>());
	if( camera == 0x0 ) return;
	if( m_sceneToolButton->isChecked() )
	{
		Frustum f = camera->frustum();
		double factor = 1.0;
		if( camera->keepAspect() )
			factor = ( width * (f.Top - f.Bottom) ) / ( height * (f.Right - f.Left) );		
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::LeftPlaneF, 0, f.Left * factor );
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::RightPlaneF, 0, f.Right * factor );
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::TopPlaneF, 0, f.Top );
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::BottomPlaneF, 0, f.Bottom );
	}
	else
	{
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::LeftPlaneF, 0, -1 );
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::RightPlaneF, 0, 1 );
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::TopPlaneF, 0, 1 );
		h3dSetNodeParamF( camera->hordeId(), H3DCamera::BottomPlaneF, 0, -1 );
	}
	h3dSetNodeParamI( camera->hordeId(), H3DCamera::ViewportXI, 0 );
	h3dSetNodeParamI( camera->hordeId(), H3DCamera::ViewportYI, 0 );
	h3dSetNodeParamI( camera->hordeId(), H3DCamera::ViewportWidthI, width );
	h3dSetNodeParamI( camera->hordeId(), H3DCamera::ViewportHeightI, height );
	h3dResizePipelineBuffers( camera->pipeline().ResourceID, width, height );
}

void CameraToolBar::nodePropertyChanged( QXmlTreeNode* node )
{
	for( int i = 0; i <  m_activeCam->count(); ++i )
	{
		QCameraNode* camera = static_cast<QCameraNode*>(m_activeCam->itemData(i).value<void*>());
		if( camera == node )
		{
			m_activeCam->setItemText(i, camera->name());
			break;
		}
	}
}

void CameraToolBar::currentCamChanged(int index)
{
	QCameraNode* camera = static_cast<QCameraNode*>(m_activeCam->itemData(index).value<void*>());
	emit activeCamChanged(camera);
	if( camera )
	{
		QVec3f pos = camera->position();
		m_orthoX = m_sceneX = pos.X; m_orthoY = m_sceneY = pos.Y; m_orthoZ = m_sceneZ = pos.Z;		
		QVec3f rot = camera->rotation();
		m_sceneRX = rot.X; m_sceneRY = rot.Y; m_sceneRZ = rot.Z;		
		m_sceneScale = camera->scale().X;
		if ( m_sceneToolButton->isChecked() )
		{
			h3dSetNodeParamI(camera->hordeId(), H3DCamera::OrthoI, camera->ortho());
			h3dSetNodeTransform(camera->hordeId(), m_sceneX, m_sceneY, m_sceneZ, m_sceneRX, m_sceneRY, m_sceneRZ, m_sceneScale, m_sceneScale, m_sceneScale);
			viewportResized(m_viewportWidth, m_viewportHeight);
		}
		else
			m_sceneToolButton->setChecked(true);
		// Set after scene button has been enabled
		float farPlane = h3dGetNodeParamF(camera->hordeId(), H3DCamera::FarPlaneF, 0);
		m_rightX = pos.X - farPlane * 0.25f;	m_topY = pos.Y + farPlane * 0.25f;	m_frontZ = pos.Z + farPlane * 0.25f;
	}
}

void CameraToolBar::activateSpecialCam(bool checked)
{
	QCameraNode* camera = static_cast<QCameraNode*>(m_activeCam->itemData(m_activeCam->currentIndex()).value<void*>());
	if( camera == 0x0 ) return;
	if (sender() == m_sceneToolButton)
	{
		if (!checked)
			h3dGetNodeTransform(camera->hordeId(), &m_sceneX, &m_sceneY, &m_sceneZ, &m_sceneRX, &m_sceneRY, &m_sceneRZ, &m_sceneScale, 0, 0);
		else
		{
			h3dSetNodeParamI(camera->hordeId(), H3DCamera::OrthoI, camera->ortho());
			h3dSetNodeTransform(camera->hordeId(), m_sceneX, m_sceneY, m_sceneZ, m_sceneRX, m_sceneRY, m_sceneRZ, m_sceneScale, m_sceneScale, m_sceneScale);
			viewportResized(m_viewportWidth, m_viewportHeight);
		}
	}
	else if (sender() == m_topToolButton)
	{		
		if (!checked && h3dGetNodeParamI(camera->hordeId(), H3DCamera::OrthoI) == 1)
		{
			h3dGetNodeTransform(camera->hordeId(), &m_orthoX, &m_topY, &m_orthoZ, 0, 0, 0, &m_orthoScale, 0, 0);
		}
		else if( checked )
		{
			h3dSetNodeParamI(camera->hordeId(), H3DCamera::OrthoI, true);
			h3dSetNodeTransform(camera->hordeId(), m_orthoX, m_topY, m_orthoZ, -90, 0, 0, m_orthoScale, m_orthoScale, m_orthoScale);
			viewportResized(m_viewportWidth, m_viewportHeight);
		}
	}
	else if (sender() == m_rightToolButton)
	{
		if (!checked && h3dGetNodeParamI(camera->hordeId(), H3DCamera::OrthoI) == 1)
		{
			h3dGetNodeTransform(camera->hordeId(), &m_rightX, &m_orthoY, &m_orthoZ, 0, 0, 0, &m_orthoScale, 0, 0);
		}
		else if( checked )
		{
			h3dSetNodeParamI(camera->hordeId(), H3DCamera::OrthoI, true);
			h3dSetNodeTransform(camera->hordeId(), m_rightX, m_orthoY, m_orthoZ, 0, -90, 0, m_orthoScale, m_orthoScale, m_orthoScale);
			viewportResized(m_viewportWidth, m_viewportHeight);
		}
	}
	else if (sender() == m_frontToolButton)
	{
		if (!checked && h3dGetNodeParamI(camera->hordeId(), H3DCamera::OrthoI) == 1)
		{
			h3dGetNodeTransform(camera->hordeId(), &m_orthoX, &m_orthoY, &m_frontZ, 0, 0, 0, &m_orthoScale, 0, 0);
		}
		else if( checked )
		{
			h3dSetNodeParamI(camera->hordeId(), H3DCamera::OrthoI, true);
			h3dSetNodeTransform(camera->hordeId(), m_orthoX, m_orthoY, m_frontZ , 0, 0, 0, m_orthoScale, m_orthoScale, m_orthoScale);
			viewportResized(m_viewportWidth, m_viewportHeight);
		}
	}
}


