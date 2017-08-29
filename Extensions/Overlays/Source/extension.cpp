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

#include "utPlatform.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include "extension.h"
#include "overlays.h"
#include "egResource.h"

#include "utDebug.h"


// Internal extension interface
namespace Horde3DOverlays {

	using namespace Horde3D;

	bool ExtOverlays::init()
	{
		return OverlayRenderer::init();
	}

	void ExtOverlays::release()
	{
		OverlayRenderer::release();
	}

}  // namespace


   // Public C API
namespace Horde3DOverlays {

	DLLEXP void h3dShowOverlays( const float *verts, int vertCount, float colR, float colG,
		float colB, float colA, uint32 materialRes, int flags )
	{
		Resource *resObj = Modules::resMan().resolveResHandle( materialRes );
		APIFUNC_VALIDATE_RES_TYPE( resObj, ResourceTypes::Material, "h3dShowOverlays", APIFUNC_RET_VOID );

		float rgba[ 4 ] = { colR, colG, colB, colA };
		OverlayRenderer::showOverlays( verts, ( uint32 ) vertCount, rgba, ( MaterialResource * ) resObj, flags );
	}


	DLLEXP void h3dClearOverlays()
	{
		OverlayRenderer::clearOverlays();
	}

}  // namespace
