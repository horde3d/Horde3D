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

#include "CustomTypes.h"

#include "Vec3fProperty.h"
#include "MaterialProperty.h"
#include "TextureProperty.h"
#include "FrustumProperty.h"
#include "PipelineProperty.h"
#include "EffectProperty.h"
#include <QPropertyEditor/Property.h>

namespace CustomTypes
{
	static int vec3fType = 0;
	static int matrix4fType = 0;
	static int materialType = 0;
	static int effectType = 0;
	static int textureType = 0;
	static int frustumType = 0;
	static int shaderType = 0;
	static int pipelineType = 0;

	void registerTypes()
	{
		static bool registered = false;
		if (!registered)
		{
			vec3fType = qRegisterMetaType<QVec3f>("QVec3f");
			matrix4fType = qRegisterMetaType<QMatrix4f>("QMatrix4f");
			materialType = qRegisterMetaType<Material>("Material");
			effectType = qRegisterMetaType<Effect>("Effect");
			textureType = qRegisterMetaType<Texture>("Texture");
			frustumType = qRegisterMetaType<Frustum>("Frustum");
			shaderType = qRegisterMetaType<Shader>("Shader");
			pipelineType = qRegisterMetaType<Pipeline>("Pipeline");
			registered = true;
		}
	}

	Property* createCustomProperty(const QString& name, QObject* propertyObject, Property* parent)
	{ 
		int userType = propertyObject->property(qPrintable(name)).userType();
		if ( userType == vec3fType )
			return new Vec3fProperty(name, propertyObject, parent);
		else if ( userType == materialType )
			return new MaterialProperty(name, propertyObject, parent);
		else if ( userType == textureType )
			return new TextureProperty(name, propertyObject, parent);
		else if ( userType == frustumType )
			return new FrustumProperty(name, propertyObject, parent);
		else if ( userType == pipelineType )
			return new PipelineProperty(name, propertyObject, parent);
		else if ( userType == effectType )
			return new EffectProperty(name, propertyObject, parent);
		else
			return 0;
	}
}

double roundIt(double x, double n)
{ 
    x = floor( x * pow(10.0, n) + 0.5) / pow(10.0, n);
    return x;
}
