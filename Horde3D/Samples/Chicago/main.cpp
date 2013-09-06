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

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "glfw.h"
#include "app.h"

// Configuration
const int appWidth = 1024;
const int appHeight = 576;
static bool fullScreen = false;
static int benchmarkLength = 600;

bool setupWindow( int, int, bool );
static bool running;
static double t0;
static int mx0, my0;
static Application *app;


std::string extractAppPath( char *fullPath )
{
#ifdef __APPLE__
	std::string s( fullPath );
	for( int i = 0; i < 4; ++i )
		s = s.substr( 0, s.rfind( "/" ) );
	return s + "/../";
#else
	const std::string s( fullPath );
	if( s.find( "/" ) != std::string::npos )
		return s.substr( 0, s.rfind( "/" ) ) + "/";
	else if( s.find( "\\" ) != std::string::npos )
		return s.substr( 0, s.rfind( "\\" ) ) + "\\";
	else
		return "";
#endif
}


int windowCloseListener()
{
	running = false;
	return 0;
}


void keyPressListener( int key, int action )
{
	if( !running ) return;

	if( action == GLFW_PRESS )
	{
		int width = appWidth, height = appHeight;
		
		switch (key)
		{
		case GLFW_KEY_ESC:
			running = false;
			break;
		case GLFW_KEY_F1:
			app->release();
			glfwCloseWindow();
			
			// Toggle fullscreen mode
			fullScreen = !fullScreen;

			if( fullScreen )
			{
				GLFWvidmode mode;
				glfwGetDesktopMode( &mode );
				
				float aspect = mode.Width / (float)mode.Height;
				if( (int)(aspect * 100) == 133 || (int)(aspect * 100) == 125 )  // Standard
				{
					width = 1280; height = 1024;
				}
				else if( (int)(aspect * 100) == 177 )                           // Widescreen 16:9
				{
					width = 1280; height = 720;
				}
				else if( (int)(aspect * 100) == 160 )                           // Widescreen 16:10
				{
					width = 1280; height = 800;
				}
				else                                                            // Unknown
				{
					// Use desktop resolution
					width = mode.Width; height = mode.Height;
				}
			}
			
			if( !setupWindow( width, height, fullScreen ) )
			{
				glfwTerminate();
				exit( -1 );
			}
			
			app->init();
			app->resize( width, height );
			t0 = glfwGetTime();
			break;
		}
	}
}


void mouseMoveListener( int x, int y )
{
	if( !running )
	{
		mx0 = x; my0 = y;
		return;
	}

	app->mouseMoveEvent( (float)(x - mx0), (float)(my0 - y) );
	mx0 = x; my0 = y;
}


bool setupWindow( int width, int height, bool fullscreen )
{
	// Create OpenGL window
	if( !glfwOpenWindow( width, height, 8, 8, 8, 8, 24, 8, fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW ) )
	{
		glfwTerminate();
		return false;
	}
	
	// Disable vertical synchronization
	glfwSwapInterval( 0 );

	// Set listeners
	glfwSetWindowCloseCallback( windowCloseListener );
	glfwSetKeyCallback( keyPressListener );
	glfwSetMousePosCallback( mouseMoveListener );
	
	return true;
}


int main( int argc, char** argv )
{
	// Initialize GLFW
	glfwInit();
	glfwEnable( GLFW_STICKY_KEYS );
	if( !setupWindow( appWidth, appHeight, fullScreen ) ) return -1;

	// Check if benchmark mode is requested
	bool benchmark = false;
	if( argc > 1 && strcmp( argv[1], "-bm" ) == 0 )
	{	
		benchmark = true;
		glfwDisable( GLFW_AUTO_POLL_EVENTS );
	}
	
	// Initialize application and engine
	app = new Application( extractAppPath( argv[0] ) );
	if( !fullScreen ) glfwSetWindowTitle( app->getTitle() );
	
	if ( !app->init() )
	{
		// Fake message box
		glfwCloseWindow();
		glfwOpenWindow( 800, 16, 8, 8, 8, 8, 24, 8, GLFW_WINDOW );
		glfwSetWindowTitle( "Unable to initalize engine - Make sure you have an OpenGL 2.0 compatible graphics card" );
		double startTime = glfwGetTime();
		while( glfwGetTime() - startTime < 5.0 ) {}  // Sleep
		
		std::cout << "Unable to initalize engine" << std::endl;
		std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card";
		glfwTerminate();
		return -1;
	}
	app->resize( appWidth, appHeight );

	glfwDisable( GLFW_MOUSE_CURSOR );

	int frames = 0;
	float fps = 30.0f;
	t0 = glfwGetTime();
	running = true;

	// Game loop
	while( running )
	{	
		// Calc FPS
		++frames;
		if( !benchmark && frames >= 3 )
		{
			double t = glfwGetTime();
			fps = frames / (float)(t - t0);
			if( fps < 5 ) fps = 30;  // Handle breakpoints
			frames = 0;
			t0 = t;
		}

		// Update key states
		for( int i = 0; i < 320; ++i )
			app->setKeyState( i, glfwGetKey( i ) == GLFW_PRESS );
		app->keyStateHandler();

		// Render
		app->mainLoop( benchmark ? 60 : fps );
		glfwSwapBuffers();

		if( benchmark && frames == benchmarkLength ) break;
	}

	glfwEnable( GLFW_MOUSE_CURSOR );

	// Show benchmark results
	if( benchmark )
	{	
		double avgFPS = benchmarkLength / (glfwGetTime() - t0);
		char title[256];
		sprintf( title, "Average FPS: %.2f", avgFPS );
		glfwCloseWindow();
		glfwOpenWindow( 800, 16, 8, 8, 8, 8, 24, 8, GLFW_WINDOW );
		glfwSetWindowTitle( title );
		double startTime = glfwGetTime();
		while( glfwGetTime() - startTime < 5.0 ) {}  // Sleep
	}
	
	// Quit
	app->release();
	delete app;
	glfwTerminate();

	return 0;
}
