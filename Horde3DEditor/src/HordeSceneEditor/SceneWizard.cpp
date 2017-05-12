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

#include "SceneWizard.h"
#include "SceneFilePage.h"
#include "PlugInPage.h"

#include "QCameraNodePage.h"
#include "QGroupNodePage.h"
#include "QReferenceNodePage.h"
#include "QLightNodePage.h"
#include "QEmitterNodePage.h"
#include "QCameraNodePage.h"

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QtCore/QDir>
#include <QtXml/qdom.h>
#include <QMessageBox>
#include <QtCore/QTextStream>

#include <math.h>

#include "SceneFile.h"

#include <Horde3D.h>

namespace SceneWizard
{
	SceneFile* createScene(QWidget* parent /*=0*/)
	{		
		SceneFile* sceneFile = 0;
		QWizard* wizard = new QWizard(parent);
		wizard->addPage(new SceneFilePage(wizard));
		wizard->addPage(new QCameraNodePage(wizard));
		wizard->addPage(new QLightNodePage(wizard));
		wizard->addPage(new PlugInPage(wizard));
		if (wizard->exec() == QDialog::Accepted)
		{	
			// Create new scene file
			sceneFile = new SceneFile();			
			// Set filename
			QFileInfo sceneFileInfo(QDir(wizard->field("scenepath").toString()), wizard->field("scenefile").toString());
			sceneFile->setSceneFileName(sceneFileInfo.absoluteFilePath());
			// create scenegraph file
			QDir sceneFileDir = sceneFileInfo.absoluteDir();
			QFileInfo sceneGraphFile(QDir(wizard->field("scenepath").toString()), wizard->field("scenegraphfile").toString() );
			// set the scene graph file within the configuration file
			sceneFile->setSceneGraphFile(sceneFileDir.relativeFilePath(sceneGraphFile.absoluteFilePath()));

			// Create Configuration XML structure
			QDomDocument sceneFileXml("SceneConfiguration");		
			sceneFileXml.appendChild(sceneFileXml.createElement("Configuration"));
			// store pipeline settings
			QDomElement element = sceneFileXml.documentElement().appendChild(sceneFileXml.createElement("Pipeline")).toElement();			
			element.setAttribute("path", QDir::current().relativeFilePath(wizard->field("pipeline").toString()));			
			// store scenegraph settings
			element = sceneFileXml.documentElement().appendChild(sceneFileXml.createElement("SceneGraph")).toElement();
			// save scene graph file relative to the scene file directory 
			element.setAttribute("path", sceneFileDir.relativeFilePath( sceneFile->sceneGraphFile() ) );			
			sceneFile->setSceneFileDom(sceneFileXml);
			// set standard light parameters
			element = sceneFileXml.createElement("LightParameters");
			if ( !wizard->field("material").toString().isEmpty() ) // Only create material attribute if necessary
				element.setAttribute("material", wizard->field("material").toString());
			element.setAttribute("lightingcontext", wizard->field("lightingcontext").toString());
			element.setAttribute("shadowcontext", wizard->field("shadowcontext").toString());
			sceneFileXml.documentElement().appendChild(element);
			// set AttachmentPlugin
			if (wizard->field("plugin").toString().compare("None", Qt::CaseInsensitive)!=0)
			{
				element = sceneFileXml.documentElement().appendChild(sceneFileXml.createElement("AttachmentPlugIn")).toElement();
				element.setAttribute("name", wizard->field("plugin").toString());
			}
			// set extras 
			sceneFileXml.documentElement().appendChild(sceneFileXml.createElement("Extras"));
			sceneFileXml.documentElement().appendChild(sceneFileXml.createElement("ActiveCamera")).toElement().setAttribute("name", wizard->field("cameraname").toString());
			// create basic scene structure
			QDomDocument sceneGraphXml("HordeSceneGraph");
			QDomElement H3DRootNode = sceneGraphXml.createElement("Group");
			H3DRootNode.setAttribute("name", QFileInfo(sceneFile->sceneGraphFile()).baseName());
			QDomElement cameraNode = sceneGraphXml.createElement("Camera");
			cameraNode.setAttribute("name", wizard->field("cameraname").toString());
			cameraNode.setAttribute("pipeline", wizard->field("pipeline").toString());
			float left, right, bottom, top, near, far;
			if (wizard->field("asymFrustum").toBool())
			{				
				left = wizard->field("leftPlane").toDouble();
				right = wizard->field("rightPlane").toDouble();
				bottom = wizard->field("bottomPlane").toDouble();
				top = wizard->field("topPlane").toDouble();
				near = wizard->field("asymNear").toDouble();
				far = wizard->field("asymFar").toDouble();
			}
			else
			{
				near = wizard->field("symNear").toDouble();
				far = wizard->field("symFar").toDouble();
				float ymax = near * tanf( ( wizard->field("camerafov").toDouble() * 3.1415926 / 360.0f ) );
				float xmax = ymax * wizard->field("quo").toDouble() / wizard->field("div").toDouble();
				left = -xmax;
				right = xmax;
				bottom = -ymax;
				top = ymax;
			}
			cameraNode.setAttribute("leftPlane", left);
			cameraNode.setAttribute("rightPlane", right);
			cameraNode.setAttribute("bottomPlane", bottom);
			cameraNode.setAttribute("topPlane", top);
			cameraNode.setAttribute("nearPlane", near);
			cameraNode.setAttribute("farPlane", far);
			cameraNode.setAttribute("tx", "0.0");
			cameraNode.setAttribute("ty", "7.5");
			cameraNode.setAttribute("tz", "12.0");
			cameraNode.setAttribute("rx", "-20");
			H3DRootNode.appendChild(cameraNode);
			QDomElement lightNode = sceneGraphXml.createElement("Light");			
			lightNode.setAttribute("name", wizard->field("lightname").toString());
			lightNode.setAttribute("radius", wizard->field("radius").toDouble());
			lightNode.setAttribute("fov", wizard->field("lightfov").toDouble());
			lightNode.setAttribute("tx", "-3.25");
			lightNode.setAttribute("ty", "4");
			lightNode.setAttribute("tz", "6.0");
			lightNode.setAttribute("rx", "-27");
			lightNode.setAttribute("ry", "-22");
			lightNode.setAttribute("col_R", 1.0f);
			lightNode.setAttribute("col_G", 1.0f);
			lightNode.setAttribute("col_B", 1.0f);
			lightNode.setAttribute("shadowMapBias", wizard->field("shadowmapbias").toDouble());
			lightNode.setAttribute("shadowSplitLambda", wizard->field("lambda").toDouble());
			lightNode.setAttribute("shadowMapCount", wizard->field("slices").toInt());
			lightNode.setAttribute("shadowContext", wizard->field("shadowcontext").toString());
			lightNode.setAttribute("lightingContext", wizard->field("lightingcontext").toString());
			if ( !wizard->field("material").toString().isEmpty() ) // Only create material attribute if necessary	
				lightNode.setAttribute("material", wizard->field("material").toString());
			H3DRootNode.appendChild(lightNode);
			sceneGraphXml.appendChild(H3DRootNode);
			QFile tempFile(sceneFile->sceneGraphFile());
			if (!tempFile.open(QIODevice::WriteOnly))
				QMessageBox::warning(0, QString("Error"), QString("File couldn't be opened for writing: ")+sceneFile->sceneGraphFile());
			else
			{
				QTextStream graphStream(&tempFile);
				sceneGraphXml.save(graphStream, 4);
				tempFile.close();
			}
			// save the new created scene
			if (!sceneFile->save())
				QMessageBox::warning(0, QObject::tr("Error"), QObject::tr("Error saving file: %1").arg(sceneFile->absoluteSceneFilePath()));
		}		
		delete wizard;
		return sceneFile;
	}

