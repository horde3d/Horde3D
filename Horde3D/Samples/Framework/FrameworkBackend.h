#pragma once

#include <string>

// Keyboard keys (from GLFW)

/*! @brief The key or mouse button was released.
*
*  The key or mouse button was released.
*
*/
#define KEY_RELEASE                0
/*! @brief The key or mouse button was pressed.
*
*  The key or mouse button was pressed.
*
*/
#define KEY_PRESS                  1
/*! @brief The key was held down until it repeated.
*
*  The key was held down until it repeated.
*
*/
#define KEY_REPEAT                 2

/* The unknown key */
#define KEY_UNKNOWN            -1

/* Printable keys */
#define KEY_SPACE              32
#define KEY_APOSTROPHE         39  /* ' */
#define KEY_COMMA              44  /* , */
#define KEY_MINUS              45  /* - */
#define KEY_PERIOD             46  /* . */
#define KEY_SLASH              47  /* / */
#define KEY_0                  48
#define KEY_1                  49
#define KEY_2                  50
#define KEY_3                  51
#define KEY_4                  52
#define KEY_5                  53
#define KEY_6                  54
#define KEY_7                  55
#define KEY_8                  56
#define KEY_9                  57
#define KEY_SEMICOLON          59  /* ; */
#define KEY_EQUAL              61  /* = */
#define KEY_A                  65
#define KEY_B                  66
#define KEY_C                  67
#define KEY_D                  68
#define KEY_E                  69
#define KEY_F                  70
#define KEY_G                  71
#define KEY_H                  72
#define KEY_I                  73
#define KEY_J                  74
#define KEY_K                  75
#define KEY_L                  76
#define KEY_M                  77
#define KEY_N                  78
#define KEY_O                  79
#define KEY_P                  80
#define KEY_Q                  81
#define KEY_R                  82
#define KEY_S                  83
#define KEY_T                  84
#define KEY_U                  85
#define KEY_V                  86
#define KEY_W                  87
#define KEY_X                  88
#define KEY_Y                  89
#define KEY_Z                  90
#define KEY_LEFT_BRACKET       91  /* [ */
#define KEY_BACKSLASH          92  /* \ */
#define KEY_RIGHT_BRACKET      93  /* ] */
#define KEY_GRAVE_ACCENT       96  /* ` */
#define KEY_WORLD_1            161 /* non-US #1 */
#define KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define KEY_ESCAPE             256
#define KEY_ENTER              257
#define KEY_TAB                258
#define KEY_BACKSPACE          259
#define KEY_INSERT             260
#define KEY_DELETE             261
#define KEY_RIGHT              262
#define KEY_LEFT               263
#define KEY_DOWN               264
#define KEY_UP                 265
#define KEY_PAGE_UP            266
#define KEY_PAGE_DOWN          267
#define KEY_HOME               268
#define KEY_END                269
#define KEY_CAPS_LOCK          280
#define KEY_SCROLL_LOCK        281
#define KEY_NUM_LOCK           282
#define KEY_PRINT_SCREEN       283
#define KEY_PAUSE              284
#define KEY_F1                 290
#define KEY_F2                 291
#define KEY_F3                 292
#define KEY_F4                 293
#define KEY_F5                 294
#define KEY_F6                 295
#define KEY_F7                 296
#define KEY_F8                 297
#define KEY_F9                 298
#define KEY_F10                299
#define KEY_F11                300
#define KEY_F12                301
#define KEY_F13                302
#define KEY_F14                303
#define KEY_F15                304
#define KEY_F16                305
#define KEY_F17                306
#define KEY_F18                307
#define KEY_F19                308
#define KEY_F20                309
#define KEY_F21                310
#define KEY_F22                311
#define KEY_F23                312
#define KEY_F24                313
#define KEY_F25                314
#define KEY_KP_0               320
#define KEY_KP_1               321
#define KEY_KP_2               322
#define KEY_KP_3               323
#define KEY_KP_4               324
#define KEY_KP_5               325
#define KEY_KP_6               326
#define KEY_KP_7               327
#define KEY_KP_8               328
#define KEY_KP_9               329
#define KEY_KP_DECIMAL         330
#define KEY_KP_DIVIDE          331
#define KEY_KP_MULTIPLY        332
#define KEY_KP_SUBTRACT        333
#define KEY_KP_ADD             334
#define KEY_KP_ENTER           335
#define KEY_KP_EQUAL           336
#define KEY_LEFT_SHIFT         340
#define KEY_LEFT_CONTROL       341
#define KEY_LEFT_ALT           342
#define KEY_LEFT_SUPER         343
#define KEY_RIGHT_SHIFT        344
#define KEY_RIGHT_CONTROL      345
#define KEY_RIGHT_ALT          346
#define KEY_RIGHT_SUPER        347
#define KEY_MENU               348

