# Horde3D

Horde3D is a 3D rendering engine written in C++ with an effort being as lightweight and conceptually clean as possible.

Horde3D requires a fully OpenGL 2.0 compatible graphics card. In terms of DirectX that means a card supporting at least Shader Model 2.0 or higher. For mobile operating systems (Android and IOS) OpenGL ES 3.0 capable device is required.

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
    - Support for geometry, tessellation and compute shaders
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

You need to have a C++11 compiler and [CMake 3.7+](http://www.cmake.org/) installed.

### Use of CMake for Windows, Linux, macOs

**CMake** is a meta-build system, e.g. it creates Makefiles or Visual Studio files using Generators. The main ways to use CMake are `cmake-gui` (Qt Interface), `ccmake` (Curses Interface) and `cmake` (Commandline Interface). Instructions for commonly used generators:

- [Qt Creator](http://qt-project.org/downloads#qt-creator): open `CMakeLists.txt` as new project, follow the instructions, hit build and you're done.
- [Visual Studio](http://www.microsoft.com/visualstudio/eng/products/visual-studio-express-products): start `cmake-gui`, choose OGDF as source path, `build-vs` as build path, press generate, open `build-vs\Horde3D.sln` and start compiling. You could also generate the solution from the command line running ``mkdir build-vs && cd build-vs && cmake -G "Visual Studio XYZ" ..``, where XYZ is the correct identifier for the desired version of Visual Studio (i.e. `14 2015` for VS 2015, `15 2015` for VS 2017, etc. Please run ``cmake --help`` for more info).
- [Xcode](https://developer.apple.com/xcode/): open up a terminal, navigate to the repository and run ``mkdir build-xcode && cd build-xcode && cmake -G "Xcode" ..``, then open the generated project file inside Xcode.
- [Makefiles](http://www.gnu.org/software/make/): open up a terminal, navigate to the repository and run ``mkdir build-make && cd build-make && cmake -G "Unix Makefiles" .. && make`` (hint: use `export JOBS=MAX` to speed things up).
- [Ninja](http://martine.github.io/ninja/): open up a terminal, navigate to the repository and run ``mkdir build-ninja && cd build-ninja && cmake -G "Ninja" .. && ninja``.

### Build samples

In order to build the samples you need [GLFW](http://www.glfw.org/download.html) *(>3.x)* or [SDL](https://github.com/libsdl-org/SDL/releases) *(>=2.0.10)*.

By default, if not present on the system, a default version will be automatically downloaded, built and linked for you.

You could force this behavior using `HORDE3D_FORCE_DOWNLOAD_GLFW` flag with CMake (from your build directory):

     cmake -DHORDE3D_FORCE_DOWNLOAD_GLFW=ON ..
With SDL you can use `HORDE3D_FORCE_DOWNLOAD_SDL` flag with CMake (from your build directory):
    
     cmake -DHORDE3D_FORCE_DOWNLOAD_SDL=ON ..

On **Debian/Ubuntu** platforms, you also need to install the following packages:

     sudo apt-get install xorg-dev

You could also skip sample building using `HORDE3D_BUILD_EXAMPLES` flag with CMake (from your build directory):

     cmake -DHORDE3D_BUILD_EXAMPLES=OFF ..

### Building for Android

Building for Android requires using two build systems: CMake and Gradle. Gradle project is included in Horde3D distribution. 

Requirements:
- [Android NDK](https://developer.android.com/ndk/downloads)
- [Android SDK](https://developer.android.com/studio#downloads)
- Java runtime

Android Studio is recommended, but not required.

CMake GUI usage is recommended. Following instruction depends on using GUI for building.

- Select Horde3D source folder and folder where cmake intermediates and binaries would be stored
- Hit "Add Entry" button and add new cmake variable "ANDROID_ABI". Tested value is **arm64-v8a**.
- Hit "Add Entry" button again and add new cmake variable "ANDROID_PLATFORM". Tested value is **android-24**.
- Hit "Add Entry" button again and add new cmake variable "ANDROID_DL_LIBRARY". Cmake may incorrectly specify libdl.a instead of libdl.so and that can lead to problems with libSDL so we have to do it manually. Please set the path to: **Android NDK/toolchains/llvm/prebuilt/<platform>/sysroot/usr/lib/aarch64-linux-android/<version>/libdl.so**
--  where platform is: **windows-x86_64, linux-x86_64**
-- version is android target api version: **24, 25, ..., 29**.
- Hit configure button, create the folder if asked, and select "Unix makefiles" and "Specify toolchain file for cross-compiling".
- Select the toolchain file that is located in the **Android NDK/build/cmake/android.toolchain.cmake**
- On Windows CMake may not be able to find make.exe program that is in the NDK. Please specify the make program in **CMAKE_MAKE_PROGRAM** field and set it to **ANDROID NDK/prebuilt/windows-x86_64/bin/make.exe**
- In Horde3D submenu select OpenGL ES3 rendering backend and deselect GL2 and GL4 backends
- If you have you own SDL2 build please specify it in SDL2 submenu (path to includes and library libSDL2.so). SDL2 can also be automatically downloaded and built with Horde3D (please select **HORDE3D_FORCE_DOWNLOAD_SDL** for that in Horde3D submenu)
- Set CMAKE_BUILD_TYPE to either Debug or Release.
- If you wish you can specify signer key for release builds of Android APK, otherwise you would be asked for it during build (debug builds do not require your own key as it would be generated by Android Studio).  You can specify ANDROID_APK_SIGNER_KEYSTORE path to keystore file, ANDROID_APK_SIGNER_KEYSTORE_PASS for keystore password and ANDROID_APK_SIGNER_KEY_PASS for key password.
- Specify ANDROID_SDK_ROOT_PATH - path to Android SDK (required on Linux).
- Specify ANDROID_SDK_BUILD_TOOLS_PATH for correct release builds (path to signature tools). Default path is **Android SDK/build-tools/28.0.3**
- Generate the make files
- On Windows, Open the cmd.exe, go to the created folder and use the full path to make.exe to launch build process. Example: **Android NDK/prebuilt/windows-x86_64/bin/make.exe -j4**. 
- On Linux, open the terminal, go to the created folder and type **make**. Please note that using **j<number of cores>** parameter for multithreaded compilation **requires** you to input ANDROID_APK_SIGNER_KEYSTORE_PASS and ANDROID_APK_SIGNER_KEY_PASS in CMake, otherwise you will get a compilation error on apk signing process.

### Building for iOS

Currently, building is supported only on macOS. Only iPhone target is supported for now, iPad and tvOS are not tested,

A new version of CMAKE is recommended, Cmake 3.16 was tested and was working correctly.

Requirements:

- [XCode](https://developer.apple.com/xcode/resources/) 
- [SDL (for samples)](https://www.libsdl.org/download-2.0.php)

CMake GUI usage is recommended. Following instruction depends on using GUI for building.

SDL is required for samples. Automatic download and build is disabled for iOS as SDL fails to build with CMAKE (**HORDE3D_FORCE_DOWNLOAD_SDL** will have no effect ). Please download SDL manually and build it with XCode project, located in <SDL folder>/XCode-iOS/SDL. In **Build Settings** set **Enable Bitcode** option to **No**, 

- Select Horde3D source folder and folder where cmake intermediates and binaries would be stored
- Select the toolchain file that is located in the **<Horde3D folder>/BuildTools/ios/ios.toolchain.cmake**
- Hit **Configure** button
- Check HORDE3D_USE_GLES3 flag and uncheck HORDE3D_USE_GL2 and HORDE3D_USE_GL4 flags.
- Set SDL2 include path to <SDL folder>/include and SDL library to the generated libSDL2.a file (it is usually located in /Users/<User>/Library/Developer/Xcode/DerivedData/SDL-<arbitrary letters>/Build/Products/Debug-iphoneos/libSDL2.a)
- In IOS category set **IOS_CODE_SIGN_IDENTITY** and **IOS_DEVELOPMENT_TEAM** properties. Details on checking the values of these parameters are provided below.
- Hit **Generate** button. 
- Now XCode project is generated and can be launched.

To get your development team id go to Applications -> Utilities -> Keychain Access.
Under the 'login' Keychain, go into the 'Certificates' category.

Scroll to find your development or distribution certificate. It should be something like that:
Apple Development: <email>(<some letters and numbers>)

Double click on this certificate. "Organizational Unit" is the Team ID.

Please note that **ParticleVortex** and **Tessellator** sample will not run on iOS as OpenGL ES 3.2 is not available. 

### Build Horde3D scene editor

There is also a scene editor available for Horde3D. To enabling build of the editor, first make sure you have the Qt 4.8 or any newer Qt 5.x SDK installed. To enable creating makefiles
for the editor via cmake set the HORDE3D_BUILD_EDITOR flag to ON (default is OFF).

    cmake -DHORDE3D_BUILD_EDITOR=ON

As the editor needs Lua as a dependency you can either make sure the Lua development files can be found by cmake, or Lua will be automatically downloaded by CMake.

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
