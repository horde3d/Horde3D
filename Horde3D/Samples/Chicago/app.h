// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2021 Nicolas Schulz and Horde3D team
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#ifndef _app_H_
#define _app_H_

#include "../Framework/sampleapp.h"

class CrowdSim;

class ChicagoSample : public SampleApplication
{
public:
    ChicagoSample( int argc, char** argv );

protected:
    bool initResources();
    void releaseResources();

    void update();
	
private:
    CrowdSim     *_crowdSim;
};

#endif // _app_H_