#define KEY_LAST               KEY_MENU

// Mouse buttons
#define MOUSE_RELEASE			0

#define MOUSE_PRESS				1

//
// Delegate interface inspired by works of marcmo (github.com/marcmo/delegates) 
// and Stefan Reinalter (https://blog.molecular-matters.com/)
//
template <typename T>
class Delegate {};

template <typename R, typename... Params>
class Delegate< R( Params... ) >
{
	typedef void* instancePtr;
	typedef R( *internalFunction ) ( void *instance, Params... );
	typedef std::pair< void *, internalFunction > methodStub;
	bool initialized;

	template <class C, R( C::*func )( Params... ) >
	static inline R classMethodStub( void* instance, Params... args )
	{
		return ( static_cast< C* >( instance )->*func )( args... );
	}

public:

	Delegate() : stub( nullptr, nullptr ), initialized( false )
	{
	}

	inline bool isInitialized() { return initialized; }

	template <class C, R( C::*func )( Params... ) >
	void bind( C *instance )
	{
		stub.first = instance;
		stub.second = &classMethodStub< C, func >;
		initialized = true;
	}

	R invoke( Params... args )
	{
		return stub.second( stub.first, args... );
	}
private:

	methodStub stub;
};


enum class RenderAPI : int
{
	OpenGL2 = 2,
	OpenGL4 = 4,
	OpenGLES3 = 8
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

enum class LogMessageLevel : int
{
	Error = 0,
	Warning,
	Info,
	Debug
};

// Event callbacks
typedef Delegate< void( int, int, int ) > KeyboardEventCallBack; // ( int key, int keyState, int modifiers );
typedef Delegate< void( float, float, float, float ) > MouseMoveEventCallBack; // ( float x, float y, float prev_x, float prev_y );
typedef Delegate< void( int, int, int ) > MouseButtonEventCallBack; // ( int mouseButton, int mouseButtonState, int actionCount );
typedef Delegate< void( int, int, int, int ) > MouseWheelEventCallBack;  // ( int wheelX, int wheelY, int wheelXPrev, int wheelYPrev );
typedef Delegate< void( int ) >  MouseEnterWindowEventCallBack; // ( int entered );
typedef Delegate< void( int, float, float, float, float ) > TouchEventCallBack; // ( int evType, float x, float y, float dx, float dy );
typedef Delegate< void( int, int ) > WindowResizeCallback; // ( int width, int height );
typedef Delegate< void() > QuitEventCallBack;

class FrameworkBackend
{
public:

	virtual ~FrameworkBackend() {};

	virtual bool Init( const BackendInitParameters &params ) = 0;

	virtual void Release() = 0;

	virtual void *CreateWindow( const WindowCreateParameters &params ) = 0;

	virtual bool DestroyWindow( void *handle ) = 0;

	virtual void SwapBuffers( void *handle ) = 0;

	virtual void ProcessEvents() = 0;

	virtual void GetSize( void *handle, int *width, int *height ) = 0;

	virtual void SetWindowTitle( void *handle, const char *title ) = 0;

	virtual void SetCursorVisible( void *handle, bool visible ) = 0;

	virtual bool CheckKeyDown( void *handle, int key ) = 0;

	virtual void LogMessage( LogMessageLevel messageLevel, const char *msg ) = 0;

	virtual bool LoadResources( const char *contentDir ) = 0;
	
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
	KeyboardEventCallBack		_keyEventHandler;
	MouseMoveEventCallBack		_mouseMoveEventHandler;
	MouseButtonEventCallBack	_mouseButtonEventHandler ;
	MouseWheelEventCallBack		_mouseWheelEventHandler;
	MouseEnterWindowEventCallBack _mouseEnterWindowEventHandler;
	TouchEventCallBack			_touchEventHandler;
	WindowResizeCallback		_windowResizeHandler;
	QuitEventCallBack			_quitEventHandler;

};