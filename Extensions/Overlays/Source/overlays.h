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

#include "egResource.h"
#include "egMaterial.h"
#include "egPipeline.h"

#include <vector>

namespace Horde3DOverlays {

struct OverlayBatch
{
	Horde3D::PMaterialResource		materialRes;
	uint32							firstVert, vertCount;
	float							colRGBA[ 4 ];
	int								flags;

	OverlayBatch() {}

	OverlayBatch( uint32 firstVert, uint32 vertCount, float *col, Horde3D::MaterialResource *materialRes, int flags ) :
		materialRes( materialRes ), firstVert( firstVert ), vertCount( vertCount ), flags( flags )
	{
		colRGBA[ 0 ] = col[ 0 ]; colRGBA[ 1 ] = col[ 1 ]; colRGBA[ 2 ] = col[ 2 ]; colRGBA[ 3 ] = col[ 3 ];
	}
};

struct OverlayVert
{
	float  x, y;  // Position
	float  u, v;  // Texture coordinates
};

struct CachedUniformLocation
{
	Horde3D::MaterialResource	*material;
	int							uniformLocation;

	CachedUniformLocation() {}

	CachedUniformLocation( Horde3D::MaterialResource *mat, int loc ) : material( mat ), uniformLocation( loc )
	{

	}
};

class OverlayRenderer
{
public:
	
	static const char *parsePipelineCommandFunc( const char *commandName, void *xmlNodeParams, Horde3D::PipelineCommand &cmd );
	static void executePipelineCommandFunc( const Horde3D::PipelineCommand *commandParams );

	static bool init();
	static void release();

	static void showOverlays( const float *verts, uint32 vertCount, float *colRGBA,
							  Horde3D::MaterialResource *matRes, int flags );

	static void clearOverlays();

	static void drawOverlays( const std::string &shaderContext );
	static int getInternalUniformLocation( MaterialResource *mat );
private:

	static std::vector< CachedUniformLocation >	_cachedLocations;
	static std::vector< OverlayBatch >			_overlayBatches;
	static OverlayVert							*_overlayVerts;
	static uint32								_overlayGeo;
	static uint32								_overlayVB;

	static int									_uni_olayColor;
	static int									_vlOverlay;
};

} // namespace