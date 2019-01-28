// *************************************************************************************************
//
// Horde3D Terrain Extension
// --------------------------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz, Volker Wiendl and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _Horde3DTerrain_extension_H_
#define _Horde3DTerrain_extension_H_

#include "egPrerequisites.h"
#include "egExtensions.h"


namespace Horde3DTerrain {

using namespace Horde3D;


class ExtTerrain: public IExtension
{
public:
	virtual const char *getName() const { return "Terrain"; }
	virtual bool init();
	virtual void release();
};

}

#endif	// _Horde3DTerrain_extension_H_
