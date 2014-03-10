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

#include "app.h"
#include "crowd.h"
#include "Horde3D.h"
#include "Horde3DUtils.h"
#include <math.h>
#include <iomanip>

using namespace std;


ChicagoSample::ChicagoSample( int argc, char** argv ) :
    SampleApplication( argc, argv ),
    _crowdSim(0)
{
    _winTitle = "Chicago - Horde3D Sample";
    _x = 15; _y = 3; _z = 20;
    _rx = -10; _ry = 60;
}


bool ChicagoSample::initResources()
{
    if ( !SampleApplication::initResources() )
        return false;

    // 1. Add resources

	// Shader for deferred shading
	H3DRes lightMatRes = h3dAddResource( H3DResTypes::Material, "materials/light.material.xml", 0 );
	// Environment
	H3DRes envRes = h3dAddResource( H3DResTypes::SceneGraph, "models/platform/platform.scene.xml", 0 );
	// Skybox
	H3DRes skyBoxRes = h3dAddResource( H3DResTypes::SceneGraph, "models/skybox/skybox.scene.xml", 0 );

    // 2. Load resources

    if ( !h3dutLoadResourcesFromDisk( _resourcePath.c_str() ) )
    {
		h3dutDumpMessages();
        return false;
    }

    // 3. Add scene nodes

	// Add camera
	_cam = h3dAddCameraNode( H3DRootNode, "Camera", _forwardPipeRes );
	//h3dSetNodeParamI( _cam, H3DCamera::OccCullingI, 1 );
	// Add environment
	H3DNode env = h3dAddNodes( H3DRootNode, envRes );
	h3dSetNodeTransform( env, 0, 0, 0, 0, 0, 0, 0.23f, 0.23f, 0.23f );
	// Add skybox
	H3DNode sky = h3dAddNodes( H3DRootNode, skyBoxRes );
	h3dSetNodeTransform( sky, 0, 0, 0, 0, 0, 0, 210, 50, 210 );
	h3dSetNodeFlags( sky, H3DNodeFlags::NoCastShadow, true );
	// Add light source
	H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP" );
	h3dSetNodeTransform( light, 0, 20, 50, -30, 0, 0, 1, 1, 1 );
	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 200 );
	h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
	h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.001f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );

	_crowdSim = new CrowdSim( _resourcePath );
	_crowdSim->init();

	return true;
}


void ChicagoSample::releaseResources()
{
    SampleApplication::releaseResources();

    delete _crowdSim;
    _crowdSim = 0x0;
}


void ChicagoSample::update()
{
	if( !_freezeMode )
	{
		_crowdSim->update( _curFPS );
    }
}
