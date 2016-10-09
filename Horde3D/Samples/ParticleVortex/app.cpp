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
#include <memory>

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

	_animTime = 0;
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
	H3DRes computeDrawMatRes = h3dAddResource( H3DResTypes::Material, "materials/computeDraw.material.xml", 0 );

	// 2. Specify compute buffer parameters

	// Create compute buffer 
	H3DRes compBuf = h3dAddResource( H3DResTypes::ComputeBuffer, "CompBuf", 0 );

	// Generate random position data for particles
	size_t particlesCount = 1000000;
	std::unique_ptr< ParticleData[] > compData = std::make_unique< ParticleData[] >( particlesCount ); //( new ParticleData[ particlesCount ] );
//	ParticleData *compData = new ParticleData[ particlesCount ];

	std::random_device rd;
	std::mt19937 gen( rd() );
	std::uniform_real_distribution<> dis( -30, 30 );

	float angle = 0;
	for ( size_t i = 0; i < particlesCount; ++i )
	{
		compData[ i ].position[ 0 ] = ( float ) dis( gen );
		compData[ i ].position[ 1 ] = ( float ) dis( gen );
		compData[ i ].position[ 2 ] = ( float ) dis( gen );

		angle = -( float ) atan2f( compData[ i ].position[ 0 ], compData[ i ].position[ 2 ] );

		compData[ i ].velocity[ 0 ] = cosf( angle );
		compData[ i ].velocity[ 1 ] = 0.f;
		compData[ i ].velocity[ 2 ] = sinf( angle ) * 5;
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
	h3dSetResParamStr( compBuf, H3DComputeBufRes::DrawParamsElem, 0, H3DComputeBufRes::DrawParamsNameStr, "partPosition" );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 0, H3DComputeBufRes::DrawParamsSizeI, 3 );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 0, H3DComputeBufRes::DrawParamsOffsetI, 0 );
	h3dSetResParamStr( compBuf, H3DComputeBufRes::DrawParamsElem, 1, H3DComputeBufRes::DrawParamsNameStr, "partVelocity" );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 1, H3DComputeBufRes::DrawParamsSizeI, 3 );
	h3dSetResParamI( compBuf, H3DComputeBufRes::DrawParamsElem, 1, H3DComputeBufRes::DrawParamsOffsetI, 12 );

	// Fill compute buffer with generated data
	void *data = h3dMapResStream( compBuf, H3DComputeBufRes::ComputeBufElem, 0, 0, false, true );
	memcpy( data, compData.get(), particlesCount * sizeof( ParticleData ) );
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

	// In order to draw the results of compute buffer we need a compute node
	_compNode = h3dAddComputeNode( H3DRootNode, "Vortex", computeDrawMatRes, compBuf );

	// Set node AABB size because currently there is no way to do it otherwise
	h3dSetNodeParamF( _compNode, H3DComputeNode::AABBMinF, 0, -30.0f ); // x
	h3dSetNodeParamF( _compNode, H3DComputeNode::AABBMinF, 1, -30.0f ); // y
	h3dSetNodeParamF( _compNode, H3DComputeNode::AABBMinF, 2, -30.0f ); // z
	h3dSetNodeParamF( _compNode, H3DComputeNode::AABBMaxF, 0, 30.0f ); // x
	h3dSetNodeParamF( _compNode, H3DComputeNode::AABBMaxF, 1, 30.0f ); // y
	h3dSetNodeParamF( _compNode, H3DComputeNode::AABBMaxF, 2, 30.0f ); // z

	// Set material uniforms that will not be changed during runtime
	h3dSetMaterialUniform( _computeMatRes, "totalParticles", ( float ) particlesCount, 0, 0, 0 );

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

	// Calculate number of groups for compute shader
	int numGroups = ( particlesCount % 1024 != 0 ) ? ( ( particlesCount / 1024 ) + 1 ) : ( particlesCount / 1024 );
	double root = pow( ( double ) numGroups, ( double ) ( 1.0 / 2.0 ) );
	root = ceil( root );
	_computeGroupX = _computeGroupY = ( unsigned int ) root;

	return true;
}


void ParticleVortexSample::releaseResources()
{
    SampleApplication::releaseResources();
}


void ParticleVortexSample::update()
{
    SampleApplication::update();
	float frame_time = 1.0f / getFPS();

    if( !checkFlag( SampleApplication::FreezeMode ) )
	{
		// Calculate animation time in seconds
		_animTime += frame_time;

		// Set animation time
		h3dSetMaterialUniform( _computeMatRes, "deltaTime", _animTime, 0, 0, 0 );

		// Set attractor point
		float angle = ( float ) _animTime * 0.5f;

		float attractorX = cosf( angle );
		float attractorY = cosf( angle ) * sinf( angle );
		float attractorZ = sinf( angle );

		h3dSetMaterialUniform( _computeMatRes, "attractor", attractorX * 2, attractorY * 2, attractorZ * 2, 0 );

		// Perform computing
		h3dDispatchCompute( _computeMatRes, "COMPUTE", _computeGroupX, _computeGroupY, 1 );
	}
}
