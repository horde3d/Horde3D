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

/*	Title: Horde3D Overlays Extension */

#pragma once

#include "Horde3D.h"


/*	Topic: Introduction
		Some words about the Overlays Extension.
	
	The Overlays Extension provides Horde3D with ability to render text and images above the rendered image.
	Extension is also used as a demonstration of registering new pipeline commands in the engine.
*/


/* Function: h3dShowOverlays
		Displays overlays on the screen.
	
	Details:
		This function displays one or more overlays with a specified material and color.
		An overlay is a screen-space quad that can be used to render 2D GUI elements. The overlay coordinate
		system has its origin (0, 0) at the top-left corner of the screen and its maximum (aspect, 1)
		at the bottom-right corner. As the x coordinate of the maximum corresponds to the aspect ratio
		of the viewport, the size of overlays can always be the same, even when different screen formats
		(standard 4:3, widescreen 16:9, etc.) are used. Texture coordinates are using a system where the
		coordinates (0, 0) correspond to the lower left corner of the image.
		Overlays are drawn in the order in which they are pushed using this function. Overlays with
		the same state will be batched together, so it can make sense to group overlays that have the
		same material, color and flags in order to achieve best performance.
		Note that the overlays have to be removed manually using the function h3dClearOverlays.
	
	Parameters:
		verts                   - vertex data (x, y, u, v), interpreted as quads
		vertCount               - number of vertices (must be multiple of 4)
		colR, colG, colB, colA  - color (and transparency) of overlays
		materialRes             - material resource used for rendering
		flags                   - overlay flags (reserved for future use)
		
	Returns:
		nothing
*/
H3D_API void h3dShowOverlays( const float *verts, int vertCount, float colR, float colG, float colB,
	                          float colA, H3DRes materialRes, int flags );


/* Function: h3dClearOverlays
		Removes all overlays.
	
	Details:
		This function removes all overlays that were added with h3dShowOverlays.
	
	Parameters:
		none
		
	Returns:
		nothing
*/
H3D_API void h3dClearOverlays();


/* Function: h3dShowText
		Shows text on the screen using a font texture.

	Details:
		This utility function uses overlays to display a text string at a specified position on the screen.
		The font texture of the specified font material has to be a regular 16x16 grid containing all
		ASCII characters in row-major order.

	Parameters:
		text              - text string to be displayed
		x, y              - position of the lower left corner of the first character;
		                    for more details on coordinate system see overlay documentation
		size              - size (scale) factor of the font
		colR, colG, colB  - font color
		fontMaterialRes   - font material resource used for rendering

	Returns:
		nothing
*/
H3D_API void h3dShowText( const char *text, float x, float y, float size,
                          float colR, float colG, float colB, H3DRes fontMaterialRes );

/* Function: h3dShowInfoBox
		Shows a customizable info box on the screen.

	Details:
		This utility function displays an info box with custom text for the current frame on the screen.

	Parameters:
		x, y              - position of the top left corner of the box;
		                    for more details on coordinate system see overlay documentation
		width             - maximum width of info box
		title             - title string of info box
		numRows           - Number of info rows
		column1           - list of strings to print in first column (=numRows)
		column2           - list of strings to print in second column (=numRows)
		panelMaterialRes  - material resource used for drawing info box

	Returns:
		nothing
*/
H3D_API void h3dShowInfoBox( float x, float y, float width, const char *title,
                             int numRows, const char **column1, const char **column2,
                             H3DRes fontMaterialRes, H3DRes panelMaterialRes );

/* Function: h3dutShowFrameStats
		Shows frame statistics on the screen.

	Details:
		This utility function displays an info box with statistics for the current frame on the screen.
		Since the statistic counters are reset after the call, the function should be called exactly once
		per frame to obtain correct values.

	Parameters:
		fontMaterialRes	  - font material resource used for drawing text
		panelMaterialRes  - material resource used for drawing info box
		mode              - display mode, specifying which data is shown (<= MaxStatMode)

	Returns:
		nothing
*/
H3D_API void h3dShowFrameStats( H3DRes fontMaterialRes, H3DRes panelMaterialRes, int mode );
