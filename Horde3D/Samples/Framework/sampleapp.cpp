// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Framework
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#include "sampleapp.h"
#include "Horde3DUtils.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <math.h>
#include <iomanip>

#include "Horde3DOverlays.h"

#if defined ( GLFW_BACKEND )
	#include "GLFWFramework.h"
	typedef GLFWBackend Backend;
#elif defined( SDL_BACKEND )
	#include "SDLFramework.h"
	typedef SDLBackend Backend;
#endif
using namespace std;
#include <chrono>


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
    //     "SampleApplication.app/Contents/MacOS/SampleApplication"
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


SampleApplication::SampleApplication(int argc, char** argv,
        const char* title,
        int renderer,
        float fov, float near_plane, float far_plane,
        int width, int height,
        bool fullscreen, bool show_cursor,
        int benchmark_length ) :
    _x(15), _y(3), _z(20),
    _rx(-10), _ry(60),
    _velocity(0.1f),
	_helpRows(12), _helpLabels(0), _helpValues(0),
    _curPipeline(0),
    _cam(0),
    _renderInterface( renderer ),
    _initialized(false),
    _running(false),
    _resourcePath( extractResourcePath( argv[0] ) ),
    _benchmark( checkForBenchmarkOption( argc, argv ) ),
    _benchmarkLength(benchmark_length),
    _curFPS(H3D_FPS_REFERENCE),
//    _winHandle(0),
    _winTitle(title),
    _initWinWidth(width), _initWinHeight(height),
    _winSampleCount(0), _sampleCount(0),
    _winFullScreen(fullscreen),
    _prevMx(0), _prevMy(0),
    _winShowCursor(show_cursor), _winHasCursor(false),
    _fov(fov), _nearPlane(near_plane), _farPlane(far_plane),
    _statMode(0), _freezeMode(0),
    _debugViewMode(false), _wireframeMode(false),_showHelpPanel(false)
{
    // Initialize backend
	_backend = new Backend();
}


SampleApplication::~SampleApplication()
{
    release();
	
	if ( _backend )
	{
		_backend->Release();
		delete _backend; _backend = nullptr;
	}
}


bool SampleApplication::init()
{
	// Init params can be changed in derived user applications
	auto params = setupInitParameters();
	if ( !_backend->Init( params ) ) return false;

	auto winParams = setupWindowParameters();
	if ( ( _winHandle = _backend->CreateWindow( winParams ) ) == nullptr ) return false;

	// Initialize engine
	if ( !h3dInit( ( H3DRenderDevice::List ) _renderInterface ) )
	{
		std::cout << "Unable to initialize engine" << std::endl;

		h3dutDumpMessages();
		return false;
	}

	// Samples require overlays extension in order to display information
	if ( !h3dCheckExtension( "Overlays" ) )
	{
		std::cout << "Unable to find overlays extension" << std::endl;
		return false;
	}

	// Set options
	h3dSetOption( H3DOptions::LoadTextures, 1 );
	h3dSetOption( H3DOptions::TexCompression, 0 );
	h3dSetOption( H3DOptions::MaxAnisotropy, 4 );
	h3dSetOption( H3DOptions::ShadowMapSize, 2048 );
	h3dSetOption( H3DOptions::FastAnimation, 1 );
	h3dSetOption( H3DOptions::SampleCount, ( float ) _sampleCount );
	h3dSetOption( H3DOptions::DumpFailedShaders, 1 );

	// Init resources
	if ( !initResources() )
	{
		std::cout << "Unable to initialize resources" << std::endl;

		h3dutDumpMessages();
		return false;
	}

	// Setup camera and resize buffers
	resizeViewport();

	h3dutDumpMessages();

	// Attach input callbacks
	Delegate< void( int, int, int ) > keyboardDelegate;
	keyboardDelegate.bind< SampleApplication, &SampleApplication::keyEventHandler >( this );
 	_backend->RegisterKeyboardEventHandler( keyboardDelegate );

	Delegate< void( float, float, float, float ) > mouseMoveDelegate;
	mouseMoveDelegate.bind< SampleApplication, &SampleApplication::mouseMoveHandler >( this );
	_backend->RegisterMouseMoveEventHandler( mouseMoveDelegate );

	// Indicate that everything is ok
	_initialized = true;

	return true;
}


