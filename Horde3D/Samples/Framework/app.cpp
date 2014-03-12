// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Framework
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#define _CRT_SECURE_NO_WARNINGS

#include "app.h"
#include "Horde3DUtils.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <math.h>
#include <iomanip>

#define H3D_FPS_REFERENCE 60.0f

using namespace std;


// Extracts an absolute path to the resources directory given the executable path.
// It assumes that the ressources can be found in "[app path]/../../Content".
// This function implements the platform specific differences.
std::string extractResourcePath( char *fullPath )
{
	std::string s( fullPath );

#ifdef WIN32
    const char delim = '\\';
#else
    const char delim = '/';
#endif

#ifdef __APPLE__
    // On MacOSX the application is in a 'bundle' and the path has the form:
    //     "Knight.app/Contents/MacOS/Knight"
    const unsigned int nbRfind = 4;
#else
    const unsigned int nbRfind = 1;
#endif
   
    // Remove the token of path until the executable parent folder is reached
	for( unsigned int i = 0; i < nbRfind; ++i )
		s = s.substr( 0, s.rfind( delim ) );
    
    // Add the 'Content' folder
    return s + delim + ".." + delim + ".." + delim + "Content";
}


// Look for -bm (benchmark) option.
bool checkForBenchmarkOption( int argc, char** argv )
{
	// Check if benchmark mode is requested
	if( argc > 1 && strcmp( argv[1], "-bm" ) == 0 )
	    return true;
	    
    return false;
}


SampleApplication::SampleApplication( int argc, char** argv, int benchmark_length ) :
    _running(false),
    _winHandle(0),
    _winTitle("Horde3D Sample"),
    _prev_winWidth(0), _winWidth(0), _prev_winHeight(0), _winHeight(0),
    _winFullScreen(false), _winShowCursor(false),
    _resourcePath( extractResourcePath( argv[0] ) ),
    _benchmark( checkForBenchmarkOption( argc, argv ) ),
    _benchmarkLength(benchmark_length),
    _mx(0), _my(0),
    _fov(45.0f), _nearPlane(0.1f), _farPlane(1000.0f),
    _x(15), _y(3), _z(20),
    _rx(-10), _ry(60),
    _velocity(0.05f),
    _curFPS(H3D_FPS_REFERENCE),
    _statMode(0), _freezeMode(0),
    _debugViewMode(false), _wireframeMode(false),
    _showHelpPanel(false), _helpRows(10), _helpLabels(0), _helpValues(0),
    _cam(0)
{
    // Initialize GLFW
    glfwInit();
}


SampleApplication::~SampleApplication()
{
    this->release();

    // Terminate GLFW
    glfwTerminate();
}


int SampleApplication::run( int width, int height, bool fullscreen, bool show_cursor )
{
    _winWidth = width;
    _winHeight = height;
    _winFullScreen = fullscreen;
    _winShowCursor = show_cursor;
    
    if( !this->init() )
	{
		glfwTerminate();	
		return -1;
	}
	
	int frames = 0;
    float fps = H3D_FPS_REFERENCE;
	_t0 = glfwGetTime();
	
	_running = true;

	// Game loop
	while( _running )
	{	
		// Calc FPS
		++frames;

        if( !_benchmark && frames >= 3 )
		{
			double t = glfwGetTime();
            fps = frames / (float)(t - _t0);
            if( fps < 5 ) fps = H3D_FPS_REFERENCE;  // Handle breakpoints
			frames = 0;
            _t0 = t;
        }

        _curFPS = _benchmark ? H3D_FPS_REFERENCE : fps;

		this->update();
		this->render();
		this->finalize();

		if( _benchmark && frames == _benchmarkLength ) break;
	}

	// Show benchmark results
    if( _benchmark )
    {
        double avgFPS = _benchmarkLength / (glfwGetTime() - _t0);
        const char* fpsLabel = "Average FPS:";
        char* fpsValue = new char[10];
        sprintf( fpsValue, "%.2f", avgFPS );

        std::cout << fpsLabel << " " << fpsValue << std::endl;

        double startTime = glfwGetTime();
        while( glfwGetTime() - startTime < 5.0 )
        {
            const float ww = (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportWidthI ) /
                             (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportHeightI );
            h3dutShowInfoBox( (ww-0.32f) * 0.5f, 0.03f, 0.32f, "Benchmark", 1, &fpsLabel, (const char**)&fpsValue, _fontMatRes, _panelMatRes );

            this->render();
            this->finalize();
        }
	}
	
	this->release();
	
	return 0;
}


void SampleApplication::requestClosing()
{
	_running = false;
}


