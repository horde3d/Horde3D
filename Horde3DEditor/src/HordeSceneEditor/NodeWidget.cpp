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

#include "NodeWidget.h"
#include "CustomTypes.h"
#include "QMeshNode.h"
#include "QLightNode.h"
#include "QEmitterNode.h"

#include <QXmlTree/QXmlTreeNode.h>
#include <QLayout>
#include <Horde3D.h>

NodeWidget::NodeWidget(QWidget* parent /*= 0*/) : QPropertyEditorWidget(parent), m_currentNode(0)
{
	CustomTypes::registerTypes();
	registerCustomPropertyCB(CustomTypes::createCustomProperty);
}


NodeWidget::~NodeWidget()
{
}

void NodeWidget::setCurrentNode(QXmlTreeNode* node)
{
	// Set Only if this not already the current object
	if (node != m_currentNode)	
	{	
		if ( m_currentNode && qobject_cast<QSceneNode*>(m_currentNode)->supportsMaterials() )
		{
			disconnect( m_currentNode, SIGNAL( materialChanged(const QString&) ), this, SIGNAL( materialChanged(const QString&) ) );
		}
		setObject(node);
		if( node && qobject_cast<QSceneNode*>( node )->supportsMaterials() )
		{
			connect( node, SIGNAL( materialChanged(const QString&) ), this, SIGNAL( materialChanged(const QString&) ) );
		}
		emit materialChanged(node ? node->xmlNode().attribute("material") : QString());		
	}
	else // Update the property view 
	{
		if (node && qobject_cast<QSceneNode*>(node)->supportsMaterials())
			emit materialChanged(node ? node->xmlNode().attribute("material") : QString());

		updateObject(node);
	}

	// If this is a scene node, save the current transformation to handle manipulations of the transformation
	if( node && node->property("Position").isValid() )
	{
		QVec3f pos, rot, scale;
		QMatrix4f relTrans = node->property("__RelativeTransformation").value<QMatrix4f>();
		relTrans.decompose(pos, rot, scale); rot.toDeg();
		m_tx = pos.X; m_ty = pos.Y; m_tz = pos.Z;
		m_rx = rot.X * 180.0f / 3.14159265358979f;	
		m_ry = rot.Y * 180.0f / 3.14159265358979f;	
		m_rz = rot.Z * 180.0f / 3.14159265358979f;
		m_sx = scale.X; m_sy = scale.Y; m_sz = scale.Z;		
	}

	m_currentNode = node;
}

void NodeWidget::moveObject(const float x, const float y, const float z)
{
	if( !m_currentNode || !m_currentNode->property("Position").isValid() ) return;

	const int hordeID = m_currentNode->property("ID").toInt();

	if (x != x && y != y && z != z) // reset (NAN is not equal to itself)
	{		
		QVec3f pos(m_currentNode->property("Position").value<QVec3f>());
		QVec3f rot(m_currentNode->property("Rotation").value<QVec3f>());
		QVec3f scale(m_currentNode->property("Scale").value<QVec3f>());
		m_currentNode->setProperty("__RelativeTransformation", 
			QVariant::fromValue(
				QMatrix4f::TransMat( pos ) * 
				QMatrix4f::RotMat(rot.toRad() ) * 
				QMatrix4f::ScaleMat( scale ) ) );
		m_tx = pos.X; m_ty = pos.Y; m_tz = pos.Z;
	}
	else if (x == 0 && y == 0 && z == 0) // apply it to the xml data if the movement is zero in every direction (acknowledge command)
	{
		m_currentNode->setProperty("Position", QVariant::fromValue(QVec3f(m_tx, m_ty, m_tz)));
		updateObject( m_currentNode );
	}
	else
	{
		const float* mat = 0;					
		float transX = x, transY = y, transZ = z;		
		H3DNode parent = 0;
		if (hordeID != 0 && ( parent = h3dGetNodeParent( hordeID ) ) != 0 )
		{
			h3dGetNodeTransMats(parent, 0, &mat);
			QMatrix4f parentMat(mat);
			QVec3f scale = parentMat.getScale();
			// Transform translation from world to local coordinate system ignoring scale from parent matrix
			transX = mat[0] * x / scale.X   +   mat[1] * y / scale.Y   +   mat[2]  * z / scale.Z   /*+   mat[12]*/;
			transY = mat[4] * x / scale.X   +   mat[5] * y / scale.Y   +   mat[6]  * z / scale.Z   /*+   mat[13]*/;
			transZ = mat[8] * x / scale.X   +   mat[9] * y / scale.Y   +   mat[10] * z / scale.Z   /*+   mat[14]*/;
			// Remove scale to avoid influence of parent matrix in Horde3D
			transX /= scale.X;
			transY /= scale.Y;
			transZ /= scale.Z;
		}
		QVec3f position(m_currentNode->property("Position").value<QVec3f>());
		// Translate the object 
		m_tx = position.X + transX;
		m_ty = position.Y + transY;
		m_tz = position.Z + transZ;
		m_currentNode->setProperty("__RelativeTransformation", 
			QVariant::fromValue( 			  
			  QMatrix4f::TransMat( m_tx, m_ty, m_tz ) *
			  QMatrix4f::RotMat( QVec3f(m_rx, m_ry, m_rz).toRad() ) * 
			  QMatrix4f::ScaleMat( m_sx, m_sy, m_sz) ));
	}	
}

