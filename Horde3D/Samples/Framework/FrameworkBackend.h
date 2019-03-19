#pragma once

#include <string>

enum class RenderAPI
{
	OpenGL2,
	OpenGL4,
	OpenGLES3
};

struct BackendInitParameters
{
	/* Surface parameters */
	uint32_t redBits = 8;
	uint32_t greenBits = 8;
	uint32_t blueBits = 8;
	uint32_t depthBits = 24;
	uint32_t alphaBits = 8;
	uint32_t sampleCount = 0;

	/* Requested render API parameters */
	RenderAPI requestedAPI = RenderAPI::OpenGL4;
	uint32_t majorVersion = 4;
	uint32_t minorVersion = 3;

	/* Misc. parameters */
	bool forceGLCompatibilityProfile = false;
	bool debugContext = false;
};

struct WindowCreateParameters
{
	std::string windowTitle;

	uint32_t width = 640;
	uint32_t height = 480;

	uint32_t swapInterval = 0;

	bool fullScreen = false;
};

// Event callbacks
typedef void *( *KeyboardEventCallBack )( int key, int keyState, int modifiers );
typedef void *( *MouseMoveEventCallBack )( float x, float y, float prev_x, float prev_y );
typedef void *( *MouseButtonEventCallBack )( int mouseButton, int mouseButtonState, int actionCount );
typedef void *( *MouseWheelEventCallBack )( int wheelX, int wheelY, int wheelXPrev, int wheelYPrev );
typedef void *( *MouseEnterWindowEventCallBack )( bool entered );
typedef void *( *TouchEventCallBack )( int evType, float x, float y, float dx, float dy );
typedef void *( *WindowResizeCallback )( int width, int height );
typedef void *( *QuitEventCallBack )();

class FrameworkBackend
{
public:

	virtual bool Init( const BackendInitParameters &params ) = 0;

	virtual void Release() = 0;

	virtual void *CreateWindow( const WindowCreateParameters &params ) = 0;

	virtual bool DestroyWindow( void *handle ) = 0;

	virtual void SetWindowTitle( void *handle, const std::string &title ) = 0;

	virtual void SetCursorVisible( void *handle, bool visible ) = 0;

	virtual void SwapBuffers( void *handle ) = 0;

	virtual void Update() = 0;

	// Callbacks
	void RegisterKeyboardEventHandler( KeyboardEventCallBack f ) { _keyEventHandler = f; }

	void RegisterMouseMoveEventHandler( MouseMoveEventCallBack f ) { _mouseMoveEventHandler = f; };

	void RegisterMouseButtonEventHandler( MouseButtonEventCallBack f ) { _mouseButtonEventHandler = f; };

	void RegisterMouseWheelEventHandler( MouseWheelEventCallBack f ) { _mouseWheelEventHandler = f; };

	void RegisterMouseEnterWindowEventHandler( MouseEnterWindowEventCallBack f ) { _mouseEnterWindowEventHandler = f; };

	void RegisterTouchEventHandler( TouchEventCallBack f ) { _touchEventHandler = f; };

	void RegisterWindowResizeEventHandler( WindowResizeCallback f ) { _windowResizeHandler = f; };

	void RegisterQuitEventHandler( QuitEventCallBack f ) { _quitEventHandler = f; };

protected:

	// Event handlers
	KeyboardEventCallBack		_keyEventHandler = nullptr;
	MouseMoveEventCallBack		_mouseMoveEventHandler = nullptr;
	MouseButtonEventCallBack	_mouseButtonEventHandler = nullptr;
	MouseWheelEventCallBack		_mouseWheelEventHandler = nullptr;
	MouseEnterWindowEventCallBack _mouseEnterWindowEventHandler = nullptr;
	TouchEventCallBack			_touchEventHandler = nullptr;
	WindowResizeCallback		_windowResizeHandler = nullptr;
	QuitEventCallBack			_quitEventHandler = nullptr;

};