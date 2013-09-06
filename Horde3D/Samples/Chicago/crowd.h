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

#ifndef _crowd_H_
#define _crowd_H_

#include "Horde3D.h"
#include <vector>
#include <string>


struct Particle
{
	float    px, pz;  // Current postition
	float    dx, dz;  // Destination position
	float    fx, fz;  // Force on particle
	float    ox, oz;  // Orientation vector
	H3DNode  node;
	float    animTime;


	Particle()
	{
		ox = 0; oz = 0;
		node = 0;
		animTime = 0;
	}
};


class CrowdSim
{
public:
	CrowdSim( const std::string& contentDir ) : _contentDir( contentDir ) {}

	void init();
	void update( float fps );

private:
	void chooseDestination( Particle &p );

private:
	std::string              _contentDir;
	std::vector< Particle >  _particles;
};

#endif // _crowd_H_