	QDomElement createReference(QWidget* parent)
	{
		QDomDocument referenceNode;
		QWizard* wizard = new QWizard(parent);
		wizard->addPage(new QReferenceNodePage(wizard));
		if (wizard->exec() == QDialog::Accepted)
		{						
			referenceNode.setContent( QString( "<Reference/>" ) );
			referenceNode.documentElement().setAttribute("name", wizard->field("name").toString());
			referenceNode.documentElement().setAttribute("sceneGraph", wizard->field("file").toString());
			referenceNode.documentElement().setAttribute("sx", 1.0f);
			referenceNode.documentElement().setAttribute("sy", 1.0f);
			referenceNode.documentElement().setAttribute("sz", 1.0f);
		}
		return referenceNode.documentElement();
	}

	QDomElement createLight(QWidget* parent)
	{
		QDomDocument lightNode;
		QWizard* wizard = new QWizard(parent);
		wizard->addPage(new QLightNodePage(wizard));
		if (wizard->exec() == QDialog::Accepted)
		{			
			lightNode.setContent( QString( "<Light/>" ) );
			lightNode.documentElement().setAttribute("name", wizard->field("lightname").toString());
			lightNode.documentElement().setAttribute("radius", wizard->field("radius").toDouble());
			lightNode.documentElement().setAttribute("fov", wizard->field("lightfov").toDouble());
			lightNode.documentElement().setAttribute("shadowMapBias", wizard->field("shadowmapbias").toDouble());
			lightNode.documentElement().setAttribute("shadowMapEnabled", wizard->field("shadowmap").toBool() ? "true" : "false");
			lightNode.documentElement().setAttribute("col_R", 1.0f);
			lightNode.documentElement().setAttribute("col_G", 1.0f);
			lightNode.documentElement().setAttribute("col_B", 1.0f);
			lightNode.documentElement().setAttribute("shadowSplitLambda", wizard->field("lambda").toDouble());
			lightNode.documentElement().setAttribute("shadowMapCount", wizard->field("slices").toInt());
			lightNode.documentElement().setAttribute("shadowContext", wizard->field("shadowcontext").toString());
			lightNode.documentElement().setAttribute("lightingContext", wizard->field("lightingcontext").toString());
			if ( !wizard->field("material").toString().isEmpty() ) // Only create material attribute if necessary
				lightNode.documentElement().setAttribute("material", wizard->field("material").toString());
		}
		delete wizard;
		return lightNode.documentElement();
	}

