# Horde3D

Horde3D is a 3D rendering engine written in C++ with an effort being as lightweight and conceptually clean as possible.

Horde3D requires a fully OpenGL 2.0 compatible graphics card. In terms of DirectX that means a card supporting at least Shader Model 2.0 or higher.

## Features

- Modern, cross-platform, shader-based architecture (requires OpenGL 2.0+)
    - Lightweight, non-intrusive design with very few dependencies, avoiding complexity where possible
    - C-style API for easy usage from virtually any programming language
- Resource management
    - Garbage collected resources, loaded from virtually any type of data stream
    - Hot-reloading of resources for more increased productivity during development
    - Access to vertex data for collision detection and interoperability with physics engines
- Übershader-based, customizable rendering pipeline
    - Hot-reloading of pipelines for rapid testing of different rendering techniques
    - Support for post processing effects like bloom, DOF or motion blur
    - Support for almost all forward, deferred and High Dynamic Range rendering techniques
    - Support for real-time reflections and other techniques that require several cameras
    - Real-time shadows using Parallel Split Shadow Maps (PSSM)
    - Particle systems that can cast shadows and have effects like motion blur
- Unified scene system
    - World, models and skeletons are scene nodes instead of special objects
    - Frustum culling based on spatial graph
    - Hardware occlusion culling
    - Level of detail for model geometry and materials
- Unified, low-level animation system
    - Key frame animation for joints and meshes
    - Skeletal animation with up to 4 weights per vertex for articulated models
    - Layered animation blending and mixing using masks and additive channels
    - Morph targets for facial animation and lip synchronization
    - Access to joint data for dynamic animations and ragdoll physics
- Content Pipeline
    - Mixture of binary and XML formats for best tradeoff between performance and productivity
        - Pipeline, material and scene descriptions are XML
        - Model and animation are binary for maximum performance
        - Textures are common image formats (DDS, PNG, JPEG, etc.)
    - COLLADA Converter for importing assets from many common DCC tools
      - Calculation of tangent space basis for normal mapping
      - Optimization of geometry for GPU post-transform vertex cache
    - Editor for composing scenes, developing shaders and rendering techniques

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

The EPL is a quite liberal license and has fewer restrictions than the popular LGPL. Basically it allows you to use Horde3D in free and commercial projects as long as you contribute improvements like bug fixes, optimizations and code refactorings back to the community. The EPL allows static linking and is not viral; hence it does not affect any other modules of your application.
