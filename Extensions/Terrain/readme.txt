Horde3D Terrain Extension
--------------------------

Copyright (C) 2006-2011 Nicolas Schulz and Volker Wiendl

The Terrain Extension extends Horde3D with the capability to render large landscapes. A special
level of detail algorithm adapts the resolution of the terrain mesh so that near regions get
more details than remote ones. The algorithm also considers the geometric complexity of the
terrain to increase the resoultion solely where this is really required. This makes the
rendering fast and provides a high quality with a minimum of popping artifacts.
	
A height map is used to define the altitude of the terrain. The height map is a usual texture
map that encodes 16 bit height information in two channels. The red channel of the texture
contains the coarse height, while the green channel encodes finer graduations. The encoding of
the information is usually done with an appropriate tool. If you just want to use 8 bit
height information, you can simply copy the greyscale image to the red channel of the height
map and leave the green channel black.

The extension defines the uniform *terBlockParams* and the attribute *terHeight* that can be used
in a shader to render the terrain. To see how this is working in detail, have a look at the included
sample shader.


Installation
------------
	
To install the extension, copy the Extensions directory to the path where the Horde3D SDK resides,
so that the two directories are on the same level in the hierarchy:

Horde3D
	-> Extensions
		-> Terrain
	-> Horde3D
		-> Binaries
		-> ...

In Visual Studio, add the extension and sample projects to the Horde3D solution. Then add the extension
project to the project dependencies of the Horde3D Engine and the Horde3D Engine to the dependencies of
the Terrain Sample.

Include 'Terrain/extension.h' in 'egModules.cpp' of the engine and add
'#pragma comment( lib, "Extension_Terrain.lib" )' to link against the terrain extension (under Windows).
Finally, add the following line to Modules::installExtensions to register the extension:

extMan().installExtension( new Horde3DTerrain::ExtTerrain() );

The extension is then part of the Horde3D DLL and can be used with the Horde3DTerrain.h header file.


ChangeLog
---------

0.2.3   - Adapted to Horde3D 1.0.0 Beta5
0.2.2   - Adapted to Horde3D 1.0.0 Beta4
0.2.1   - Adapted to Horde3D 1.0.0 Beta3
0.2.0   - Disabled texture compression for height map
        - Fixed culling issues
        - Fixed intersection calculation
        - Fixed visible gaps in terrain (thanks to phoenix64)
		
0.1.1   - Fixed culling issues and simplified installation

0.1.0   - Initial release


------------------------------------------------------------------------------------------
Notes on sample application
------------------------------------------------------------------------------------------

To run the sample, simply copy the contents of the extension root directory to the Horde3D
SDK folder, allow to overwrite the Horde3D.dll and run the demo executable from the
Binaries directory.

Input:

	Use WASD to move and the mouse to look around.
	Hold down LSHIFT to move faster.
	F1 sets fullscreen mode.
	F6 toggles frame stats display.
	F7 toggles debug view.
	F8 toggles wireframe mode.
	ESC quits the application.

Notes on content:

	The height map was created by Harald Österreicher.
	Many thanks to him for allowing us to use it.
