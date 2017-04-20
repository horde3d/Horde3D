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

#ifndef QCAMERANODE_H_
#define QCAMERANODE_H_

#include "QSceneNode.h"

/**
 * A camera node item for the SceneTreeModel
 */
class QCameraNode : public QSceneNode
{
	Q_OBJECT
	Q_CLASSINFO("QCameraNode", "Camera specific");

	Q_PROPERTY(Frustum Frustum READ frustum WRITE setFrustum DESIGNABLE true USER true)
	Q_PROPERTY(bool Keep_Aspect_Ratio READ keepAspect WRITE setKeepAspect DESIGNABLE true USER true)
	Q_PROPERTY(Pipeline Pipeline READ pipeline WRITE setPipeline DESIGNABLE true USER true)
	Q_PROPERTY(bool Ortho READ ortho WRITE setOrtho DESIGNABLE true USER true)
	Q_PROPERTY(bool Occlusion_Culling READ occlusionCulling WRITE setOcclusionCulling DESIGNABLE true USER true)

public:
	static QSceneNode* loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent);

	QCameraNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);
	virtual ~QCameraNode();

	Frustum frustum() const;
	void setFrustum(const Frustum& frustum);

	/**
	 * Returns the pipeline resource of this camera
	 */
	Pipeline pipeline() const;
	/**
	 * Sets the pipeline resource of this camera
	 */
	void setPipeline(const Pipeline& pipeline);

	/**
	 * Indicates if the aspect ratio is being kept when the viewport is resized
	 */
	bool keepAspect() const;
	
	/**
	 * Toggles the settings for keeping the aspect ratio when resizing the viewport
	 * @param keep if true the camera frustum will be adjusted when resizing the viewport
	 */
	void setKeepAspect(bool keep);

	/**
	 * @return True if camera is configured for orthogonal projection, otherwise false 
	 */
	bool ortho() const;

	/**
	 * Configures camera to use orthogonal projection
	 * @param ortho true = orthogonal projection, false = perspective projection
	 */
	void setOrtho(bool ortho);


	bool occlusionCulling() const;

	void setOcclusionCulling(bool culling);

	/**
	 * Activates this camera
	 */
	void activate();

        QString toolTip( int /*column*/ ) { return tr("Double click to activate this camera"); }

public slots:

	void viewportResized(int width, int height);

signals:
	void pipelineChanged(QCameraNode*);

protected:
	void addRepresentation();

private:

        int     m_pipelineID;
        int     m_viewportWidth;
        int     m_viewportHeight;

};
#endif
