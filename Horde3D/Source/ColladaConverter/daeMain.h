// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2021 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _daeMain_H_
#define _daeMain_H_

#include "utils.h"
#include "daeLibImages.h"
#include "daeLibEffects.h"
#include "daeLibMaterials.h"
#include "daeLibGeometries.h"
#include "daeLibVisualScenes.h"
#include "daeLibControllers.h"
#include "daeLibAnimations.h"
#include "daeLibNodes.h"

namespace Horde3D {
namespace ColladaConverter {


class ColladaDocument
{
public:
	ColladaDocument();
	bool parseFile( const std::string &fileName );

public:
	DaeLibImages       libImages;
	DaeLibEffects      libEffects;
	DaeLibMaterials    libMaterials;
	DaeLibGeometries   libGeometries;
	DaeLibVisScenes    libVisScenes;
	DaeLibControllers  libControllers;
	DaeLibAnimations   libAnimations;
	DaeLibNodes        libNodes;

	DaeVisualScene     *scene;
	bool               y_up;
};


} // namespace ColladaConverter
} // namespace Horde3D

#endif // _daeMain_H_
