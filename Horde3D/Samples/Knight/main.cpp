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
#include "GLFW/glfw3.h"
#include "app.h"

// Configuration
const int appWidth = 1024;
const int appHeight = 576;
static bool fullScreen = false;
static int benchmarkLength = 600;

GLFWwindow* setupWindow( int, int, bool );
static bool running;
static bool fullscreenToggled = false;
static double t0;
static int mx0 = 0, my0 = 0;
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


void windowCloseListener( GLFWwindow* window )
{
    running = false;
}


void keyPressListener( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if( !running ) return;

    if( action == GLFW_PRESS )
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            running = false;
            break;
        case GLFW_KEY_F1:
            app->release();
            
            // Toggle fullscreen mode
            fullScreen = !fullScreen;
            fullscreenToggled = true;

            break;
        }
    }
}


void mouseMoveListener( GLFWwindow* window, double x, double y )
{
    int xi = (int)floor(x);
    int yi = (int)floor(y);
    if( !running )
    {
        mx0 = xi; my0 = yi;
        return;
    }

    app->mouseMoveEvent( (float)(xi - mx0), (float)(my0 - yi) );
    mx0 = xi; my0 = yi;
}


GLFWwindow* setupWindow( int width, int height, bool fullscreen )
{
    // Create OpenGL window
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    GLFWwindow* window = NULL;
    if( !(window = glfwCreateWindow( width, height, "Knight", fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL ) ) )
    {
        glfwTerminate();
        return NULL;
    }
    
    glfwMakeContextCurrent( window );
    
    // Disable vertical synchronization
    glfwSwapInterval( 0 );

    // Set listeners
    glfwSetWindowCloseCallback( window, windowCloseListener );
    glfwSetKeyCallback( window, keyPressListener );
    glfwSetCursorPosCallback( window, mouseMoveListener );
    
    return window;
}


int main( int argc, char** argv )
{
    // Initialize GLFW
    glfwInit();
    GLFWwindow* window = NULL;
    if( !(window = setupWindow( appWidth, appHeight, fullScreen ) ) ) return -1;
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    // Check if benchmark mode is requested
    bool benchmark = false;
    if( argc > 1 && strcmp( argv[1], "-bm" ) == 0 )
    {    
        benchmark = true;
    }
    
    // Initialize application and engine
    app = new Application( extractAppPath( argv[0] ) );
    if( !fullScreen ) glfwSetWindowTitle( window, app->getTitle() );
    
    if ( !app->init() )
    {
        // Fake message box
        glfwDestroyWindow(window);
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 8);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        window = glfwCreateWindow( 800, 16, "Unable to initalize engine - Make sure you have an OpenGL 2.0 compatible graphics card", NULL, NULL );
        glfwMakeContextCurrent( window );
        double startTime = glfwGetTime();
        while( glfwGetTime() - startTime < 5.0 ) {}  // Sleep
        
        std::cout << "Unable to initalize engine" << std::endl;
        std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card";
        glfwTerminate();
        return -1;
    }
    app->resize( appWidth, appHeight );

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

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

        // Poll keys and act on them
        if (!benchmark)
        {
            glfwPollEvents();
        }

        if (fullscreenToggled)
        {
            fullscreenToggled = false;

            glfwDestroyWindow(window);
            window = NULL;
            int width = appWidth, height = appHeight;

            if( fullScreen )
            {
                const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );

                float aspect = mode->width / (float)mode->height;
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
                    width = mode->width; height = mode->height;
                }
            }

            if( !(window = setupWindow( width, height, fullScreen ) ) )
            {
                glfwTerminate();
                exit( -1 );
            }

            app->init();
            app->resize( width, height );
            t0 = glfwGetTime();
        }

        // Update key states
        for( int i = 0; i <= GLFW_KEY_LAST; ++i )
            app->setKeyState( i, glfwGetKey( window, i ) == GLFW_PRESS );
        app->keyStateHandler();

        // Render
        app->mainLoop( benchmark ? 60 : fps );
        glfwSwapBuffers(window);

        if( benchmark && frames == benchmarkLength ) break;
    }

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

    // Show benchmark results
    if( benchmark )
    {    
        double avgFPS = benchmarkLength / (glfwGetTime() - t0);
        char title[256];
        sprintf( title, "Average FPS: %.2f", avgFPS );
        glfwDestroyWindow(window);
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 8);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        window = glfwCreateWindow( 800, 16, title, NULL, NULL );
        glfwMakeContextCurrent( window );
        double startTime = glfwGetTime();
        while( glfwGetTime() - startTime < 5.0 ) {}  // Sleep
    }
    
    // Quit
    app->release();
    delete app;
    glfwTerminate();

    return 0;
}