void NodeWidget::rotateObject(const float rx, const float ry, const float rz)
{	
	if( !m_currentNode || !m_currentNode->property("Rotation").isValid() ) return;

        // const int hordeID = m_currentNode->property("ID").toInt();
	if (rx != rx && ry != ry && rz != rz) // reset
	{
		QVec3f pos(m_currentNode->property("Position").value<QVec3f>());
		QVec3f rot(m_currentNode->property("Rotation").value<QVec3f>());
		QVec3f scale(m_currentNode->property("Scale").value<QVec3f>());
		m_currentNode->setProperty("__RelativeTransformation", 
			QVariant::fromValue(
				QMatrix4f::TransMat( pos ) *
				QMatrix4f::RotMat(rot.toRad() ) * 
				QMatrix4f::ScaleMat( scale ) ) );
		m_rx = rot.X; m_ry = rot.Y; m_rz = rot.Z;
	}	
	else if (rx == 0 && ry == 0 && rz == 0) // apply it to the xml data if the movement is zero in every direction (acknowledge command)
	{
		m_currentNode->setProperty("Rotation", QVariant::fromValue( QVec3f( m_rx, m_ry, m_rz ) ));
		m_rx = m_ry = m_rz = 0;
		updateObject( m_currentNode );
	}
	else // Update transformation settings without applying it to the xml data		
	{		
		QVec3f rot(m_currentNode->property("Rotation").value<QVec3f>());
		m_rx = rot.X + rx;
		m_ry = rot.Y + ry;
		m_rz = rot.Z + rz;
		m_currentNode->setProperty("__RelativeTransformation", 
			QVariant::fromValue( 
			  QMatrix4f::TransMat( m_tx, m_ty, m_tz ) *
			  QMatrix4f::RotMat( QVec3f(m_rx, m_ry, m_rz).toRad() ) * 
			  QMatrix4f::ScaleMat( m_sx, m_sy, m_sz ) ) );
	}
}

void NodeWidget::scaleObject(const float sx, const float sy, const float sz)
{	
	if( !m_currentNode || !m_currentNode->property("Scale").isValid() ) return;

        // const int hordeID = m_currentNode->property("ID").toInt();
	if (sx != sx && sy != sy && sz != sz) // reset
	{
		QVec3f pos(m_currentNode->property("Position").value<QVec3f>());
		QVec3f rot(m_currentNode->property("Rotation").value<QVec3f>());
		QVec3f scale(m_currentNode->property("Scale").value<QVec3f>());
		m_currentNode->setProperty("__RelativeTransformation", 
			QVariant::fromValue(
				QMatrix4f::TransMat( pos ) * 
				QMatrix4f::RotMat(rot.toRad() ) * 
				QMatrix4f::ScaleMat( scale ) ) );
		m_sx = scale.X; m_sy = scale.Y; m_sz = scale.Z;
	}
	else if (sx == 0 && sy == 0 && sz == 0) // apply it to the xml data if the movement is zero in every direction (acknowledge command)
	{
		m_currentNode->setProperty("Scale", QVariant::fromValue(QVec3f(m_sx, m_sy, m_sz)));
		updateObject( m_currentNode );
	}
	else // Update transformation settings without applying it to the xml data		
	{		
		QVec3f scale(m_currentNode->property("Scale").value<QVec3f>());
		m_sx = scale.X * sx;
		m_sy = scale.Y * sy;
		m_sz = scale.Z * sz;
		m_currentNode->setProperty("__RelativeTransformation", 
			QVariant::fromValue( 
			  QMatrix4f::TransMat( m_tx, m_ty, m_tz ) * 
			  QMatrix4f::RotMat( QVec3f(m_rx, m_ry, m_rz).toRad() ) * 
			  QMatrix4f::ScaleMat( m_sx, m_sy, m_sz ) ) );
	}
}

void NodeWidget::transformObject(const QMatrix4f m)
{
	if( !m_currentNode ) return;

	m_currentNode->setProperty("__RelativeTransformation", QVariant::fromValue( m ) );

	QVec3f ts = m.getTranslation();
	QVec3f rt = m.getRotation();
	QVec3f sc = m.getScale();

	rt *= 180.0f / 3.1415926f;

	m_tx = ts.X; m_ty = ts.Y; m_tz = ts.Z;
	// m_rx = m_rx + rt.X; m_ry = m_ry + rt.Y; m_rz = m_rz + rt.Z;
	m_rx = rt.X; m_ry = rt.Y; m_rz = rt.Z;
	m_sx = sc.X; m_sy = sc.Y; m_sz = sc.Z;
}
