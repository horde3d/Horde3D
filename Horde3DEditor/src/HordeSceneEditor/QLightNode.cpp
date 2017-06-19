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


#include "QLightNode.h"

#include <QtCore/QTextStream>
#include <QtCore/QFile>

#include <QXmlTree/QXmlNodePropertyCommand.h>
#include "HordeSceneEditor.h"
#include "AttachmentPlugIn.h"
#include "GLWidget.h"
#include "SceneTreeModel.h"
#include "PlugInManager.h"

#include "SceneFile.h"

#include <Horde3D.h>
#include <Horde3DUtils.h>

QSceneNode* QLightNode::loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent)
{
	return new QLightNode(xmlNode, row, model, parent);
}

QLightNode::QLightNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode) : 
	QSceneNode(xmlNode, row, model, parentNode), m_resourceID(0), 
                m_debugID(0), m_debugResourceMat(0), m_debugResourceGeo(0)
{
	setObjectName(tr("Light"));
	addRepresentation();
}

QLightNode::~QLightNode()
{
	if (m_resourceID != 0)
		h3dRemoveResource(m_resourceID);
	//removeDebugRepresentation();
}

void QLightNode::addRepresentation()
{	
	if ( m_xmlNode.hasAttribute("material") )
		m_resourceID = h3dAddResource( H3DResTypes::Material, qPrintable(m_xmlNode.attribute("material")), 0 );

	QSceneNode* parentNode = static_cast<QSceneNode*>(parent());
	unsigned int rootID = parentNode ? parentNode->hordeId() : H3DRootNode;

	m_hordeID = h3dAddLightNode(
		rootID, 
		qPrintable(m_xmlNode.attribute("name", "ATTENTION No Node Name")), 
		m_resourceID,
		qPrintable(m_xmlNode.attribute("lightingContext", "LIGHTING")),  
		qPrintable(m_xmlNode.attribute("shadowContext", "SHADOWMAP")));

	h3dSetNodeParamF(m_hordeID, H3DLight::FovF, 0, fov());
	h3dSetNodeParamI(m_hordeID, H3DLight::ShadowMapCountI, shadowMapCount());
	h3dSetNodeParamF(m_hordeID, H3DLight::ShadowMapBiasF, 0, shadowMapBias());
	h3dSetNodeParamF(m_hordeID, H3DLight::ShadowSplitLambdaF, 0, lambda());
	QColor c = color();
	h3dSetNodeParamF(m_hordeID, H3DLight::ColorF3, 2, c.blueF());
	h3dSetNodeParamF(m_hordeID, H3DLight::ColorF3, 1, c.greenF());
	h3dSetNodeParamF(m_hordeID, H3DLight::ColorF3, 0, c.redF());
	h3dSetNodeParamF(m_hordeID, H3DLight::RadiusF, 0, radius());
	h3dSetNodeParamF(m_hordeID, H3DLight::ColorMultiplierF, 0, intensity());

	// load transformation from file...
	float x, y, z, rx, ry, rz, sx, sy, sz;
	getTransformation(x,y,z,rx,ry,rz,sx,sy,sz);
	// ...and update scene representation
	h3dSetNodeTransform(m_hordeID, x, y, z, rx, ry, rz, sx, sy, sz);
	// Attachment
	QDomElement attachment = m_xmlNode.firstChildElement("Attachment");	
	SceneTreeModel* model = static_cast<SceneTreeModel*>(m_model);
	AttachmentPlugIn* plugIn = model->nodeFactory()->attachmentPlugIn();
	if (!attachment.isNull() &&  plugIn != 0)
		plugIn->initNodeAttachment(this);
	// add debug representation
	//addDebugRepresentation();
}

