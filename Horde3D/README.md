Horde3D SDK v2.0.0
==================

> Next-Generation Graphics Engine

http://www.horde3d.org

## Introduction

Horde3D requires an OpenGL 2.0 compatible graphics card with the latest drivers.
A GeForce 6 or Radeon X1000 series card is the minimum requirement to run the samples.

The source code of the engine and tools is included in the SDK. It has the following dependencies:

 * **RapidXml**: http://rapidxml.sourceforge.net
 * **stbi** by Sean Barrett: http://nothings.org
 * **GLFW** for window management in samples: http://glfw.sourceforge.net
		
These libraries are included directly as code in the SDK.

## Release Notes
	
The 1.0 release brings a lot of new features and performance improvements. 
Some notable features:

 * New OpenGL 4 render interface, fully functional in OpenGL core profile. Also works with GL 3.3.
 * Horde3D now supports all types of shaders, including geometry, tesselation and compute shaders. 
 * Preferred render interface may be specified. Engine will fallback to OpenGL 2 if OpenGL 4 render interface is unavailable.
 * Small performance optimizations in h3dRender function.
 * CRTP variation is used instead of virtual functions for render interface class. 
   In release version function call performs like a direct call because of inlining, 
   in debug version it can be a bit slower than virtual function.
 * Shader storage buffer objects (SSBO) that can be used to pass large amount of arbitrary data to shaders.
 * Horde3D can now be used on big-endian systems.
 * New samples that demonstrate compute and tessellation shaders use.

## License

Copyright (C) 2006-2017 Nicolas Schulz and the Horde3D Team
	
The complete SDK is licensed under the terms of the Eclipse Public License (EPL).
	
Special thanks go to the University of Augsburg for supporting this project!
	
