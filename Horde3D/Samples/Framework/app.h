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

#ifndef _framework_app_H_
#define _framework_app_H_

#include "Horde3D.h"
#include <GLFW/glfw3.h>
#include <string>

#define H3D_DEFAULT_SAMPLE_BENCHMARK_LENGTH 600
#define H3D_DEG2RAD 0.017453292f


class SampleApplication
{
public:
    SampleApplication( int argc, char** argv, int benchmark_length = H3D_DEFAULT_SAMPLE_BENCHMARK_LENGTH );
	virtual ~SampleApplication();

	const char *getTitle() const { return _winTitle.c_str(); }
	int getWidth() const { return _winWidth; }
	int getHeight() const { return _winHeight; }
	bool isFullScreen() const { return _winFullScreen; }
	bool isCursorVisible() const { return _winShowCursor; }
	
    int run( int width = 1024, int height = 576, bool fullscreen = false, bool show_cursor = false );
	void requestClosing();
	
	void setTitle( const char* title );
	void resize( int width, int height );
	void toggleFullScreen();
	void showCursor( bool show );
	
protected:
    bool isKeyPressed(int key) const;
    
    virtual bool initResources();
    virtual void releaseResources();
    
    virtual void update();
    virtual void render();
    virtual void finalize();
    
    virtual void keyStateHandler();
    virtual void mouseMoveHandler();

    static float degToRad( float f ) { return f * H3D_DEG2RAD; }

private:
	bool init();
    void release();
	
	// GLFW listeners.
    static void windowCloseListener(  GLFWwindow* win );
    static void keyPressListener( GLFWwindow* win, int key, int scancode, int action, int mods );
    static void mouseMoveListener( GLFWwindow* win, double x, double y );

protected:
	bool         _running;
    GLFWwindow*  _winHandle;
	std::string  _winTitle;
    int          _prev_winWidth, _winWidth;
    int          _prev_winHeight, _winHeight;
    bool         _winFullScreen;
    bool         _winShowCursor;
	std::string  _resourcePath;
	bool         _benchmark;
	int          _benchmarkLength;
    
    bool         _keys[GLFW_KEY_LAST], _prevKeys[GLFW_KEY_LAST];
    float        _mx, _my, _prev_mx, _prev_my;

	float        _x, _y, _z, _rx, _ry;  // Viewer position and orientation
	float        _velocity;  // Velocity for movement
	float        _curFPS;
	double       _t0;

	int          _statMode;
	int          _freezeMode;
	bool         _debugViewMode, _wireframeMode;
	
	// Engine objects
	H3DRes       _fontMatRes, _panelMatRes;
	H3DRes       _logoMatRes, _forwardPipeRes, _deferredPipeRes, _hdrPipeRes;
	H3DNode      _cam;
};

#endif // _framework_app_H_