void SampleApplication::release()
{
	if ( _winHandle )
	{
		// Release loaded resources
		releaseResources();

		// Release engine
		h3dRelease();

		// Destroy window
		_backend->DestroyWindow( _winHandle );
		_winHandle = nullptr;
	}
}


BackendInitParameters SampleApplication::setupInitParameters()
{
	BackendInitParameters params;
	switch ( _renderInterface )
	{
		case ( int ) RenderAPI::OpenGL2:
			params.requestedAPI = RenderAPI::OpenGL2;
			params.majorVersion = 3;
			params.minorVersion = 1;
			break;
		case ( int ) RenderAPI::OpenGL4:
			params.requestedAPI = RenderAPI::OpenGL4;
#ifdef __APPLE__
			// macOS supports GL up to 4.1, no support for compute shaders
			params.majorVersion = 4;
			params.minorVersion = 1;
#else
			// other OS support GL up to 4.6
			params.majorVersion = 4;
			params.minorVersion = 3;
#endif
			break;
		case ( int ) RenderAPI::OpenGLES3:
			params.requestedAPI = RenderAPI::OpenGLES3;
			params.majorVersion = 3;
			params.minorVersion = 0; // set to 3.0 in order to support IOS
			break;
		default:
			break;
	}

	params.sampleCount = _winSampleCount;

	return std::move( params );
}


WindowCreateParameters SampleApplication::setupWindowParameters()
{
	WindowCreateParameters winParams;
	winParams.width = _initWinWidth;
	winParams.height = _initWinHeight;
	winParams.fullScreen = _winFullScreen;
	winParams.windowTitle = _winTitle;
	winParams.swapInterval = 0;

	return std::move( winParams );
}


void SampleApplication::getSize( int &width, int &height ) const
{
	if ( _winHandle ) _backend->GetSize( _winHandle, &width, &height );
	else { width = -1; height = -1; }
}


void SampleApplication::setTitle( const char* title )
{
	_backend->SetWindowTitle( _winHandle, title );

    _winTitle = title;
}


void SampleApplication::toggleFullScreen()
{
    if( !_winFullScreen )
        getSize( _initWinWidth, _initWinHeight );

    // Toggle fullscreen mode
    _winFullScreen = !_winFullScreen;
    
    // Force to recreate window on the next frame
    // (this cannot be done in the event handler)
    _initialized = false;
}


void SampleApplication::setPipeline( int pipline )
{
    _curPipeline = pipline;

    int newWinSampleCount = _curPipeline == 0 ? _sampleCount : 0;
    if ( _winSampleCount == newWinSampleCount )
    {
        // Sample count of the default FBO doesn't change, just adjust the pipeline
        h3dSetNodeParamI( _cam, H3DCamera::PipeResI, _pipelineRes[_curPipeline] );
    }
    else
    {
        // Otherwise, re-create the window
        _winSampleCount = newWinSampleCount;
        _initialized = false;
    }
}

void SampleApplication::setSampleCount( int sampleCount )
{
    if ( sampleCount < 0 || sampleCount > 32 || sampleCount == _sampleCount )
        return;

    // Only forward pipeline requires default framebuffer with multisampling
    _winSampleCount = _curPipeline == 0 ? sampleCount : 0;
    _sampleCount = sampleCount;
    
    // Force to recreate window on the next frame
    _initialized = false;
}


void SampleApplication::enableDebugViewMode( bool enabled )
{
    if ( enabled ) enableWireframeMode(false);

    h3dSetOption( H3DOptions::DebugViewMode, enabled ? 1.0f : 0.0f );

    _debugViewMode = enabled;
}


void SampleApplication::enableWireframeMode( bool enabled )
{
    if ( enabled ) enableDebugViewMode(false);

    h3dSetOption( H3DOptions::WireframeMode, enabled ? 1.0f : 0.0f );

    _wireframeMode = enabled;
}


