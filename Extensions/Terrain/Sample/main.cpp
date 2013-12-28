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
#include <GLFW/glfw3.h>
#include "app.h"

// Configuration
const int appWidth = 1024;
const int appHeight = 576;
static bool fullScreen = false;
static int benchmarkLength = 600;

bool setupWindow( int, int, bool );
static bool running;
static double t0;
static double mx0, my0;
static Application *app;

GLFWwindow* windowFromGLFW = NULL;


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


void windowCloseListener( GLFWwindow *windowFromGLFW )
{
	running = false;
}


void keyPressListener( GLFWwindow *windowFromGLFW, int key, int scancode, int action, int mods )
{
	if( !running ) return;

	if( action == GLFW_PRESS )
	{
		int width = appWidth, height = appHeight;
		
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			running = false;
			break;
		case GLFW_KEY_F1:
			app->release();
			glfwDestroyWindow(windowFromGLFW);
			
			// Toggle fullscreen mode
			fullScreen = !fullScreen;

			if( fullScreen )
			{
			    const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );			
				width = mode->width;
				height = mode->height;
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


void mouseMoveListener( GLFWwindow* windowFromGLFW, double x, double y )
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
	windowFromGLFW = glfwCreateWindow( width, height, "Horde3D", fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL );
	
	if( windowFromGLFW == NULL )
	{
		glfwTerminate();
		return false;
	}
	
	glfwMakeContextCurrent( windowFromGLFW );
	glfwSetInputMode( windowFromGLFW, GLFW_STICKY_KEYS, true );
	
	// Disable vertical synchronization
	glfwSwapInterval( 0 );

	// Set listeners
	glfwSetWindowCloseCallback( windowFromGLFW, windowCloseListener );
	glfwSetKeyCallback( windowFromGLFW, keyPressListener );
	glfwSetCursorPosCallback( windowFromGLFW, mouseMoveListener );
	
	return true;
}


int main( int argc, char** argv )
{
	// Initialize GLFW
	glfwInit();
	
	if( !setupWindow( appWidth, appHeight, fullScreen ) ) return -1;

	// Check if benchmark mode is requested
	bool benchmark = false;
	if( argc > 1 && strcmp( argv[1], "-bm" ) == 0 )
	{	
		benchmark = true;
	}
	
	// Initialize application and engine
	app = new Application( extractAppPath( argv[0] ) );
	if( !fullScreen ) glfwSetWindowTitle( windowFromGLFW, app->getTitle() );
	
	if ( !app->init() )
	{
		// Fake message box
		glfwDestroyWindow( windowFromGLFW );
		windowFromGLFW = glfwCreateWindow(800, 16, "Unable to initalize engine - Make sure you have an OpenGL 2.0 compatible graphics card", NULL, NULL );
		double startTime = glfwGetTime();
		while( glfwGetTime() - startTime < 5.0 ) {}  // Sleep
		
		std::cout << "Unable to initalize engine" << std::endl;
		std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card";
		glfwTerminate();
		return -1;
	}
	app->resize( appWidth, appHeight );

	glfwSetInputMode( windowFromGLFW, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

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
			app->setKeyState( i, glfwGetKey( windowFromGLFW, i ) == GLFW_PRESS );
		app->keyStateHandler();

		// Render
		app->mainLoop( benchmark ? 60 : fps );
		glfwSwapBuffers( windowFromGLFW );
		glfwPollEvents();

		if( benchmark && frames == benchmarkLength ) break;
	}

	glfwSetInputMode( windowFromGLFW, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

	// Show benchmark results
	if( benchmark )
	{	
		double avgFPS = benchmarkLength / (glfwGetTime() - t0);
		char title[256];
		sprintf( title, "Average FPS: %.2f", avgFPS );
		glfwDestroyWindow( windowFromGLFW );
		windowFromGLFW = glfwCreateWindow( 800, 16, title, NULL, NULL );
		glfwMakeContextCurrent( windowFromGLFW );
		double startTime = glfwGetTime();
		while( glfwGetTime() - startTime < 5.0 ) {}  // Sleep
	}
	
	// Quit
	app->release();
	delete app;
	glfwTerminate();

	return 0;
}
