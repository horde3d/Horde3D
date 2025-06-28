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

/*	Title: Horde3D Utility Library */

// About:
// C Language header created from the C++ header



#ifndef H3D_HORDE3DUTILS_C_HEADER
#define H3D_HORDE3DUTILS_C_HEADER

//C++ defines
//from: https://riptutorial.com/c/example/19737/defining-a-bool-type-using-typedef
#if __STDC_VERSION__ > 199900L
typedef enum { false, true } bool;
/* Modern C code might expect these to be macros. */
# ifndef bool
#  define bool bool
# endif
# ifndef true
#  define true true
# endif
# ifndef false
#  define false false
# endif
#else
# include <stdbool.h>
#endif

//C++ defines


#include "Horde3D_C.h"


/*	Topic: Introduction
		Some words about the Utility Library.
	
	Horde3D has a simple core API which offers all the functionality needed to control the engine. The
	engine core is intended to be as generic as possible to make the complete system lightweight and clean.
	Nevertheless, it is sometimes useful to have more specific functions in order to increase productivity.
	For this reason the Utility Library is offered. It has some very handy functions that can help
	to make your life easier.
*/


/* Group: Typedefs and constants */
/*	Constants: Predefined constants
	H3DUTMaxStatMode  - Maximum stat mode number supported in showFrameStats
*/
//const int H3DUTMaxStatMode = 2;
#define H3DUTMaxStatMode 2

/*	Group: General functions */
/*	Function: h3dutFreeMem
		Frees memory allocated by the Utility Library.
	
	Details:
		This utility function frees the memory that was allocated by another function of the Utility Library.
	
	Parameters:
		ptr  - address of a pointer that references to memory allocated by the Utility Library
		
	Returns:
		nothing
*/
H3D_API void h3dutFreeMem( char **ptr );

/*	Function: h3dutDumpMessages
		Writes all messages in the queue to a log file.
	
	Details:
		This utility function pops all messages from the message queue and writes them to a HTML formated
		log file 'Horde3D_Log.html'.
	
	Parameters:
		none
		
	Returns:
		true in case of success, otherwise false
*/
H3D_API bool h3dutDumpMessages();

/*	Group: Resource management */
/* Function: h3dutGetResourcePath
		*Deprecated*
		Returns  the search path of a resource type.
	
	Details:
		This function returns the search path of a specified resource type.

	The function is now marked as deprecated since it is better practice to make all paths
	relative to the content directory.
	
	Parameters:
		type  - type of resource
		
	Returns:
		pointer to the search path string
*/
H3D_API const char *h3dutGetResourcePath( int type );

/* Function: h3dutSetResourcePath
		*Deprecated*
		Sets the search path for a resource type.

	Details:
		This function sets the search path for a specified resource type.
	
	The function is now marked as deprecated since it is better practice to make all paths
	relative to the content directory.
	
	Parameters:
		type  - type of resource
		path  - path where the resources can be found ((back-)slashes at end are removed)
		
	Returns:
		nothing
*/
H3D_API void h3dutSetResourcePath( int type, const char *path );

/* Function: h3dutSetShaderCachePath
		Sets the search path for binary shaders.

	Details:
		This function sets the search path for binary shaders. Allows replacing text-based shaders if binary variant is found
		at the search path.

	Parameters:
		path  - path where the binary shaders can be found ((back-)slashes at end are removed)

	Returns:
		nothing
*/
H3D_API void h3dutSetShaderCachePath( const char *path );

/* Function: h3dutLoadResourcesFromDisk
		Loads previously added resources from a data drive.
	
	Details:
		This utility function loads previously added and still unloaded resources from the specified
		directories on a data drive. Several search paths can be specified using the pipe character (|)
		as separator. All resource names are directly converted to filenames and the function tries to
		find them in the specified directories using the given order of the search paths.
	
	Parameters:
		contentDir  - directories where data is located on the drive ((back-)slashes at end are removed)
		
	Returns:
		false if at least one resource could not be loaded, otherwise true
*/
H3D_API bool h3dutLoadResourcesFromDisk( const char *contentDir );

/* Function: h3dutCreateGeometryRes
		Creates a Geometry resource from specified vertex data.
	
	Details:
		This utility function allocates and initializes a Geometry resource
		with the specified vertex attributes and indices. The optional tangent space
		data (normal, tangent, bitangent) is encoded as int16, where -1.0 maps to
		-32'767 and 1.0f to +32'767.
	
	Parameters:
		name               - unique name of the new Geometry resource 
		numVertices        - number of vertices
		numTriangleIndices - number of vertex indices
		posData            - vertex positions (xyz)
		indexData          - indices defining triangles
		normalData         - normals xyz (optional, can be NULL)
		tangentData        - tangents xyz (optional, can be NULL)
		bitangentData      - bitangents xyz (required if tangents specified, otherwise NULL)
		texData1           - first texture coordinate uv set (optional, can be NULL)
		texData2           - second texture coordinate uv set (optional, can be NULL)
		
	Returns:
		handle to new Geometry resource or 0 in case of failure
*/