void SampleApplication::showCursor( bool visible )
{
	_backend->SetCursorVisible( _winHandle, visible );

    _winShowCursor = visible;
}


void SampleApplication::showHelpPanel( bool visible )
{
    _showHelpPanel = visible;
}


void SampleApplication::showStatPanel( int mode )
{
    _statMode = mode % (H3DUTMaxStatMode+1);
}


void SampleApplication::setFreezeMode( int mode )
{
    _freezeMode = mode % 3;
}


int SampleApplication::run()
{
//	int frames = 0;
	_running = true;

	if ( !_initialized ) return -1;

	// Game loop
#ifdef __EMSCRIPTEN__
	const int simulate_infinite_loop = 1; // call the function repeatedly
	const int fps = -1; // call the function as fast as the browser wants to render (typically 60fps)
	emscripten_set_main_loop_arg( mainloop, this, fps, simulate_infinite_loop );
#else
	while ( _running )
	{
		mainLoop( this );
	}
#endif

	// Show benchmark results
    if( _benchmark )
    {
		auto t = std::chrono::high_resolution_clock::now();
        double avgFPS = _benchmarkLength / ( std::chrono::duration_cast<std::chrono::duration< double > >( t - _t0 ).count() );
        const char* fpsLabel = "Average FPS:";
		const char* fpsValue = new char[ 16 ];
        sprintf( (char*)fpsValue, "%.2f", avgFPS );

        std::cout << fpsLabel << " " << fpsValue << std::endl;

        double startTime = glfwGetTime();
        while( glfwGetTime() - startTime < 5.0 )
        {
            const float ww = (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportWidthI ) /
                             (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportHeightI );
            h3dShowInfoBox( (ww-0.32f) * 0.5f, 0.03f, 0.32f, "Benchmark", 1, &fpsLabel, &fpsValue, _fontMatRes, _panelMatRes );

            render();
            finalize();
        }

		delete[] fpsValue;
	}
	
    release();
	
	return 0;
}


void SampleApplication::mainLoop( void *arg )
{
	SampleApplication *app = static_cast< SampleApplication * > ( arg );
	static float fps = H3D_FPS_REFERENCE;
	static int frames = 0;
	static bool firstCall = true;

	if ( firstCall )
	{
		app->_t0 = std::chrono::high_resolution_clock::now();
		firstCall = false;
	}

	// 1. Calc FPS
	++frames;
	if ( !app->_benchmark && frames >= 3 )
	{
		auto t = std::chrono::high_resolution_clock::now();
		fps = frames / ( float ) ( std::chrono::duration_cast< std::chrono::duration<double> >( t - app->_t0 ).count() );
		if ( fps < 5 ) fps = H3D_FPS_REFERENCE;  // Handle breakpoints
		frames = 0;
		app->_t0 = t;
	}

	app->_curFPS = app->_benchmark ? H3D_FPS_REFERENCE : fps;

	// 2. Poll window events...
	app->_backend->ProcessEvents();

	// 3. ...update logic...
	app->update();

	// 4. ...render and finalize frame.
	app->render();
	app->finalize();

	if ( app->_benchmark && frames == app->_benchmarkLength ) app->requestClosing();
}

void SampleApplication::requestClosing()
{
#ifdef __EMSCRIPTEN__
	emscripten_cancel_main_loop();
#else
	_running = false;
#endif // __EMSCRIPTEN__
}


