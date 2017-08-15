// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2017 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************
#pragma once

#include "egPrerequisites.h"
#include "egExtensions.h"

namespace Horde3DOverlays {

	using namespace Horde3D;

	class ExtOverlays : public IExtension
	{
	public:
		virtual const char *getName() const { return "Overlays"; }
		virtual bool init();
		virtual void release();
	};

}

