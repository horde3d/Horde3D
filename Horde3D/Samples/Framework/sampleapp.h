// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Framework
// --------------------------------------
// Copyright (C) 2006-2021 Nicolas Schulz and Horde3D team
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#ifndef _sample_app_H_
#define _sample_app_H_

#include "Horde3D.h"
#include <string>
#include <chrono>

#define H3D_RAD2DEG 57.324840764f
#define H3D_DEG2RAD  0.017453292f
#define H3D_FPS_REFERENCE 60.0f
#define H3D_DEFAULT_SAMPLE_BENCHMARK_LENGTH 600

class FrameworkBackend;
struct BackendInitParameters;
struct WindowCreateParameters;

struct FingerData
{
    int lastPosX = 0;
    int lastPosY = 0;
    int fingerID = -1;
    bool active = false;
};

struct RenderCapabilities
{
	enum List 
	{
		GeometryShader = 2,
		TessellationShader = 4,
		ComputeShader = 8,
        DebugBackend = 16
	};
};

class SampleApplication
{
public:
    enum ApplicationFlag
    {
        Running,
        FullScreen,
        CursorVisible,
        CursorInside,
        StatMode,
        FreezeMode,
        DebugViewMode,
        WireframeMode,
        BenchmarkMode,
        BenchmarkLength,
    };

public:
    SampleApplication(
            int argc, char** argv,
            const char* title = "Horde3D Sample",
            float fov = 45.0f, float near_plane = 0.1f, float far_plane = 1000.0f,
            int width = 1280, int height = 720,
            bool fullscreen = false, bool show_cursor = false,
            int benchmark_length = H3D_DEFAULT_SAMPLE_BENCHMARK_LENGTH );
	virtual ~SampleApplication();

	bool init();


	void release();

    const char *getResourcePath() const { return _resourcePath.c_str(); }
    const char *getTitle() const { return _winTitle.c_str(); }
    void getSize( int &width, int &height ) const;
    H3DRes getPipelineRes() { return _pipelineRes[_curPipeline]; }
    float getFOV() const { return _fov; }
    float getNearPlane() const { return _nearPlane; }
    float getFarPlane() const { return _farPlane; }
	float getStartTime() const
	{
		return std::chrono::duration< float >( _t0.time_since_epoch() ).count();
	}
    float getFPS() const { return _curFPS; }

    int checkFlag(ApplicationFlag flag) const {

        switch (flag)
        {
        case Running:
            return _running;
        case FullScreen:
            return _winFullScreen;
        case CursorVisible:
            return _winShowCursor;
        case CursorInside:
            return _winHasCursor;
        case StatMode:
            return _statMode;
        case FreezeMode:
            return _freezeMode;
        case DebugViewMode:
            return _debugViewMode;
        case WireframeMode:
            return _wireframeMode;
        case BenchmarkMode:
            return _benchmark;
        case BenchmarkLength:
            return _benchmarkLength;
        }

        return 0;
    }

//    bool isKeyDown( int key ) const { return glfwGetKey( _winHandle, key ) == GLFW_PRESS; }

    void setTitle( const char* title );
    void setHelpPanel( int num_rows, char** column1, char** column2 = 0);
	void toggleFullScreen();
    void setPipeline( int pipeline );
    void setSampleCount( int sampleCount );
    void enableDebugViewMode( bool enabled );
    void enableWireframeMode( bool enabled );
    void showCursor( bool visible );
    void showHelpPanel( bool visible );
    void showStatPanel( int mode );
    void setFreezeMode( int mode );

	void setRequiredCapabilities( int caps );

    int run();
	
protected:
//     GLFWwindow* getWindowHandle() const { return _winHandle; }
    int defaultRenderInterface();

	virtual BackendInitParameters setupInitParameters( int render_interface );
	virtual WindowCreateParameters setupWindowParameters();

    virtual bool initResources();
    virtual void releaseResources();

    virtual void update();
    virtual void render();
    virtual void finalize();
    
    virtual void keyEventHandler( int key, int keyState, int mods );
    virtual void mouseMoveHandler( float x, float y, float prev_x, float prev_y );
	virtual void mousePressHandler( int mouseButton, int mouseButtonState, int actionCount );
	virtual void mouseEnterHandler( int entered );
    virtual void touchEventHandler( int evType, int touchPosX, int touchPosY, int fingerID );
    virtual void multiTouchHandler( int touchX, int touchY, float distance, float angle, int prevTouchX, int prevTouchY );

    void setViewportSize( int width, int height );
    void setInvertedMouseMovement( bool invertX, bool invertY );

	static inline void mainLoop( void *arg );

	// Event handlers
	void requestClosing();
	
	FrameworkBackend *getBackend() { return _backend; }
	void *getWindowHandle() { return _winHandle; }
	void recreateWindow();
//	static void keyEventHandler( int key, int scancode, int action, int mods );
//	static void mouseMoveHandler( float x, float y, float prev_x, float prev_y );

private:
	
protected:
    // Camera movement
    float        _x, _y, _z, _rx, _ry;  // Viewer position and orientation
    float        _velocity;  // Velocity for movement

    // Help panel
    int          _helpRows;
    const char** _helpLabels;
    const char** _helpValues;

    // Engine objects
    int          _curPipeline;
    H3DRes       _pipelineRes[3];
    H3DRes       _fontMatRes, _panelMatRes, _logoMatRes;
    H3DNode      _cam;
	int			 _renderInterface;
	int			 _renderCaps;

private:

	FrameworkBackend *_backend = nullptr;

	bool         _initialized;
	bool         _running;
    std::string  _resourcePath;
    bool         _benchmark;
    int          _benchmarkLength;
    float        _curFPS;
	std::chrono::steady_clock::time_point      _t0;

    void		*_winHandle;
	std::string  _winTitle;
    int          _initWinWidth, _initWinHeight;
    int          _winSampleCount, _sampleCount;
    bool         _winFullScreen;
    float        _prevMx, _prevMy;
    bool         _winShowCursor;
    bool         _winHasCursor;

    float        _fov, _nearPlane, _farPlane;

	int          _statMode;
    int          _freezeMode;
    bool         _debugViewMode, _wireframeMode;
    bool         _showHelpPanel;

    bool         _invertMouseX, _invertMouseY;
};

#endif // _sample_app_H_



