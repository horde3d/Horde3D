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
#ifndef CAMERATOOLBAR_H_
#define CAMERATOOLBAR_H_

#include "./ui_CameraToolBar.h"

class QCameraNode;
class QXmlTreeNode;

class CameraToolBar : public QWidget, protected Ui_CameraToolBar
{
	Q_OBJECT

	friend class HordeSceneEditor;

public:
	CameraToolBar(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~CameraToolBar();

	void setActiveCamera(const QCameraNode* camera);
	void setActiveCamera(const QString& name);

        int currentPipelineID() const;

	const QString currentPipelineFile() const;

	QString activeCameraName() const;	

public slots:
	void pipelineChanged(QCameraNode* camera);
	void viewportResized(int width, int height);

	void nodePropertyChanged( QXmlTreeNode* node );

signals:
	void activeCamChanged(const QCameraNode* camera);

private slots:
	void currentCamChanged(int index);
	void activateSpecialCam(bool checked);

private:
	int		m_viewportWidth;
	int		m_viewportHeight;

	float	m_sceneX, m_sceneY, m_sceneZ;
	float	m_sceneRX, m_sceneRY, m_sceneRZ;
	float	m_orthoX, m_orthoY, m_orthoZ;	
	float   m_orthoScale, m_sceneScale;
	float	m_topY, m_rightX, m_frontZ;

};
#endif
