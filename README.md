# Horde3D

Horde3D is an open source 3D rendering engine. It is written in an effort being as lightweight and conceptually clean as possible.

Horde3D requires a fully OpenGL 2.0 compatible graphics card. In terms of DirectX that means a card supporting at least Shader Model 2.0 or better SM 3.0. Hence the minimum hardware needed to run an application using Horde3D is a NVidia GeForce FX or ATI Radeon 9500 graphics accelerator. 

## Features


#### General Features

- Modern shader-based architecture with SM 2.0 compatible hardware as a minimum requirement
- Cross-platform compatible by using OpenGL as rendering API
- Elegant and lightweight overall design with very few dependencies, avoiding complexity where possible
- Strong modularity and high abstraction through flat C-style DLL API (also makes it possible to use Horde3D from virtually any programming language)
- Easy integration with game engines and other middleware like physics engines due to non-intrusive API design

#### Scene and Resource Management

- Robust resource management with internal garbage collection
- Interface for loading data from files, streams or any type of archives
- Hot-reloading of resources for more increased productivity during development
- Lightweight scene tree structure with hierarchical transformations and bounding volumes
- Unified scene system where world, models and skeletons are just scene graph branches and no special objects
- Loading of scene graph branches from XML files with complete orthogonality to API functions
- Frustum culling based on spatial graph
- Hardware occlusion culling
- Level of detail support for model geometry and materials
- Possibility to attach scene nodes to joints (e.g. for character props)
- Access to vertex data for collision detection and interoperability with physics engines
- Ray collision queries and node picking

#### Rendering

- Übershader-based effect/material system with automatic shader permutation generation (using GLSL shaders)
- XML based customizable rendering pipeline with allocatable render targets and commands for rapid testing of different rendering techniques
- Postprocessing framework for effects like bloom, DOF or motion blur
- Support for forward rendering and different deferred shading techniques
- Support for High Dynamic Range (HDR) textures and lighting
- Support for almost all modern rendering techniques, including normal-mapped phong lighting and parallax mapping
- Support for real-time reflections and other techniques that require several cameras for rendering
- Realtime shadows using Parallel Split Shadow Maps (PSSM)
- Software skinning and hardware skinning in vertex shader for rendering hundreds of animated characters
- Fully integrated particle systems that can cast shadows and have effects like motion blur
- Overlays for rendering GUI elements and font

#### Animation

- Unified low-level animation system working directly on scene graph
- Keyframe animation for joints and meshes
- Skeletal animation with up to 4 weights per vertex for articulated models
- Layered animation blending and mixing using masks and additive channels
- Inter-frame interpolation for smooth animations
- Access to joint data for dynamic animations and ragdoll physics
- Morph targets for facial animation and lip synchronization

#### Content Pipeline

- Custom optimized model and animation formats for maximum performance
- Mixture of binary and XML formats for best tradeoff between performance and productivity
- Support for DDS textures and other common image formats
- Collada Converter for bringing assets from many common DCC tools to Horde3D
- Collada Converter implemented as command line tool that can be integrated in automatic build process
- Calculation of tangent space basis for normal mapping
- Optimization of geometry for GPU post-transform vertex cache
- Data-driven rendering pipelines for straight switching between different rendering techniques
- **Powerful editor for composing scenes and developing shaders and rendering techniques**


## Building

You need to have a C++ compiler and [CMake 2.4+](http://www.cmake.org/) installed. CMake is a meta-build system, e.g. it creates Makefiles or Visual Studio files using Generators. The main ways to use CMake are `cmake-gui` (Qt Interface), `ccmake` (Curses Interface) and `cmake` (Commandline Interface). Instructions for commonly used generators:

- [Qt Creator](http://qt-project.org/downloads#qt-creator): open `CMakeLists.txt` as new project, follow the instructions, hit build and you're done.
- [Visual Studio](http://www.microsoft.com/visualstudio/eng/products/visual-studio-express-products): start `cmake-gui`, choose OGDF as source path, `build-vs` as build path, press generate, open `build-vs\Horde3D.sln` and start compiling.
- [Makefiles](http://www.gnu.org/software/make/): open up a terminal, navigate to the repository and run ``mkdir build-make && cd build-make && cmake -G "Unix Makefiles" .. && make`` (hint: use `export JOBS=MAX` to speed things up).
- [Ninja](http://martine.github.io/ninja/): open up a terminal, navigate to the repository and run ``mkdir build-ninja && cd build-ninja && cmake -G "Ninja" .. && ninja``.

## What's next

Here are some quick links to help you get started:

- [Homepage](http://horde3d.org/)
- [Introduction Tutorial](http://www.horde3d.org/docs/html/_tutorial.html)
- [Reference Documentation](http://www.horde3d.org/docs/manual.html)
- [Community Forums](http://www.horde3d.org/forums)
- [Community Wiki](http://horde3d.org/wiki/)

## License

Horde3D is licensed under the [Eclipse Public License v1.0 (EPL)](http://www.eclipse.org/legal/epl-v10.html).

The EPL is a quite liberal license and has less restrictions than the popular LGPL. Basically it allows you to use Horde3D in free and commercial projects as long as you contribute improvements like bug fixes, optimizations and code refactorings back to the community. The EPL allows static linking and is not viral, hence it does not affect any other modules of your application.
