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

#ifndef _app_H_
#define _app_H_

#include "../Framework/sampleapp.h"

class TessellatorSample : public SampleApplication
{
public:
    TessellatorSample( int argc, char** argv );

protected:
    bool initResources();
    void releaseResources();

    virtual void keyEventHandler( int key, int keyState, int mods );

    void update();

private:
	float _animTime;
	unsigned int _tessInner;
	unsigned int _tessOuter;

	float _rotation;

	H3DNode _model;
};

#endif // _app_H_



