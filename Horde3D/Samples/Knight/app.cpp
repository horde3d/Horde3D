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
#include <iostream>
#include <math.h>
#include <iomanip>

using namespace std;


KnightSample::KnightSample( int argc, char** argv ) :
    SampleApplication( argc, argv, "Knight - Horde3D Sample", H3DRenderDevice::OpenGL2 ),
    _animTime(0),
    _weight(1.0f)
{
    _x = 5; _y = 3; _z = 19;
    _rx = 7; _ry = 15;
    _helpRows += 1;
    
    // Default to HDR pipeline
    _curPipeline = 2;
}


bool KnightSample::initResources()
{	
    if ( !SampleApplication::initResources() )
        return false;

	// 1. Add resources
	h3dSetOption( H3DOptions::FastAnimation, 0 );

	// Environment
    H3DRes envRes = h3dAddResource( H3DResTypes::SceneGraph, "models/sphere/sphere.scene.xml", 0 );

    // Knight
	H3DRes knightRes = h3dAddResource( H3DResTypes::SceneGraph, "models/knight/knight.scene.xml", 0 );
	H3DRes knightAnim1Res = h3dAddResource( H3DResTypes::Animation, "animations/knight_order.anim", 0 );
	H3DRes knightAnim2Res = h3dAddResource( H3DResTypes::Animation, "animations/knight_attack.anim", 0 );

    // Shader for deferred shading
    H3DRes lightMatRes = h3dAddResource( H3DResTypes::Material, "materials/light.material.xml", 0 );

    // Particle system
	H3DRes particleSysRes = h3dAddResource( H3DResTypes::SceneGraph, "particles/particleSys1/particleSys1.scene.xml", 0 );

    // Help info
    _helpLabels[_helpRows-1] = "1/2:"; _helpValues[_helpRows-1] = "Animation blending";

	// 2. Load resources

    if ( !h3dutLoadResourcesFromDisk( getResourcePath() ) )
	{
		h3dutDumpMessages();
        return false;
    }

	// 3. Add scene nodes
	
	// Add camera
	_cam = h3dAddCameraNode( H3DRootNode, "Camera", getPipelineRes() );
	//h3dSetNodeParamI( _cam, H3DCamera::OccCullingI, 1 );

    // Add environment
	H3DNode env = h3dAddNodes( H3DRootNode, envRes );
	h3dSetNodeTransform( env, 0, -20, 0, 0, 0, 0, 20, 20, 20 );

    // Add knight
	_knight = h3dAddNodes( H3DRootNode, knightRes );
	h3dSetNodeTransform( _knight, 0, 0, 0, 0, 180, 0, 0.1f, 0.1f, 0.1f );
	h3dSetupModelAnimStage( _knight, 0, knightAnim1Res, 0, "", false );
	h3dSetupModelAnimStage( _knight, 1, knightAnim2Res, 0, "", false );

    // Attach particle system to hand joint
	h3dFindNodes( _knight, "Bip01_R_Hand", H3DNodeTypes::Joint );
	H3DNode hand = h3dGetNodeFindResult( 0 );
	_particleSys = h3dAddNodes( hand, particleSysRes );
	h3dSetNodeTransform( _particleSys, 0, 40, 0, 90, 0, 0, 1, 1, 1 );

	// Add light source
    H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP" );
	h3dSetNodeTransform( light, 0, 15, 10, -60, 0, 0, 1, 1, 1 );
	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 30 );
	h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 1 );
	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.01f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 1.0f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.8f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.7f );
	h3dSetNodeParamF( light, H3DLight::ColorMultiplierF, 0, 1.0f );

	// Customize post processing effects
	H3DNode matRes = h3dFindResource( H3DResTypes::Material, "pipelines/postHDR.material.xml" );
	h3dSetMaterialUniform( matRes, "hdrExposure", 2.5f, 0, 0, 0 );
	h3dSetMaterialUniform( matRes, "hdrBrightThres", 0.5f, 0, 0, 0 );
	h3dSetMaterialUniform( matRes, "hdrBrightOffset", 0.08f, 0, 0, 0 );
	
	return true;
}

void KnightSample::update()
{
    SampleApplication::update();

    int freeze_mode = checkFlag( SampleApplication::FreezeMode );
    float frame_time = 1.0f / getFPS();

    // --------------
    // Key-down state
    // --------------
    if( freeze_mode != 2 )
    {
        if( isKeyDown(GLFW_KEY_1) )
        {
            // Change blend weight
            _weight += frame_time;
            if( _weight > 1 ) _weight = 1;
        }
        if( isKeyDown(GLFW_KEY_2) )
        {
            // Change blend weight
            _weight -= frame_time;
            if( _weight < 0 ) _weight = 0;
        }
    }
	
    // ----------------
    // Update animation
    // ----------------
    if( freeze_mode == 0 )
    {
        _animTime += frame_time;

		// Do animation blending
        h3dSetModelAnimParams( _knight, 0, _animTime * 24.0f, _weight );
        h3dSetModelAnimParams( _knight, 1, _animTime * 24.0f, 1.0f - _weight );
		h3dUpdateModel( _knight, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry );

		// Animate particle systems (several emitters in a group node)
		unsigned int cnt = h3dFindNodes( _particleSys, "", H3DNodeTypes::Emitter );
		for( unsigned int i = 0; i < cnt; ++i )
            h3dUpdateEmitter( h3dGetNodeFindResult( i ), frame_time );
	}
	
    if( checkFlag( SampleApplication::StatMode ) > 0 )
	{	
		// Display weight
		_text.str( "" );
		_text << fixed << setprecision( 2 ) << "Weight: " << _weight;
        h3dutShowText( _text.str().c_str(), 0.175f, 0.26f, 0.026f, 1, 1, 1, _fontMatRes );
	}
}