	QDomElement createCamera(QWidget* parent)
	{
		QDomDocument cameraNode;
		QWizard* wizard = new QWizard(parent);
		wizard->addPage(new QCameraNodePage(/*QDomElement(), */wizard));
		if (wizard->exec() == QDialog::Accepted)
		{
			cameraNode.setContent( QString( "<Camera/>" ) );
			cameraNode.documentElement().setAttribute("name", wizard->field("cameraname").toString());
			float left, right, bottom, top, near, far;
			if (wizard->field("asymFrustum").toBool())
			{				
				left = wizard->field("leftPlane").toDouble();
				right = wizard->field("rightPlane").toDouble();
				bottom = wizard->field("bottomPlane").toDouble();
				top = wizard->field("topPlane").toDouble();
				near = wizard->field("asymNear").toDouble();
				far = wizard->field("asymFar").toDouble();
			}
			else
			{
				near = wizard->field("symNear").toDouble();
				far = wizard->field("symFar").toDouble();
				float ymax = near * tanf( ( wizard->field("camerafov").toDouble() * 3.1415926 / 360.0f ) );
				float xmax = ymax * wizard->field("quo").toDouble() / wizard->field("div").toDouble();
				left = -xmax;
				right = xmax;
				bottom = -ymax;
				top = ymax;
			}
			cameraNode.documentElement().setAttribute("leftPlane", left);
			cameraNode.documentElement().setAttribute("rightPlane", right);
			cameraNode.documentElement().setAttribute("bottomPlane", bottom);
			cameraNode.documentElement().setAttribute("topPlane", top);
			cameraNode.documentElement().setAttribute("nearPlane", near);
			cameraNode.documentElement().setAttribute("farPlane", far);
			cameraNode.documentElement().setAttribute("pipeline", wizard->field("pipeline").toString());
		}
		delete wizard;
		return cameraNode.documentElement();
	}

	QDomElement createGroup(QWidget* parent)
	{
		QDomDocument groupNode;
		QWizard* wizard = new QWizard(parent);
		wizard->addPage(new QGroupNodePage(wizard));
		if (wizard->exec() == QDialog::Accepted)
		{
			groupNode.setContent( QString( "<Group/>" ) );
			groupNode.documentElement().setAttribute("name", wizard->field("name").toString());
		}
		delete wizard;
		return groupNode.documentElement();
	}

	QDomElement createEmitter(QWidget* parent)
	{
		QDomDocument emitterNode;
		QWizard* wizard = new QWizard(parent);
		wizard->addPage(new QEmitterNodePage(wizard));
		if (wizard->exec() == QDialog::Accepted)
		{			
			emitterNode.setContent( QString( "<Emitter/>") );
			emitterNode.documentElement().setAttribute("name", wizard->field("name").toString());
			emitterNode.documentElement().setAttribute("particleEffect", wizard->field("particleEffect").toString());
			emitterNode.documentElement().setAttribute("material", wizard->field("material").toString());
			emitterNode.documentElement().setAttribute("maxCount", wizard->field("maxCount").toDouble());
			emitterNode.documentElement().setAttribute("respawnCount", wizard->field("respawnCount").toDouble());
			emitterNode.documentElement().setAttribute("emissionRate", wizard->field("emissionRate").toUInt());
		}
		delete wizard;
		return emitterNode.documentElement();
	}
}


