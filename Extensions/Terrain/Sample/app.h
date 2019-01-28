// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#ifndef _app_H_
#define _app_H_

#include "../../../Horde3D/Samples/Framework/sampleapp.h"


class TerrainSample : public SampleApplication
{
public:
	TerrainSample( int argc, char** argv );

protected:
	bool initResources();
};

#endif // _app_H_