H3D_API H3DRes h3dutCreateGeometryRes( const char *name, int numVertices, int numTriangleIndices, 
								   float *posData, unsigned int *indexData, short *normalData,
								   short *tangentData, short *bitangentData, 
								   float *texData1, float *texData2 );

/* Function: h3dutCreateTGAImage
		Creates a TGA image in memory.
	
	Details:
		This utility function allocates memory at the pointer outData and creates a TGA image from the
		specified pixel data. The dimensions of the image have to be specified as well as the bit depth.
		The created TGA-image-data can be used as Texture2D or TexureCube resource in the engine.
		
		*Note: The memory allocated by this routine has to freed manually using the freeMem function.*
	
	Parameters:
		pixels   - pointer to pixel source data in BGR(A) format from which TGA-image is constructed;
		           memory layout: pixel with position (x, y) in image (origin of image is lower left
		           corner) has memory location (y * width + x) * (bpp / 8) in pixels array
		width    - width of source image
		height   - height of source image
		bpp      - color bit depth of source data (valid values: 24, 32)
		outData  - address of a pointer to which the address of the created memory block is written
		outSize  - variable to which to size of the created memory block is written
		
	Returns:
		false if at least one resource could not be loaded, otherwise true
*/
H3D_API bool h3dutCreateTGAImage( const unsigned char *pixels, int width, int height, int bpp,
                                  char **outData, int *outSize );

/*	Group: Utils */
/* Function: h3dutScreenshot
		Writes the content of the backbuffer to a tga file.
	
	Details:
		This function reads back the content of the backbuffer and writes it to a tga file with the
		specified filename and path.
	
	Parameters:
		filename  - filename and path of the output tga file
		
	Returns:
		true if the file could be written, otherwise false
*/
H3D_API bool h3dutScreenshot( const char *filename );


/*	Group: Scene graph */
/* Function: h3dutPickRay
		Calculates the ray originating at the specified camera and window coordinates
	
	Details:
		This utility function takes normalized window coordinates (ranging from 0 to 1 with the
		origin being the bottom left corner of the window) and returns ray origin and direction for the
		given camera. The function is especially useful for selecting objects by clicking
		on them.
	
	Parameters:
		cameraNode  - camera used for picking
		nwx, nwy    - normalized window coordinates
		ox, oy, oz  - calculated ray origin
		dx, dy, dz  - calculated ray direction
		
	Returns:
		nothing
*/
H3D_API void h3dutPickRay( H3DNode cameraNode, float nwx, float nwy, float *ox, float *oy, float *oz,
                           float *dx, float *dy, float *dz );

/* Function: h3dutPickNode
		Returns the scene node which is at the specified window coordinates.
	
	Details:
		This utility function takes normalized window coordinates (ranging from 0 to 1 with the
		origin being the bottom left corner of the window) and returns the scene node which is
		visible at that location. The function is especially useful for selecting objects by clicking
		on them. Currently picking is only working for Meshes.
	
	Parameters:
		cameraNode  - camera used for picking
		nwx, nwy    - normalized window coordinates

	Returns:
		handle of picked node or 0 if no node was hit
*/
H3D_API H3DNode h3dutPickNode( H3DNode cameraNode, float nwx, float nwy );

/* Function: h3dutGetScreenshotParam
		Return width and height for current screenshot.

	Details:
		This function is useful in conjunction with h3dutScreenshotRaw to allocate
		arrays with the correct size.

	Parameters:
		width: will contain the screenshot width
	height: will contain the screenshot height

	Returns:
		None
*/
H3D_API void h3dutGetScreenshotParam( int *width,  int *height );

/* Function: h3dutScreenshotRaw
		Take screenshot and copy it to user provided `rgb` buffer.

	Details:
		The user must provide an RGB array and its size in bytes. The buffer must be
		at least width * height * 3 bytes large. Use `h3dutGetScreenshotParam` to
		determine the width and height.

	Parameters:
		rgb: buffer to hold the image (must be at least width * height * 3 bytes large)
		rgb_len: length of `rgb` buffer in bytes.

	Returns:
		true if screenshot could be copied.
*/
H3D_API bool h3dutScreenshotRaw( unsigned char *rgb, int rgb_len );

H3D_API bool h3dutCreateBinaryShader( H3DRes shaderResource, const char *filename );



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


#endif //#ifndef H3D_HORDE3DUTILS_C_HEADER
