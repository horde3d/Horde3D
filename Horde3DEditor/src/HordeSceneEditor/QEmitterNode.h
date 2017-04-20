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
#ifndef QEMITTERNODE_H_
#define QEMITTERNODE_H_

#include "QSceneNode.h"

#include "CustomTypes.h"

class QEmitterNode : public QSceneNode
{
	Q_OBJECT

	Q_CLASSINFO("QEmitterNode", "Emitter specific");

	Q_PROPERTY(Material Material READ material WRITE setMaterial DESIGNABLE true USER true)
	Q_PROPERTY(Effect Effect READ effect WRITE setEffect DESIGNABLE true USER true)
	Q_PROPERTY(unsigned int Max_Count READ maxCount WRITE setMaxCount DESIGNABLE true USER true)
	Q_PROPERTY(int Respawn_Count READ respawnCount WRITE setRespawnCount DESIGNABLE true USER true)
	Q_CLASSINFO("Respawn_Count", "minimum=-1;");
	Q_PROPERTY(QVec3f Force READ force WRITE setForce DESIGNABLE true USER true)
	Q_PROPERTY(int Emission_Rate READ emissionRate WRITE setEmissionRate DESIGNABLE true USER true)
	Q_PROPERTY(float Spread_Angle READ spreadAngle WRITE setSpreadAngle DESIGNABLE true USER true)
	Q_CLASSINFO("Spread_Angle", "suffix=�;");
	Q_PROPERTY(float Delay READ delay WRITE setDelay DESIGNABLE true USER true)
	Q_CLASSINFO("Delay", "suffix=s;");


public:
	static QSceneNode* loadNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parent);

	QEmitterNode(const QDomElement& xmlNode, int row, SceneTreeModel* model, QSceneNode* parentNode);
	virtual ~QEmitterNode();

	void addRepresentation();

	Material material() const;
	void setMaterial(const Material& material);

	Effect effect() const;
	void setEffect(const Effect& effect);

	unsigned int maxCount() const;
	void setMaxCount(unsigned int value);

	int respawnCount() const;
	void setRespawnCount(int value);

	QVec3f force() const;
	void setForce(const QVec3f& value);

	int emissionRate() const;
	void setEmissionRate(int value);

	float spreadAngle() const;
	void setSpreadAngle(float value);

	float delay() const;
	void setDelay(float value);

protected:
	void timerEvent(QTimerEvent* event);
	
private:
	unsigned int	m_effectResource;
	unsigned int	m_matResource;	
};
#endif
