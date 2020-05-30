// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

// For IOS we have to replace main function with SDL_main
#if defined( __APPLE__ ) || defined( __APPLE_CC__ )
#include "AvailabilityMacros.h"
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
#include "SDL_main.h"
#ifdef __cplusplus
    extern "C"
#endif
#endif
#endif

#include "app.h"

int main( int argc, char** argv )
{
    ParticleVortexSample app( argc, argv );
   	if ( !app.init() ) return 1;

    return app.run();
}
