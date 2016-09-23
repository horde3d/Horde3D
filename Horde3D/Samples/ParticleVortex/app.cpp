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

#include "app.h"
#include "Horde3D.h"
#include "Horde3DUtils.h"
#include <math.h>
#include <iomanip>
#include <random>

using namespace std;

struct ParticleData
{
	float position[ 3 ];
	float velocity[ 3 ];
};

ParticleVortexSample::ParticleVortexSample( int argc, char** argv ) :
    SampleApplication( argc, argv, "Particle vortex - Horde3D Sample" )
{
    _x = 15; _y = 3; _z = 20;
    _rx = -10; _ry = 60;
}


bool ParticleVortexSample::initResources()
{
    if ( !SampleApplication::initResources() )
        return false;

    // 1. Add resources

	// Shader for deferred shading
	H3DRes lightMatRes = h3dAddResource( H3DResTypes::Material, "materials/light.material.xml", 0 );

    // Shader that contains geometry and compute shaders for particles
	_computeMatRes = h3dAddResource( H3DResTypes::Material, "materials/compute.material.xml", 0 );

	// 2. Specify compute buffer parameters

	// Create compute buffer 
	H3DRes compBuf = h3dAddResource( H3DResTypes::ComputeBuffer, "CompBuf", 0 );

	// Generate random position data for particles
	size_t particlesCount = 1000000;
	ParticleData *compData = new ParticleData[ particlesCount ];

	std::random_device rd;
	std::mt19937 gen( rd() );
	std::uniform_real_distribution<> dis( -30, 30 );

	for ( size_t i = 0; i < particlesCount; ++i )
	{
		compData[ i ].position[ 0 ] = dis( gen );
		compData[ i ].position[ 1 ] = dis( gen );
		compData[ i ].position[ 2 ] = dis( gen );

		compData[ i ].velocity[ 0 ] = 0.f;
		compData[ i ].velocity[ 1 ] = 0.f;
		compData[ i ].velocity[ 2 ] = 0.f;
	}

	// Set size of the compute buffer
	h3dSetResParamI( compBuf, H3DComputeBufRes::ComputeBufElem, 0, H3DComputeBufRes::CompBufDataSizeI, particlesCount * sizeof( ParticleData ) );

	// Mark that compute buffer will be used for rendering as a vertex buffer
	h3dSetResParamI( compBuf, H3DComputeBufRes::ComputeBufElem, 0, H3DComputeBufRes::CompBufUseAsVertexBufferI, 1 );

	// Set preferred draw type (for this example we draw with points - 2)
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawTypeElem, 0, H3DComputeBufRes::DataDrawTypeI, 2 );

	// Set number of elements to draw (for this example we draw 1000000 points)
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 0, H3DComputeBufRes::DrawParamsElementsCountI, 1000000 );

	// Set vertex binding parameters.
	// Name - name of the parameter. Used for binding parameter to shader variable.
	// Size - number of components (3 float for particle position, so 3), 
	// Offset - number of bytes. For second parameter it is 12, because the first parameter had 3 floats (12 bytes)
	h3dSetResParamStr( compBuf, H3DComputeBufRes::DrawParamsElem, 0, H3DComputeBufRes::DrawParamsNameStr, "position" );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 0, H3DComputeBufRes::DrawParamsSizeI, 3 );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 0, H3DComputeBufRes::DrawParamsOffsetI, 0 );
	h3dSetResParamStr( compBuf, H3DComputeBufRes::DrawParamsElem, 1, H3DComputeBufRes::DrawParamsNameStr, "velocity" );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 1, H3DComputeBufRes::DrawParamsSizeI, 3 );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 1, H3DComputeBufRes::DrawParamsOffsetI, 12 );

	// Fill compute buffer with generated data
	void *data = h3dMapResStream( compBuf, H3DComputeBufRes::ComputeBufElem, 0, 0, false, true );
	memcpy( data, compData, particlesCount * sizeof( ParticleData ) );
	h3dUnmapResStream( compBuf );

    // 3. Load resources

    if ( !h3dutLoadResourcesFromDisk( getResourcePath() ) )
    {
		h3dutDumpMessages();
        return false;
    }

    // 4. Add scene nodes

	// Add camera
	_cam = h3dAddCameraNode( H3DRootNode, "Camera", getPipelineRes() );
	//h3dSetNodeParamI( _cam, H3DCamera::OccCullingI, 1 );

	// In order to draw the results of compute buffer we need a compute node
	_compNode = h3dAddComputeNode( H3DRootNode, "Vortex", _computeMatRes, compBuf );

    // Add light source
// 	H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP" );
// 	h3dSetNodeTransform( light, 0, 20, 50, -30, 0, 0, 1, 1, 1 );
// 	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 200 );
// 	h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
// 	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
// 	h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.9f );
// 	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.001f );
// 	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
// 	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
// 	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );

	return true;
}


void ParticleVortexSample::releaseResources()
{
    SampleApplication::releaseResources();
}


void ParticleVortexSample::update()
{
    SampleApplication::update();

    if( !checkFlag( SampleApplication::FreezeMode ) )
	{

	}
}