bool SampleApplication::initResources()
{
    // 1. Add resources
	
	// Pipelines
	_pipelineRes[0] = h3dAddResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );
	_pipelineRes[1] = h3dAddResource( H3DResTypes::Pipeline, "pipelines/deferred.pipeline.xml", 0 );
	_pipelineRes[2] = h3dAddResource( H3DResTypes::Pipeline, "pipelines/hdr.pipeline.xml", 0 );
	
	// Overlays
	_fontMatRes = h3dAddResource( H3DResTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = h3dAddResource( H3DResTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = h3dAddResource( H3DResTypes::Material, "overlays/logo.material.xml", 0 );

    // Help info
    _helpLabels = new const char*[_helpRows];
    _helpValues = new const char*[_helpRows];

    if ( _helpRows > 0 ) { _helpLabels[0] = "F1:"; _helpValues[0] = "Help (ON/OFF)"; }
    if ( _helpRows > 1 ) { _helpLabels[1] = "F2:"; _helpValues[1] = "Stats (...)"; }
    if ( _helpRows > 2 ) { _helpLabels[2] = "F3:"; _helpValues[2] = "Pipeline (...)"; }
    if ( _helpRows > 3 ) { _helpLabels[3] = "F4:"; _helpValues[3] = "Debug (ON/OFF)"; }
    if ( _helpRows > 4 ) { _helpLabels[4] = "F5:"; _helpValues[4] = "Wireframe (ON/OFF)"; }
    if ( _helpRows > 5 ) { _helpLabels[5] = "F6:"; _helpValues[5] = "MSAA: decrease samples"; }
    if ( _helpRows > 6 ) { _helpLabels[6] = "F7:"; _helpValues[6] = "MSAA: increase samples"; }
    if ( _helpRows > 7 ) { _helpLabels[7] = "F11:"; _helpValues[7] = "Fullscreen (ON/OFF)"; }
    if ( _helpRows > 8 ) { _helpLabels[8] = "Esc:"; _helpValues[8] = "Exit"; }
    if ( _helpRows > 9 ) { _helpLabels[9] = "Space:"; _helpValues[9] = "Freeze (...)"; }
    if ( _helpRows > 10 ) { _helpLabels[10] = "W/A/S/D:"; _helpValues[10] = "Movement"; }
    if ( _helpRows > 11 ) { _helpLabels[11] = "LShift:"; _helpValues[11] = "Turbo"; }
	
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
    if( _freezeMode != 2 || _benchmark )
    {
        float curVel = _velocity / _curFPS * H3D_FPS_REFERENCE;

        if( _backend->CheckKeyDown( _winHandle, KEY_LEFT_SHIFT) ) curVel *= 5;	// LShift

        if( _backend->CheckKeyDown( _winHandle, KEY_W ) )
        {
            // Move forward
            _x -= sinf( H3D_DEG2RAD * ( _ry ) ) * cosf( -H3D_DEG2RAD * ( _rx ) ) * curVel;
            _y -= sinf( -H3D_DEG2RAD * ( _rx ) ) * curVel;
            _z -= cosf( H3D_DEG2RAD * ( _ry ) ) * cosf( -H3D_DEG2RAD * ( _rx ) ) * curVel;
        }

        if( _backend->CheckKeyDown( _winHandle, KEY_S ) )
        {
            // Move backward
            _x += sinf( H3D_DEG2RAD * ( _ry ) ) * cosf( -H3D_DEG2RAD * ( _rx ) ) * curVel;
            _y += sinf( -H3D_DEG2RAD * ( _rx ) ) * curVel;
            _z += cosf( H3D_DEG2RAD * ( _ry ) ) * cosf( -H3D_DEG2RAD * ( _rx ) ) * curVel;
        }

        if( _backend->CheckKeyDown( _winHandle, KEY_A ) )
        {
            // Strafe left
            _x += sinf( H3D_DEG2RAD * ( _ry - 90) ) * curVel;
            _z += cosf( H3D_DEG2RAD * ( _ry - 90 ) ) * curVel;
        }

        if( _backend->CheckKeyDown( _winHandle, KEY_D ) )
        {
            // Strafe right
            _x += sinf( H3D_DEG2RAD * ( _ry + 90 ) ) * curVel;
            _z += cosf( H3D_DEG2RAD * ( _ry + 90 ) ) * curVel;
        }
    }
}


void SampleApplication::render()
{
    if ( !_cam )
        return;
	
	// Set camera parameters
	h3dSetNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	// Show stats
	h3dShowFrameStats( _fontMatRes, _panelMatRes, _statMode );
	if( _statMode > 0 )
    {
        std::string piperes_name = "Pipeline: forward";

        if( _curPipeline == 1 )
            piperes_name = "Pipeline: deferred";

        else if( _curPipeline == 2 )
            piperes_name = "Pipeline: HDR";

        h3dShowText( piperes_name.c_str(), 0.03f, 0.23f, 0.026f, 1, 1, 1, _fontMatRes );
        
        char buf[64];
        if ( _sampleCount == 0 ) {
            strcpy(buf, "MSAA: off");
        } else {
            sprintf( buf, "MSAA: %d", _sampleCount );
        }
        h3dShowText( buf, 0.03f, 0.26f, 0.026f, 1, 1, 1, _fontMatRes );
	}

    const float ww = (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportWidthI ) /
                     (float)h3dGetNodeParamI( _cam, H3DCamera::ViewportHeightI );

    // Show help
    if( _showHelpPanel )
    {
        h3dShowInfoBox( ww-0.48f, 0.03f, 0.45f, "Help", _helpRows, _helpLabels, _helpValues, _fontMatRes, _panelMatRes );
    }

    // Show logo
    const float ovLogo[] = {
        ww-0.29f, 0.87f, 0, 1,
        ww-0.29f, 0.97f, 0, 0,
        ww-0.03f, 0.97f, 1, 0,
        ww-0.03f, 0.87f, 1, 1
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

    // Swap buffers
    _backend->SwapBuffers( _winHandle );
}


void SampleApplication::keyEventHandler( int key, int keyState, int mods )
{
	if ( keyState != KEY_PRESS )
        return;

    switch ( key )
    {
    case KEY_ESCAPE:
    {
        requestClosing();
    }
    break;

    case KEY_SPACE:
    {
        setFreezeMode( _freezeMode + 1 );
    }
    break;

    case KEY_F1:
    {
        showHelpPanel( !_showHelpPanel );
    }
    break;

    case KEY_F2:
    {
        showStatPanel( _statMode + 1 );
    }
    break;

    case KEY_F3:
    {
        setPipeline( (_curPipeline + 1) % 3 );
    }
    break;

    case KEY_F4:
    {
        enableDebugViewMode( !_debugViewMode );
    }
    break;

    case KEY_F5:
    {
        enableWireframeMode( !_wireframeMode );
    }
    break;
            
    case KEY_F6:
    {
        setSampleCount( _sampleCount > 2 ? _sampleCount / 2 : 0 );
    }
    break;

    case KEY_F7:
    {
        setSampleCount( _sampleCount ? _sampleCount * 2 : 2 );
    }
    break;

    case KEY_F11:
    {
        toggleFullScreen();
    }
    break;
    }
}


void SampleApplication::mouseMoveHandler( float x, float y, float prev_x, float prev_y )
{
    if( _freezeMode == 2 || _benchmark ) return;

    float dx = x - prev_x;
    float dy = prev_y - y;

	// Look left/right
    _ry -= dx * 0.3f;
	
	// Loop up/down but only in a limited range
    _rx += dy * 0.3f;
	if( _rx > 90 ) _rx = 90; 
	if( _rx < -90 ) _rx = -90;
}


void SampleApplication::resizeViewport()
{
    int width, height;
    getSize( width, height );

    // Resize viewport
    h3dSetNodeParamI( _cam, H3DCamera::ViewportXI, 0 );
    h3dSetNodeParamI( _cam, H3DCamera::ViewportYI, 0 );
    h3dSetNodeParamI( _cam, H3DCamera::ViewportWidthI, width );
    h3dSetNodeParamI( _cam, H3DCamera::ViewportHeightI, height );

    // Set virtual camera parameters
    h3dSetupCameraView( _cam, _fov, (float)width / height, _nearPlane, _farPlane );
    h3dResizePipelineBuffers( _pipelineRes[0], width, height );
    h3dResizePipelineBuffers( _pipelineRes[1], width, height );
    h3dResizePipelineBuffers( _pipelineRes[2], width, height );
}