//void QLightNode::addDebugRepresentation()
//{
//	// Add Light Geometry
//	m_debugResourceGeo = h3dAddResource(H3DResTypes::Geometry, "DebugLightCone.geo", 0);
//	QFile lightDebugGeometry(":/Models/Resources/DebugLightCone.geo");	
//	if (!lightDebugGeometry.open(QIODevice::ReadOnly))
//	{
//		qWarning("Error opening resource: %s", qPrintable(lightDebugGeometry.fileName()));
//		return;
//	}
//	QByteArray data = lightDebugGeometry.readAll();
//	QString path = h3dutGetResourcePath(H3DResTypes::Geometry);
//	path += "DebugLightCone.geo";
//	h3dLoadResource(qPrintable(path), data.constData(), data.size());
//	// add light cone shader	
//	ResHandle shader = h3dAddResource(H3DResTypes::Shader, "DebugLightCone.shader.xml", 0);
//	QFile lightDebugShader(":/Shader/Resources/DebugLightCone.shader.xml");	
//	if (!lightDebugShader.open(QIODevice::ReadOnly))
//	{
//		qWarning("Error opening resource: %s", qPrintable(lightDebugShader.fileName()));
//		return;
//	}
//	data = lightDebugShader.readAll();
//	path = h3dutGetResourcePath(H3DResTypes::Shader);
//	path += "DebugLightCone.shader.xml";
//	h3dLoadResource(qPrintable(path), data.constData(), data.size());
//	// Add Light Material
//	QString lightMaterialName("DebugLightCone%1.material.xml");
//	lightMaterialName = lightMaterialName.arg(m_hordeID);
//	m_debugResourceMat = h3dAddResource(H3DResTypes::Material, qPrintable(lightMaterialName), 0);	
//	data = "<Material><Shader name=\"DebugLightCone.shader.xml\"/> <Uniform name=\"color\" a=\"";
//	data += QString::number(Horde3D::getLightParam(m_hordeID, LightNodeParams::Col_R));
//	data += "\" b=\"";
//	data += QString::number(Horde3D::getLightParam(m_hordeID, LightNodeParams::Col_G));
//	data +="\" c=\"";
//	data += QString::number(Horde3D::getLightParam(m_hordeID, LightNodeParams::Col_B));
//	data +="\" /></Material>";
//	path = h3dutGetResourcePath(H3DResTypes::Material);
//	path += qPrintable(lightMaterialName);
//	h3dLoadResource(qPrintable(path), data.constData(), data.size());
//	// Add Light Model 
//	QString lightSceneName("DebugLightCone%1.scene.xml");
//	lightSceneName = lightSceneName.arg(m_hordeID);
//	m_debugID = h3dAddResource(H3DResTypes::SceneGraph, qPrintable(lightSceneName), 0);
//	QFile lightDebugModel(":/Models/Resources/DebugLightCone.scene.xml");	
//	if (!lightDebugModel.open(QIODevice::ReadOnly))
//	{
//		qWarning("Error opening resource: %s", qPrintable(lightDebugModel.fileName()));
//		return;
//	}
//	data = lightDebugModel.readAll();
//	data.replace("DebugLightCone.material.xml", qPrintable(lightMaterialName));
//	path = h3dutGetResourcePath(H3DResTypes::SceneGraph);
//	path += lightSceneName;
//	h3dLoadResource(qPrintable(path), data.constData(), data.size());
//	h3dAddNodes(m_hordeID, m_debugID);
//}

//void QLightNode::removeDebugRepresentation()
//{	
//	if (m_debugResourceGeo != 0)
//	{
//		h3dRemoveResource(m_debugResourceGeo);
//		m_debugResourceGeo = 0;
//	}
//	if (m_debugResourceMat != 0)
//	{
//		h3dRemoveResource(m_debugResourceMat);
//		m_debugResourceMat = 0;
//	}
//}

void QLightNode::activate()
{	
    H3DNode cam = static_cast<HordeSceneEditor*>(qApp->property("SceneEditorInstance").value<void*>())->glContext()->activeCam();
	const float* absMat = 0;	
	h3dGetNodeTransMats(h3dGetNodeParent(cam), 0, &absMat);
	if( !absMat ) return;
	const float* lightAbsMat = 0;
	h3dGetNodeTransMats(m_hordeID, 0, &lightAbsMat);
	if( !lightAbsMat) return;
	
	// Don't apply scale to camera
	QMatrix4f l(lightAbsMat);
	QVec3f s = l.getScale();
	l.x[0] /= s.X; l.x[1] /= s.Y; l.x[2] /= s.Z; 
	l.x[4] /= s.X; l.x[5] /= s.Y; l.x[6] /= s.Z; 
	l.x[8] /= s.X; l.x[9] /= s.Y; l.x[10] /= s.Z; 	
	QMatrix4f m = ( QMatrix4f( absMat ).inverted() * l );
	
	h3dSetNodeTransMat(cam, m.x);
}

float QLightNode::fov() const
{
	return m_xmlNode.attribute("fov").toFloat();
}

void QLightNode::setFov(float value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("fov", value);
		h3dSetNodeParamF(m_hordeID, H3DLight::FovF, 0, value);
	}
	else if (value != fov())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Field of View"), this, "Field_Of_View", value, LightFoVID));

}

float QLightNode::shadowMapBias() const
{
	return m_xmlNode.attribute("shadowMapBias").toFloat();
}

void QLightNode::setShadowMapBias(float value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("shadowMapBias", value);
		h3dSetNodeParamF(m_hordeID, H3DLight::ShadowMapBiasF, 0, value);
	}
	else if (value != shadowMapBias())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Shadowmap Bias"), this, "Shadow_Map_Bias", value, LightShadowMapBiasID));

}