void SampleApplication::setTitle( const char* title )
{
    glfwSetWindowTitle( _winHandle, title );
    
    _winTitle = title;
}


void SampleApplication::toggleFullScreen()
{
    this->release();

    // Toggle fullscreen mode
    _winFullScreen = !_winFullScreen;

    if( _winFullScreen )
    {
        const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
        _prev_winWidth = _winWidth;
        _prev_winHeight = _winHeight;
	    _winWidth = mode->width;
        _winHeight = mode->height;
    }
    else
    {
        int width = _winWidth;
        int height = _winHeight;
        _winWidth = _prev_winWidth;
        _winHeight = _prev_winHeight;
        _prev_winWidth = width;
        _prev_winHeight = height;
    }

    if( !this->init() )
    {
        this->release();
        exit( -1 );
    }
    
    _t0 = glfwGetTime();
}


void SampleApplication::showCursor( bool show )
{
    glfwSetInputMode( _winHandle, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
	
    _winShowCursor = show;
}


bool SampleApplication::initResources()
{
    // 1. Add resources
	
	// Pipelines
	_forwardPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );
	_deferredPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/deferred.pipeline.xml", 0 );
    _hdrPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/hdr.pipeline.xml", 0 );
	
	// Overlays
	_fontMatRes = h3dAddResource( H3DResTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = h3dAddResource( H3DResTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = h3dAddResource( H3DResTypes::Material, "overlays/logo.material.xml", 0 );

    // Help info
    _helpLabels = (const char**)malloc(_helpRows * sizeof(char*));
    _helpValues = (const char**)malloc(_helpRows * sizeof(char*));

    if ( _helpRows > 0 ) { _helpLabels[0] = "F1:"; _helpValues[0] = "Help (ON/OFF)"; }
    if ( _helpRows > 1 ) { _helpLabels[1] = "F2:"; _helpValues[1] = "Stats (...)"; }
    if ( _helpRows > 2 ) { _helpLabels[2] = "F3:"; _helpValues[2] = "Pipeline (...)"; }
    if ( _helpRows > 3 ) { _helpLabels[3] = "F4:"; _helpValues[3] = "Debug (ON/OFF)"; }
    if ( _helpRows > 4 ) { _helpLabels[4] = "F5:"; _helpValues[4] = "Wireframe (ON/OFF)"; }
    if ( _helpRows > 5 ) { _helpLabels[5] = "F11:"; _helpValues[5] = "Fullscreen (ON/OFF)"; }
    if ( _helpRows > 6 ) { _helpLabels[6] = "Esc:"; _helpValues[6] = "Exit"; }
    if ( _helpRows > 7 ) { _helpLabels[7] = "Space:"; _helpValues[7] = "Freeze (...)"; }
    if ( _helpRows > 8 ) { _helpLabels[8] = "W/A/S/D:"; _helpValues[8] = "Movement"; }
    if ( _helpRows > 9 ) { _helpLabels[9] = "LShift:"; _helpValues[9] = "Turbo"; }
	
	// 2. Load resources

    if ( !h3dutLoadResourcesFromDisk( _resourcePath.c_str() ) )
	{
        std::cout << "Error in loading resources" << std::endl;

		h3dutDumpMessages();
        return false;
    }
	
	return true;
}


void SampleApplication::releaseResources()
{
    delete[] _helpLabels;
    _helpLabels = 0;

    delete[] _helpValues;
    _helpValues = 0;
}


void SampleApplication::update()
{
}


void SampleApplication::render()
{
	h3dSetOption( H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
	h3dSetOption( H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f );
	
	// Set camera parameters
	h3dSetNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	// Show stats
	h3dutShowFrameStats( _fontMatRes, _panelMatRes, _statMode );
	if( _statMode > 0 )
    {
        std::string piperes_name = "Pipeline: forward";
        int current_piperes = h3dGetNodeParamI( _cam, H3DCamera::PipeResI );

        if( current_piperes == _deferredPipeRes )
            piperes_name = "Pipeline: deferred";

        else if( current_piperes == _hdrPipeRes )
            piperes_name = "Pipeline: HDR";

        h3dutShowText( piperes_name.c_str(), 0.03f, 0.23f, 0.026f, 1, 1, 1, _fontMatRes );
	}

    const float ww = (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportWidthI ) /
                     (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportHeightI );

    // Show help
    if( _showHelpPanel )
    {
        h3dutShowInfoBox( ww-0.48f, 0.03f, 0.45f, "Help", _helpRows, _helpLabels, _helpValues, _fontMatRes, _panelMatRes );
    }

    // Show logo
    const float ovLogo[] = {
        ww-0.29f, 0.87f, 0, 1,
        ww-0.29f, 0.97f, 0, 0,
        ww-0.03, 0.97f, 1, 0,
        ww-0.03, 0.87f, 1, 1
    };
    h3dShowOverlays( ovLogo, 4, 1.f, 1.f, 1.f, 1.f, _logoMatRes, 0 );
	
	// Render scene
    h3dRender( _cam );
}


void SampleApplication::finalize()
{
    // Finish rendering of frame
    h3dFinalizeFrame();

    // Remove all overlays
    h3dClearOverlays();

    // Write all messages to log file
    h3dutDumpMessages();

    // Swap buffers and poll window events
    glfwSwapBuffers( _winHandle );
    glfwPollEvents();
}


void SampleApplication::keyStateHandler()
{    
	// ----------------
	// Key-press events
	// ----------------
	if( this->isKeyPressed(GLFW_KEY_ESCAPE) ) // Esc
	{
	    this->requestClosing();
	}
	
    if( this->isKeyPressed(GLFW_KEY_SPACE) )  // Space
	{
        _freezeMode = (_freezeMode + 1) % 3;
	}

    if( this->isKeyPressed(GLFW_KEY_F1) )  // F1
        _showHelpPanel = !_showHelpPanel;

    if( this->isKeyPressed(GLFW_KEY_F2) )  // F2
        _statMode = (_statMode + 1) % (H3DUTMaxStatMode+1);
	
    if( this->isKeyPressed(GLFW_KEY_F3) )  // F3
	{
        int current_piperes = h3dGetNodeParamI( _cam, H3DCamera::PipeResI );

        if( current_piperes == _forwardPipeRes )
			h3dSetNodeParamI( _cam, H3DCamera::PipeResI, _deferredPipeRes );

        else if( current_piperes == _deferredPipeRes )
            h3dSetNodeParamI( _cam, H3DCamera::PipeResI, _hdrPipeRes );

		else
			h3dSetNodeParamI( _cam, H3DCamera::PipeResI, _forwardPipeRes );
    }

    if( this->isKeyPressed(GLFW_KEY_F4) )  // F4
        _debugViewMode = !_debugViewMode;

    if( this->isKeyPressed(GLFW_KEY_F5) )  // F5
        _wireframeMode = !_wireframeMode;

    if( this->isKeyPressed(GLFW_KEY_F11) )  // F11
    {
        this->toggleFullScreen();
    }

	// --------------
	// Key-down state
	// --------------
    if( _freezeMode != 2 || _benchmark )
	{
        float curVel = _velocity / H3D_FPS_REFERENCE * _curFPS;
		
        if( this->isKeyDown(GLFW_KEY_LEFT_SHIFT) ) curVel *= 5;	// LShift
		
        if( this->isKeyDown(GLFW_KEY_W) )
		{
			// Move forward
            _x -= sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
            _y -= sinf( -degToRad( _rx ) ) * curVel;
            _z -= cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
        if( this->isKeyDown(GLFW_KEY_S) )
		{
			// Move backward
            _x += sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
            _y += sinf( -degToRad( _rx ) ) * curVel;
            _z += cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
        if( this->isKeyDown(GLFW_KEY_A) )
		{
			// Strafe left
			_x += sinf( degToRad( _ry - 90) ) * curVel;
			_z += cosf( degToRad( _ry - 90 ) ) * curVel;
		}
        if( this->isKeyDown(GLFW_KEY_D) )
		{
			// Strafe right
			_x += sinf( degToRad( _ry + 90 ) ) * curVel;
			_z += cosf( degToRad( _ry + 90 ) ) * curVel;
		}
	}
}


void SampleApplication::mouseMoveHandler()
{
    if( _freezeMode == 2 || _benchmark ) return;

    float dX = ( _mx - _prev_mx );
    float dY = ( _prev_my - _my );
	
	// Look left/right
    _ry -= dX * 0.3f;
	
	// Loop up/down but only in a limited range
    _rx += dY * 0.3f;
	if( _rx > 90 ) _rx = 90; 
	if( _rx < -90 ) _rx = -90;
}


void SampleApplication::resizeViewport( int width, int height )
{
    // Resize viewport
    h3dSetNodeParamI( _cam, H3DCamera::ViewportXI, 0 );
    h3dSetNodeParamI( _cam, H3DCamera::ViewportYI, 0 );
    h3dSetNodeParamI( _cam, H3DCamera::ViewportWidthI, width );
    h3dSetNodeParamI( _cam, H3DCamera::ViewportHeightI, height );

    // Set virtual camera parameters
    h3dSetupCameraView( _cam, _fov, (float)width / height, _nearPlane, _farPlane );
    h3dResizePipelineBuffers( _deferredPipeRes, width, height );
    h3dResizePipelineBuffers( _forwardPipeRes, width, height );
    h3dResizePipelineBuffers( _hdrPipeRes, width, height );
}


bool SampleApplication::init()
{
    this->release();

    // Init key states
    for( unsigned int i = 0; i < GLFW_KEY_LAST; ++i )
    {
        _keys[i] = false;
        _prevKeys[i] = false;
    }

    // Create OpenGL window
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    _winHandle = glfwCreateWindow( _winWidth, _winHeight, _winTitle.c_str(), _winFullScreen ? glfwGetPrimaryMonitor() : NULL, NULL );
	
    if( _winHandle == NULL )
	{
		// Fake message box
        glfwDestroyWindow(_winHandle);
		
        _winHandle = glfwCreateWindow( 800, 50, "Unable to initalize engine - Make sure you have an OpenGL 2.0 compatible graphics card", NULL, NULL );
		double startTime = glfwGetTime();
        while( glfwGetTime() - startTime < 5.0 ) { /* Sleep */ }
		
		std::cout << "Unable to initalize window" << std::endl;
        std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card" << std::endl;
		
		return false;
	}
	
    glfwSetWindowUserPointer( _winHandle, this );
    glfwMakeContextCurrent( _winHandle );
    glfwSetInputMode( _winHandle, GLFW_STICKY_KEYS, GL_TRUE );
	
	// Disable vertical synchronization
    glfwSwapInterval(0);

	// Set listeners
    glfwSetWindowCloseCallback( _winHandle, windowCloseListener );
    glfwSetWindowSizeCallback( _winHandle, windowResizeListener );
	glfwSetKeyCallback( _winHandle, keyPressListener );
	glfwSetCursorPosCallback( _winHandle, mouseMoveListener );
    
    // Init cursor
    this->showCursor( _winShowCursor );
    
	// Initialize engine
	if( !h3dInit() )
	{
		std::cout << "Unable to initalize engine" << std::endl;
		
		h3dutDumpMessages();
		return false;
	}

	// Set options
	h3dSetOption( H3DOptions::LoadTextures, 1 );
	h3dSetOption( H3DOptions::TexCompression, 0 );
	h3dSetOption( H3DOptions::MaxAnisotropy, 4 );
	h3dSetOption( H3DOptions::ShadowMapSize, 2048 );
    h3dSetOption( H3DOptions::FastAnimation, 1 );

	// Init resources
    if( !this->initResources() )
	{
		std::cout << "Unable to initalize resources" << std::endl;
		
		h3dutDumpMessages();
	    return false;
    }

    // Resize viewport
    this->resizeViewport( _winWidth, _winHeight );

    h3dutDumpMessages();
	return true;
}


void SampleApplication::release()
{    
    if( _winHandle )
    {
        // Release loaded resources
        this->releaseResources();

        // Release engine
        h3dRelease();

        // Destroy window
        glfwDestroyWindow( _winHandle );
        _winHandle = 0;
    }
}


void SampleApplication::windowCloseListener(  GLFWwindow* win )
{
    SampleApplication* app = static_cast<SampleApplication*>( glfwGetWindowUserPointer( win ) );
    
    if( app )
        app->_running = false;
}


void SampleApplication::windowResizeListener(  GLFWwindow* win, int width, int height )
{
    SampleApplication* app = static_cast<SampleApplication*>( glfwGetWindowUserPointer( win ) );

    if( app )
    {
        // Avoid registering resize due to
        // window decoration adjustment in
        // fullscreen mode.
        if (!app->_winFullScreen)
        {
            app->_prev_winWidth = app->_winWidth;
            app->_prev_winHeight = app->_winHeight;
            app->_winWidth = width;
            app->_winHeight = height;
        }

        app->resizeViewport( width, height );
    }
}


void SampleApplication::keyPressListener( GLFWwindow* win, int key, int scancode, int action, int mods )
{
    SampleApplication* app = static_cast<SampleApplication*>( glfwGetWindowUserPointer( win ) );
    
    if ( app && app->_running )
    {
        app->_prevKeys[key] = app->_keys[key];
        app->_keys[key] = ( action == GLFW_PRESS || action == GLFW_REPEAT );

        app->keyStateHandler();
    }
}


void SampleApplication::mouseMoveListener( GLFWwindow* win, double x, double y )
{
    SampleApplication* app = static_cast<SampleApplication*>( glfwGetWindowUserPointer( win ) );
    
    if ( app && app->_running )
    {
        app->_prev_mx = app->_mx;
        app->_prev_my = app->_my;
        
		app->_mx = x;
        app->_my = y;

        app->mouseMoveHandler();
	}
}
