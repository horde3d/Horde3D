Horde3D 
-------	
	Next-Generation Graphics Engine
	
		-------
		-S D K-
		-------
		
		Version 1.0.0 
		

Copyright (C) 2006-2017 Nicolas Schulz and the Horde3D Team

http://www.horde3d.org

Contact: See website

	
The complete SDK is licensed under the terms of the Eclipse Public License (EPL).

Horde3D requires an OpenGL 2.0 compatible graphics card with the latest drivers.
A GeForce 6 or Radeon X1000 series card is the minimum requirement to run the samples.

The source code of the engine and tools is included in the SDK. It has the following dependencies:

	- RapidXml
		http://rapidxml.sourceforge.net
	- stbi by Sean Barrett
		http://nothings.org
	- GLFW for window management in samples
		http://glfw.sourceforge.net
		
These libraries are included directly as code in the SDK.


Release Notes:
	
	The 1.0 release brings a lot of new features and performance improvements. 
	Some notable features:
	- new OpenGL 4 render interface, fully functional in OpenGL core profile. Also works with GL 3.3.
    - Horde3D now supports all types of shaders, including geometry, tesselation and compute shaders. 
	- preferred render interface may be specified. Engine will fallback to OpenGL 2 if OpenGL 4 render interface is unavailable.
    - small performance optimizations in h3dRender function.
    - CRTP variation is used instead of virtual functions for render interface class. 
	  In release version function call performs like a direct call because of inlining, 
	  in debug version it can be a bit slower than virtual function.
    - Shader storage buffer objects (SSBO) that can be used to pass large amount of arbitrary data to shaders.
	- Horde3D can now be used on big-endian systems.
	- New samples that demonstrate compute and tessellation shaders use.
	
Special thanks go to the University of Augsburg for supporting this project!


------------------------------------------------------------------------------------------
Description of included samples
------------------------------------------------------------------------------------------

Chicago
-------

This sample uses a simple crowd simulation to show how Horde3D can
be used to render environments and a plenty of animated models.
The demo supports forward and deferred rendering which can be
selected by modifying the pipeline_Chicago.xml file.

Input:

	Use WASD to move and the mouse to look around.
	Hold down LSHIFT to move faster.
	Space freezes the scene, hitting space two times freezes the camera as well.
	F1 sets fullscreen mode.
	F3 switches between forward, hdr and deferred shading.
	F6 toggles frame stats display.
	F7 toggles debug view.
	F8 toggles wireframe mode.
	ESC quits the application.

Notes on content:

	The character model was created by Sirda from TurboSquid.
	Some of the textures are taken from the ATI Radeon SDK and are thus
	copyrighted by ATI Technologies, Inc.
	

	
Knight
------

This sample shows a knight model with reflections coming from a cube map. The
pipeline applies high dynamic range lighting with simple tone mapping and a
bloom effect. The knight has a particle system attached to the tip of his sword
which results in some interesting graphical effects. Using the keyboard it is
possible to blend seamlessly between two animations.


Input:

	Use WASD to move and the mouse to look around.
	Hold down LSHIFT to move faster.
	Use 1 and 2 to blend between character animations.
	Space freezes the scene, hitting space two times freezes the camera as well.
	F1 sets fullscreen mode.
	F3 switches between hdr, standard forward lighting and deferred shading.
	F6 toggles frame stats and information display.
	F7 toggles debug view.
	F8 toggles wireframe mode.
	ESC quits the application.

Notes on content:

	The character model and animations were created by Rob Galanakis (www.robg3d.com)
	and are distributed under the terms of the Creative Commons Attribution-
	Noncommercial 3.0 License (http://creativecommons.org/licenses/by-nc/3.0/).
	The cubemap texture is a modified version of one of M@dcow's high res skymaps
	which can be found at BlenderArtist.org.


Particle Vortex
------

This sample shows particle simulation process using compute shaders. Currently up to 1 million
particles are simulated. 


Input:

	Use WASD to move and the mouse to look around.
	Hold down LSHIFT to move faster.
	Space freezes the scene, hitting space two times freezes the camera as well.
	F1 sets fullscreen mode.
	F3 switches between hdr, standard forward lighting and deferred shading (currently works only with forward shading).
	F6 toggles frame stats and information display.
	F7 toggles debug view.
	F8 toggles wireframe mode.
	ESC quits the application.

Notes on content:

	This sample is inspired by GPU Particles DirectX 11 example, created by ForhaxeD. Link (in russian): https://habrahabr.ru/post/248755/
	

	
Tessellator
------

This sample shows procedurally generated tessellatable mesh that resembles icosahedron. 
Tessellation levels can be adjusted.


Input:

	Use WASD to move and the mouse to look around.
	Hold down LSHIFT to move faster.
	Space freezes the scene, hitting space two times freezes the camera as well.
	Up/Down arrows to modify tessellation level of the mesh.
	F1 sets fullscreen mode.
	F3 switches between hdr, standard forward lighting and deferred shading (currently works only with forward shading).
	F6 toggles frame stats and information display.
	F7 toggles debug view.
	F8 toggles wireframe mode.
	ESC quits the application.

Notes on content:

	This sample is inspired by OpenGL tessellation example, created by Philip Rideout. Link: http://prideout.net/blog/?p=48
	