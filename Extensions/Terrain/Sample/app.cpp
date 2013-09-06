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
#include "Horde3DUtils.h"
#include "Horde3DTerrain.h"
#include <math.h>
#include <iomanip>

using namespace std;

// Convert from degrees to radians
inline float degToRad( float f ) 
{
	return f * (3.1415926f / 180.0f);
}


Application::Application( const string &appDir )
{
	for( unsigned int i = 0; i < 320; ++i )
	{	
		_keys[i] = false;
		_prevKeys[i] = false;
	}

	_x = 512; _y = 120; _z = 512; _rx = 0; _ry = 225; _velocity = 10.0f;
	_curFPS = 30;

	_statMode = 0;
	_freezeMode = 0; _debugViewMode = false; _wireframeMode = false;
	_cam = 0;

	_contentDir = appDir + "../Content" + "|" + appDir + "../../../Extensions/Terrain/Sample/Content";
}


bool Application::init()
{	
	// Initialize engine
	if( !h3dInit() )
	{	
		h3dutDumpMessages();
		return false;
	}

	// Set options
	h3dSetOption( H3DOptions::LoadTextures, 1 );
	h3dSetOption( H3DOptions::TexCompression, 0 );
	h3dSetOption( H3DOptions::MaxAnisotropy, 4 );
	h3dSetOption( H3DOptions::ShadowMapSize, 2048 );
	h3dSetOption( H3DOptions::FastAnimation, 1 );

	// Add resources
	// Pipeline
	H3DRes pipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );
	// Overlays
	_fontMatRes = h3dAddResource( H3DResTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = h3dAddResource( H3DResTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = h3dAddResource( H3DResTypes::Material, "overlays/logo.material.xml", 0 );
	// Terrain
	H3DRes terrainRes = h3dAddResource( H3DResTypes::SceneGraph, "terrains/terrain1/terrain1.scene.xml", 0 );
	
	
	// Load resources
	h3dutLoadResourcesFromDisk( _contentDir.c_str() );

	// Add scene nodes
	// Add camera
	_cam = h3dAddCameraNode( H3DRootNode, "Camera", pipeRes );
	// Add terrain
	H3DNode terrain = h3dAddNodes( H3DRootNode, terrainRes );
	
	/*// Add light source
	H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP" );
	h3dSetNodeTransform( light, 512, 700, -256, -120, 0, 0, 1, 1, 1 );
	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 2000 );
	h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
	h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.5f );
	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.005f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 1.0f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.7f );*/

	// Set sun direction for ambient pass
	H3DRes matRes = h3dFindResource( H3DResTypes::Material, "terrains/terrain1/terrain1.material.xml" );
	h3dSetMaterialUniform( matRes, "sunDir", 1, -1, 0, 0 );

	return true;
}


void Application::mainLoop( float fps )
{
	_curFPS = fps;
	
	h3dSetOption( H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
	h3dSetOption( H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f );
	
	// Set camera parameters
	h3dSetNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	// Show stats
	h3dutShowFrameStats( _fontMatRes, _panelMatRes, _statMode );

	// Show logo
	const float ww = (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportWidthI ) /
	                 (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportHeightI );
	const float ovLogo[] = { ww-0.4f, 0.8f, 0, 1,  ww-0.4f, 1, 0, 0,  ww, 1, 1, 0,  ww, 0.8f, 1, 1 };
	h3dShowOverlays( ovLogo, 4, 1.f, 1.f, 1.f, 1.f, _logoMatRes, 0 );
	
	// Render scene
	h3dRender( _cam );

	// Finish rendering of frame
	h3dFinalizeFrame();

	// Remove all overlays
	h3dClearOverlays();

	// Write all messages to log file
	h3dutDumpMessages();
}


void Application::release()
{
	// Release engine
	h3dRelease();
}


void Application::resize( int width, int height )
{
	// Resize viewport
	h3dSetNodeParamI( _cam, H3DCamera::ViewportXI, 0 );
	h3dSetNodeParamI( _cam, H3DCamera::ViewportYI, 0 );
	h3dSetNodeParamI( _cam, H3DCamera::ViewportWidthI, width );
	h3dSetNodeParamI( _cam, H3DCamera::ViewportHeightI, height );
	
	// Set virtual camera parameters
	h3dSetupCameraView( _cam, 45.0f, (float)width / height, 0.5f, 2048.0f );
	h3dResizePipelineBuffers( h3dGetNodeParamI( _cam, H3DCamera::PipeResI ), width, height );
}


void Application::keyStateHandler()
{
	// ----------------
	// Key-press events
	// ----------------
	if( _keys[32] && !_prevKeys[32] )  // Space
	{
		if( ++_freezeMode == 3 ) _freezeMode = 0;
	}
	
	if( _keys[264] && !_prevKeys[264] )  // F7
		_debugViewMode = !_debugViewMode;

	if( _keys[265] && !_prevKeys[265] )  // F8
		_wireframeMode = !_wireframeMode;
	
	if( _keys[263] && !_prevKeys[263] )  // F6
	{
		_statMode += 1;
		if( _statMode > H3DUTMaxStatMode ) _statMode = 0;
	}

	// --------------
	// Key-down state
	// --------------
	if( _freezeMode != 2 )
	{
		float curVel = _velocity / _curFPS;
		
		if( _keys[287] ) curVel *= 10;	// LShift
		
		if( _keys['W'] )
		{
			// Move forward
			_x -= sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
			_y -= sinf( -degToRad( _rx ) ) * curVel;
			_z -= cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
		if( _keys['S'] )
		{
			// Move backward
			_x += sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
			_y += sinf( -degToRad( _rx ) ) * curVel;
			_z += cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
		if( _keys['A'] )
		{
			// Strafe left
			_x += sinf( degToRad( _ry - 90) ) * curVel;
			_z += cosf( degToRad( _ry - 90 ) ) * curVel;
		}
		if( _keys['D'] )
		{
			// Strafe right
			_x += sinf( degToRad( _ry + 90 ) ) * curVel;
			_z += cosf( degToRad( _ry + 90 ) ) * curVel;
		}
	}
}


void Application::mouseMoveEvent( float dX, float dY )
{
	if( _freezeMode == 2 ) return;
	
	// Look left/right
	_ry -= dX / 100 * 30;
	
	// Loop up/down but only in a limited range
	_rx += dY / 100 * 30;
	if( _rx > 90 ) _rx = 90; 
	if( _rx < -90 ) _rx = -90;
}
