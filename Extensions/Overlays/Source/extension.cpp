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


	DLLEXP void h3dShowText( const char *text, float x, float y, float size, float colR,
		float colG, float colB, ResHandle fontMaterialRes )
	{
		Resource *resObj = Modules::resMan().resolveResHandle( fontMaterialRes );
		APIFUNC_VALIDATE_RES_TYPE( resObj, ResourceTypes::Material, "h3dShowText", APIFUNC_RET_VOID );

		OverlayRenderer::showText( text, x, y, size, colR, colG, colB, ( Horde3D::MaterialResource * ) resObj );
	}


	DLLEXP void h3dShowInfoBox( float x, float y, float width, const char *title,
		int numRows, const char **column1, const char **column2,
		ResHandle fontMaterialRes, ResHandle panelMaterialRes )
	{
		Resource *fontResObj = Modules::resMan().resolveResHandle( fontMaterialRes );
		APIFUNC_VALIDATE_RES_TYPE( fontResObj, ResourceTypes::Material, "h3dShowInfoBox", APIFUNC_RET_VOID );

		Resource *panelResObj = Modules::resMan().resolveResHandle( panelMaterialRes );
		APIFUNC_VALIDATE_RES_TYPE( panelResObj, ResourceTypes::Material, "h3dShowInfoBox", APIFUNC_RET_VOID );

		OverlayRenderer::beginInfoBox( x, y, width, numRows, title, 
									 ( Horde3D::MaterialResource * ) fontResObj, ( Horde3D::MaterialResource * ) panelResObj );
		for ( int i = 0; i < numRows; ++i )
			OverlayRenderer::addInfoBoxRow( column1 ? column1[ i ] : 0, column2 ? column2[ i ] : 0 );
	}


	DLLEXP void h3dShowFrameStats( ResHandle fontMaterialRes, ResHandle panelMaterialRes, int mode )
	{
		Resource *fontResObj = Modules::resMan().resolveResHandle( fontMaterialRes );
		APIFUNC_VALIDATE_RES_TYPE( fontResObj, ResourceTypes::Material, "h3dShowFrameStats", APIFUNC_RET_VOID );

		Resource *panelResObj = Modules::resMan().resolveResHandle( panelMaterialRes );
		APIFUNC_VALIDATE_RES_TYPE( panelResObj, ResourceTypes::Material, "h3dShowFrameStats", APIFUNC_RET_VOID );

		OverlayRenderer::showFrameStats( ( Horde3D::MaterialResource * ) fontResObj, ( Horde3D::MaterialResource * ) panelResObj, mode );
	}


}  // namespace