QString QLightNode::shadowContext() const
{
	return m_xmlNode.attribute("shadowContext");
}
void QLightNode::setShadowContext(const QString& value)
{
	m_xmlNode.setAttribute("shadowContext", value);	
	h3dSetNodeParamStr( m_hordeID, H3DLight::ShadowContextStr, qPrintable( value ) );
}

QString QLightNode::lightContext() const
{
	return m_xmlNode.attribute("lightingContext");
}

void QLightNode::setLightContext(const QString& value)
{
	m_xmlNode.setAttribute("lightingContext", value);
	h3dSetNodeParamStr( m_hordeID, H3DLight::LightingContextStr, qPrintable( value ) );
}

QColor QLightNode::color() const
{
	QColor color;
	color.setBlueF(m_xmlNode.attribute("col_B").toFloat());
	color.setRedF(m_xmlNode.attribute("col_R").toFloat());
	color.setGreenF(m_xmlNode.attribute("col_G").toFloat());
	return color;
}
void QLightNode::setColor(QColor value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("col_B", value.blueF());
		m_xmlNode.setAttribute("col_R", value.redF());
		m_xmlNode.setAttribute("col_G", value.greenF());
		h3dSetNodeParamF(m_hordeID, H3DLight::ColorF3, 2, value.blueF());
		h3dSetNodeParamF(m_hordeID, H3DLight::ColorF3, 1, value.greenF());
		h3dSetNodeParamF(m_hordeID, H3DLight::ColorF3, 0, value.redF());
	}
	else if (value != color())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Color"), this, "Color", value, LightColorID));
}

float QLightNode::radius() const
{
	return m_xmlNode.attribute("radius").toFloat();
}
void QLightNode::setRadius(float value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("radius", value);
		h3dSetNodeParamF(m_hordeID, H3DLight::RadiusF, 0, value);
	}
	else if (value != radius())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Radius"), this, "Radius", value, LightRadiusID));
}

float QLightNode::intensity() const
{
	return m_xmlNode.attribute( "colMult" ).toFloat();
}

void QLightNode::setIntensity( float value )
{
	if ( signalsBlocked() )
	{
		m_xmlNode.setAttribute( "colMult", value );
		h3dSetNodeParamF( m_hordeID, H3DLight::ColorMultiplierF, 0, value );
	}
	else if ( value != radius() )
		m_model->undoStack()->push( new QXmlNodePropertyCommand( tr( "Set Intensity" ), this, "Intensity", value, LightIntensityID ) );
}

int QLightNode::shadowMapCount() const
{
	return m_xmlNode.attribute("shadowMapCount", "3").toInt();
}

void QLightNode::setShadowMapCount(int value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("shadowMapCount", value);
		h3dSetNodeParamI(m_hordeID, H3DLight::ShadowMapCountI, value);
	}
	else if (value != shadowMapCount())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Shadow Map Count"), this, "Shadow_Map_Count", value, LightShadowMapCountID));
}

float QLightNode::lambda() const
{
	return m_xmlNode.attribute("shadowSplitLambda", "0.95").toFloat();
}

void QLightNode::setLambda(float value)
{
	if (signalsBlocked())
	{
		m_xmlNode.setAttribute("shadowSplitLambda", value);
		h3dSetNodeParamF(m_hordeID, H3DLight::ShadowSplitLambdaF, 0, value);
	}
	else if (value != lambda())
		m_model->undoStack()->push(new QXmlNodePropertyCommand(tr("Set Shadow Map Lambda"), this, "Shadow_Map_Lambda", value, LightShadowMapLambdaID));
}

Material QLightNode::material() const
{
	return m_xmlNode.attribute("material");
}

void QLightNode::setMaterial(const Material& material)
{
	if (signalsBlocked())
	{
		if (m_resourceID != 0)
		{
			h3dRemoveResource(m_resourceID);
			m_resourceID = 0;
		}
		if ( material.FileName.isEmpty() )
			m_xmlNode.removeAttribute("material");
		else
		{
			m_xmlNode.setAttribute("material", material.FileName);
			m_resourceID = h3dAddResource( H3DResTypes::Material, qPrintable(material.FileName), 0 );
			h3dutLoadResourcesFromDisk(".");
		}
		h3dSetNodeParamI(m_hordeID, H3DLight::MatResI, m_resourceID);
	}
	else if (material != QLightNode::material())
		m_model->undoStack()->push(new QXmlNodePropertyCommand("Set Material", this, "Material", QVariant::fromValue(material), LightMaterialID));
}



