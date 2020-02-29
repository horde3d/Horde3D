#include "SDLFramework.h"

#include <iostream>
#include <unordered_map>
#include <array>
#include <vector>

#include "Horde3D.h"
#include "Horde3DUtils.h"

const std::unordered_map< int, int > keyToSDLKey = {
	{ KEY_SPACE, SDL_SCANCODE_SPACE },
	{ KEY_APOSTROPHE, SDL_SCANCODE_APOSTROPHE },
	{ KEY_COMMA, SDL_SCANCODE_COMMA },
	{ KEY_MINUS, SDL_SCANCODE_MINUS	},
	{ KEY_PERIOD, SDL_SCANCODE_PERIOD },
	{ KEY_SLASH, SDL_SCANCODE_SLASH },
	{ KEY_0, SDL_SCANCODE_0 },
	{ KEY_1, SDL_SCANCODE_1 },
	{ KEY_2, SDL_SCANCODE_2 },
	{ KEY_3, SDL_SCANCODE_3 },
	{ KEY_4, SDL_SCANCODE_4 },
	{ KEY_5, SDL_SCANCODE_5 },
	{ KEY_6, SDL_SCANCODE_6 },
	{ KEY_7, SDL_SCANCODE_7 },
	{ KEY_8, SDL_SCANCODE_8 },
	{ KEY_9, SDL_SCANCODE_9 },
	{ KEY_SEMICOLON, SDL_SCANCODE_SEMICOLON },
	{ KEY_EQUAL, SDL_SCANCODE_EQUALS },
	{ KEY_A, SDL_SCANCODE_A },
	{ KEY_B, SDL_SCANCODE_B },
	{ KEY_C, SDL_SCANCODE_C },
	{ KEY_D, SDL_SCANCODE_D },
	{ KEY_E, SDL_SCANCODE_E },
	{ KEY_F, SDL_SCANCODE_F },
	{ KEY_G, SDL_SCANCODE_G },
	{ KEY_H, SDL_SCANCODE_H },
	{ KEY_I, SDL_SCANCODE_I },
	{ KEY_J, SDL_SCANCODE_J },
	{ KEY_K, SDL_SCANCODE_K },
	{ KEY_L, SDL_SCANCODE_L },
	{ KEY_M, SDL_SCANCODE_M },
	{ KEY_N, SDL_SCANCODE_N },
	{ KEY_O, SDL_SCANCODE_O },
	{ KEY_P, SDL_SCANCODE_P },
	{ KEY_Q, SDL_SCANCODE_Q },
	{ KEY_R, SDL_SCANCODE_R },
	{ KEY_S, SDL_SCANCODE_S },
	{ KEY_T, SDL_SCANCODE_T },
	{ KEY_U, SDL_SCANCODE_U },
	{ KEY_V, SDL_SCANCODE_V },
	{ KEY_W, SDL_SCANCODE_W },
	{ KEY_X, SDL_SCANCODE_X },
	{ KEY_Y, SDL_SCANCODE_Y },
	{ KEY_Z, SDL_SCANCODE_Z },
	{ KEY_LEFT_BRACKET, SDL_SCANCODE_LEFTBRACKET },
	{ KEY_BACKSLASH, SDL_SCANCODE_BACKSLASH },
	{ KEY_RIGHT_BRACKET, SDL_SCANCODE_RIGHTBRACKET },
	{ KEY_GRAVE_ACCENT, SDL_SCANCODE_GRAVE },
	{ KEY_ESCAPE, SDL_SCANCODE_ESCAPE },
	{ KEY_ENTER, SDL_SCANCODE_RETURN },
	{ KEY_TAB, SDL_SCANCODE_TAB },
	{ KEY_BACKSPACE, SDL_SCANCODE_BACKSPACE },
	{ KEY_INSERT, SDL_SCANCODE_INSERT },
	{ KEY_DELETE, SDL_SCANCODE_DELETE },
	{ KEY_RIGHT, SDL_SCANCODE_RIGHT },
	{ KEY_LEFT, SDL_SCANCODE_LEFT },
	{ KEY_DOWN, SDL_SCANCODE_DOWN },
	{ KEY_UP, SDL_SCANCODE_UP },
	{ KEY_PAGE_UP, SDL_SCANCODE_PAGEUP },
	{ KEY_PAGE_DOWN, SDL_SCANCODE_PAGEDOWN },
	{ KEY_HOME, SDL_SCANCODE_HOME },
	{ KEY_END, SDL_SCANCODE_END },
	{ KEY_CAPS_LOCK, SDL_SCANCODE_CAPSLOCK },
	{ KEY_SCROLL_LOCK, SDL_SCANCODE_SCROLLLOCK },
	{ KEY_NUM_LOCK, SDL_SCANCODE_NUMLOCKCLEAR },
	{ KEY_PRINT_SCREEN, SDL_SCANCODE_PRINTSCREEN },
	{ KEY_PAUSE, SDL_SCANCODE_PAUSE },
	{ KEY_F1, SDL_SCANCODE_F1 },
	{ KEY_F2, SDL_SCANCODE_F2 },
	{ KEY_F3, SDL_SCANCODE_F3 },
	{ KEY_F4, SDL_SCANCODE_F4 },
	{ KEY_F5, SDL_SCANCODE_F5 },
	{ KEY_F6, SDL_SCANCODE_F6 },
	{ KEY_F7, SDL_SCANCODE_F7 },
	{ KEY_F8, SDL_SCANCODE_F8 },
	{ KEY_F9, SDL_SCANCODE_F9 },
	{ KEY_F10, SDL_SCANCODE_F10 },
	{ KEY_F11, SDL_SCANCODE_F11 },
	{ KEY_F12, SDL_SCANCODE_F12 },
	{ KEY_F13, SDL_SCANCODE_F13 },
	{ KEY_F14, SDL_SCANCODE_F14 },
	{ KEY_F15, SDL_SCANCODE_F15 },
	{ KEY_F16, SDL_SCANCODE_F16 },
	{ KEY_F17, SDL_SCANCODE_F17 },
	{ KEY_F18, SDL_SCANCODE_F18 },
	{ KEY_F19, SDL_SCANCODE_F19 },
	{ KEY_F20, SDL_SCANCODE_F20 },
	{ KEY_F21, SDL_SCANCODE_F21 },
	{ KEY_F22, SDL_SCANCODE_F22 },
	{ KEY_F23, SDL_SCANCODE_F23 },
	{ KEY_F24, SDL_SCANCODE_F24 },
	{ KEY_KP_0, SDL_SCANCODE_KP_0 },
	{ KEY_KP_1, SDL_SCANCODE_KP_1 },
	{ KEY_KP_2, SDL_SCANCODE_KP_2 },
	{ KEY_KP_3, SDL_SCANCODE_KP_3 },
	{ KEY_KP_4, SDL_SCANCODE_KP_4 },
	{ KEY_KP_5, SDL_SCANCODE_KP_5 },
	{ KEY_KP_6, SDL_SCANCODE_KP_6 },
	{ KEY_KP_7, SDL_SCANCODE_KP_7 },
	{ KEY_KP_8, SDL_SCANCODE_KP_8 },
	{ KEY_KP_9, SDL_SCANCODE_KP_9 },
	{ KEY_KP_DECIMAL, SDL_SCANCODE_KP_DECIMAL },
	{ KEY_KP_DIVIDE, SDL_SCANCODE_KP_DIVIDE },
	{ KEY_KP_MULTIPLY, SDL_SCANCODE_KP_MULTIPLY },
	{ KEY_KP_SUBTRACT, SDL_SCANCODE_KP_MINUS },
	{ KEY_KP_ADD, SDL_SCANCODE_KP_PLUS },
	{ KEY_KP_ENTER, SDL_SCANCODE_KP_ENTER },
	{ KEY_KP_EQUAL, SDL_SCANCODE_KP_EQUALS },
	{ KEY_LEFT_SHIFT, SDL_SCANCODE_LSHIFT },
	{ KEY_LEFT_CONTROL, SDL_SCANCODE_LCTRL },
	{ KEY_LEFT_ALT, SDL_SCANCODE_LALT },
	{ KEY_LEFT_SUPER, SDL_SCANCODE_LGUI },
	{ KEY_RIGHT_SHIFT, SDL_SCANCODE_RSHIFT },
	{ KEY_RIGHT_CONTROL, SDL_SCANCODE_RCTRL },
	{ KEY_RIGHT_ALT, SDL_SCANCODE_RALT },
	{ KEY_RIGHT_SUPER, SDL_SCANCODE_RGUI },
	{ KEY_MENU, SDL_SCANCODE_MENU },
};

const std::unordered_map< int, int > sdlKeyToKey = {
	{ SDL_SCANCODE_SPACE, KEY_SPACE },
	{ SDL_SCANCODE_APOSTROPHE, KEY_APOSTROPHE },
	{ SDL_SCANCODE_COMMA, KEY_COMMA },
	{ SDL_SCANCODE_MINUS, KEY_MINUS },
	{ SDL_SCANCODE_PERIOD, KEY_PERIOD },
	{ SDL_SCANCODE_SLASH, KEY_SLASH },
	{ SDL_SCANCODE_0, KEY_0 },
	{ SDL_SCANCODE_1, KEY_1 },
	{ SDL_SCANCODE_2, KEY_2 },
	{ SDL_SCANCODE_3, KEY_3 },
	{ SDL_SCANCODE_4, KEY_4 },
	{ SDL_SCANCODE_5, KEY_5 },
	{ SDL_SCANCODE_6, KEY_6 },
	{ SDL_SCANCODE_7, KEY_7 },
	{ SDL_SCANCODE_8, KEY_8 },
	{ SDL_SCANCODE_9, KEY_9 },
	{ SDL_SCANCODE_SEMICOLON, KEY_SEMICOLON },
	{ SDL_SCANCODE_EQUALS, KEY_EQUAL },
	{ SDL_SCANCODE_A, KEY_A },
	{ SDL_SCANCODE_B, KEY_B },
	{ SDL_SCANCODE_C, KEY_C },
	{ SDL_SCANCODE_D, KEY_D },
	{ SDL_SCANCODE_E, KEY_E },
	{ SDL_SCANCODE_F, KEY_F },
	{ SDL_SCANCODE_G, KEY_G },
	{ SDL_SCANCODE_H, KEY_H },
	{ SDL_SCANCODE_I, KEY_I },
	{ SDL_SCANCODE_J, KEY_J },
	{ SDL_SCANCODE_K, KEY_K },
	{ SDL_SCANCODE_L, KEY_L },
	{ SDL_SCANCODE_M, KEY_M },
	{ SDL_SCANCODE_N, KEY_N },
	{ SDL_SCANCODE_O, KEY_O },
	{ SDL_SCANCODE_P, KEY_P },
	{ SDL_SCANCODE_Q, KEY_Q },
	{ SDL_SCANCODE_R, KEY_R },
	{ SDL_SCANCODE_S, KEY_S },
	{ SDL_SCANCODE_T, KEY_T },
	{ SDL_SCANCODE_U, KEY_U },
	{ SDL_SCANCODE_V, KEY_V },
	{ SDL_SCANCODE_W, KEY_W },
	{ SDL_SCANCODE_X, KEY_X },
	{ SDL_SCANCODE_Y, KEY_Y },
	{ SDL_SCANCODE_Z, KEY_Z },
	{ SDL_SCANCODE_LEFTBRACKET, KEY_LEFT_BRACKET },
	{ SDL_SCANCODE_BACKSLASH, KEY_BACKSLASH },
	{ SDL_SCANCODE_RIGHTBRACKET, KEY_RIGHT_BRACKET },
	{ SDL_SCANCODE_GRAVE, KEY_GRAVE_ACCENT },
	{ SDL_SCANCODE_ESCAPE, KEY_ESCAPE },
	{ SDL_SCANCODE_RETURN, KEY_ENTER },
	{ SDL_SCANCODE_TAB, KEY_TAB },
	{ SDL_SCANCODE_BACKSPACE, KEY_BACKSPACE },
	{ SDL_SCANCODE_INSERT, KEY_INSERT },
	{ SDL_SCANCODE_DELETE, KEY_DELETE },
	{ SDL_SCANCODE_RIGHT, KEY_RIGHT },
	{ SDL_SCANCODE_LEFT, KEY_LEFT },
	{ SDL_SCANCODE_DOWN, KEY_DOWN },
	{ SDL_SCANCODE_UP, KEY_UP },
	{ SDL_SCANCODE_PAGEUP, KEY_PAGE_UP },
	{ SDL_SCANCODE_PAGEDOWN, KEY_PAGE_DOWN },
	{ SDL_SCANCODE_HOME, KEY_HOME },
	{ SDL_SCANCODE_END, KEY_END },
	{ SDL_SCANCODE_CAPSLOCK, KEY_CAPS_LOCK },
	{ SDL_SCANCODE_SCROLLLOCK, KEY_SCROLL_LOCK },
	{ SDL_SCANCODE_NUMLOCKCLEAR, KEY_NUM_LOCK },
	{ SDL_SCANCODE_PRINTSCREEN, KEY_PRINT_SCREEN },
	{ SDL_SCANCODE_PAUSE, KEY_PAUSE },
	{ SDL_SCANCODE_F1, KEY_F1 },
	{ SDL_SCANCODE_F2, KEY_F2 },
	{ SDL_SCANCODE_F3, KEY_F3 },
	{ SDL_SCANCODE_F4, KEY_F4 },
	{ SDL_SCANCODE_F5, KEY_F5  },
	{ SDL_SCANCODE_F6, KEY_F6 },
	{ SDL_SCANCODE_F7, KEY_F7 },
	{ SDL_SCANCODE_F8, KEY_F8 },
	{ SDL_SCANCODE_F9, KEY_F9 },
	{ SDL_SCANCODE_F10, KEY_F10 },
	{ SDL_SCANCODE_F11, KEY_F11 },
	{ SDL_SCANCODE_F12, KEY_F12 },
	{ SDL_SCANCODE_F13, KEY_F13 },
	{ SDL_SCANCODE_F14, KEY_F14 },
	{ SDL_SCANCODE_F15, KEY_F15 },
	{ SDL_SCANCODE_F16, KEY_F16 },
	{ SDL_SCANCODE_F17, KEY_F17 },
	{ SDL_SCANCODE_F18, KEY_F18 },
	{ SDL_SCANCODE_F19, KEY_F19 },
	{ SDL_SCANCODE_F20, KEY_F20 },
	{ SDL_SCANCODE_F21, KEY_F21 },
	{ SDL_SCANCODE_F22, KEY_F22 },
	{ SDL_SCANCODE_F23, KEY_F23 },
	{ SDL_SCANCODE_F24, KEY_F24 },
	{ SDL_SCANCODE_KP_0, KEY_KP_0 },
	{ SDL_SCANCODE_KP_1, KEY_KP_1 },
	{ SDL_SCANCODE_KP_2, KEY_KP_2 },
	{ SDL_SCANCODE_KP_3, KEY_KP_3 },
	{ SDL_SCANCODE_KP_4, KEY_KP_4 },
	{ SDL_SCANCODE_KP_5, KEY_KP_5 },
	{ SDL_SCANCODE_KP_6, KEY_KP_6 },
	{ SDL_SCANCODE_KP_7, KEY_KP_7 },
	{ SDL_SCANCODE_KP_8, KEY_KP_8 },
	{ SDL_SCANCODE_KP_9, KEY_KP_9 },
	{ SDL_SCANCODE_KP_DECIMAL, KEY_KP_DECIMAL },
	{ SDL_SCANCODE_KP_DIVIDE, KEY_KP_DIVIDE },
	{ SDL_SCANCODE_KP_MULTIPLY, KEY_KP_MULTIPLY },
	{ SDL_SCANCODE_KP_MINUS, KEY_KP_SUBTRACT }, 
	{ SDL_SCANCODE_KP_PLUS, KEY_KP_ADD },
	{ SDL_SCANCODE_KP_ENTER, KEY_KP_ENTER },
	{ SDL_SCANCODE_KP_EQUALS, KEY_KP_EQUAL },
	{ SDL_SCANCODE_LSHIFT, KEY_LEFT_SHIFT },
	{ SDL_SCANCODE_LCTRL, KEY_LEFT_CONTROL },
	{ SDL_SCANCODE_LALT, KEY_LEFT_ALT },
	{ SDL_SCANCODE_LGUI, KEY_LEFT_SUPER },
	{ SDL_SCANCODE_RSHIFT, KEY_RIGHT_SHIFT },
	{ SDL_SCANCODE_RCTRL, KEY_RIGHT_CONTROL },
	{ SDL_SCANCODE_RALT, KEY_RIGHT_ALT },
	{ SDL_SCANCODE_RGUI, KEY_RIGHT_SUPER },
	{ SDL_SCANCODE_MENU, KEY_MENU },
};

bool loadResourcesBySDL( const char *contentDir )
{
	auto cleanPath = []( std::string &path )
	{
		// Remove spaces at the beginning
		int cnt = 0;
		for( int i = 0; i < (int)path.length(); ++i )
		{
			if( path[i] != ' ' ) break;
			else ++cnt;
		}
		if( cnt > 0 ) path.erase( 0, cnt );

		// Remove slashes, backslashes and spaces at the end
		cnt = 0;
		for( int i = (int)path.length() - 1; i >= 0; --i )
		{
			if( path[i] != '/' && path[i] != '\\' && path[i] != ' ' ) break;
			else ++cnt;
		}

		if( cnt > 0 ) path.erase( path.length() - cnt, cnt );

		return std::move( path );
	};

    bool result = true;
	std::string dir;
	std::vector< std::string > dirs;

	// Split path string
	char *c = (char *)contentDir;
	do
	{
		if( *c != '|' && *c != '\0' )
			dir += *c;
		else
		{
			dir = cleanPath( dir );
			if( dir != "" ) dir += '/';
			dirs.push_back( dir );
			dir = "";
		}
	} while( *c++ != '\0' );
	
	// Get the first resource that needs to be loaded
	int res = h3dQueryUnloadedResource( 0 );
	
	char *dataBuf = 0;
	size_t bufSize = 0;

	while( res != 0 )
	{
		SDL_RWops *inf = nullptr;
		
		// Loop over search paths and try to open files
		for( unsigned int i = 0; i < dirs.size(); ++i )
		{
			std::string fileName = dirs[i] + h3dGetResName( res );
			SDL_LogInfo( 0, "Current file path: %s", fileName.c_str() );

			inf = SDL_RWFromFile( fileName.c_str(), "rb" );
			if( inf ) break;
		}

		// Open resource file
		if( inf ) // Resource file found
		{
			// Find size of resource file
			size_t fileSize = SDL_RWseek( inf, 0, RW_SEEK_END );
			if( bufSize < fileSize  )
			{
				delete[] dataBuf;				
				dataBuf = new char[fileSize];
				if( !dataBuf )
				{
					bufSize = 0;
					continue;
				}
				bufSize = fileSize;
			}
			if( fileSize == 0 )	continue;

			// Copy resource file to memory
			SDL_RWseek( inf, 0, RW_SEEK_SET );
			SDL_RWread( inf, dataBuf, fileSize, 1 );
			SDL_RWclose( inf );

			// Send resource data to engine
			result &= h3dLoadResource( res, dataBuf, ( int ) fileSize );
		}
		else // Resource file not found
		{
			// Tell engine to use the default resource by using NULL as data pointer
			h3dLoadResource( res, 0x0, 0 );
			result = false;
		}
		// Get next unloaded resource
		res = h3dQueryUnloadedResource( 0 );
	}
	delete[] dataBuf;

	return result;
}

bool SDLBackend::init( const BackendInitParameters &params )
{
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// Set parameters for context and window
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, params.redBits );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, params.greenBits );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, params.blueBits );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, params.alphaBits );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, params.depthBits );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, params.sampleCount );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	// Specify context parameters
	switch ( params.requestedAPI )
	{
		case RenderAPI::OpenGL2:
//			SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

			break;
		case RenderAPI::OpenGL4:
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, params.majorVersion );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, params.minorVersion );

			if ( params.debugContext ) SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );
			else if ( params.forceGLCompatibilityProfile ) SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
			else SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

			break;
		case RenderAPI::OpenGLES3:
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, params.majorVersion );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, params.minorVersion );

			break;
		default:
			return false;
	}

	// Save parameters for resetting/debug info
	_usedInitParams = params;

	return true;
}

void SDLBackend::release()
{
	SDL_Quit();
}

void * SDLBackend::createWindow( const WindowCreateParameters &params )
{
	// Additional hints for mobile platforms
	if ( _curPlatform == Platform::Android || _curPlatform == Platform::IOS )
	{
		// Currently force to landscape mode
		SDL_SetHint( SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight" );
		SDL_SetHint( SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH, "1" );
	}

	if ( params.fullScreen )
	{
		SDL_DisplayMode display;
		SDL_GetDesktopDisplayMode( 0, &display );

		_wnd = SDL_CreateWindow( params.windowTitle.c_str(), 0, 0, display.w, display.h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN );
	}
	else
	{
		_wnd = SDL_CreateWindow( params.windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params.width, 
								 params.height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
	}

	if ( !_wnd )
	{
		SDL_DestroyWindow( _wnd );

		std::cout << "Unable to initialize window!" << std::endl;
		std::cout << "Context parameters:" << std::endl;
		std::cout << "API: " << ( _usedInitParams.requestedAPI == RenderAPI::OpenGLES3 ? "OpenGL ES" : "OpenGL" ) << std::endl;
		std::cout << "API version: " << "Major - " << _usedInitParams.majorVersion << "; Minor - " << _usedInitParams.minorVersion << std::endl;

		if ( _usedInitParams.requestedAPI != RenderAPI::OpenGLES3 && _usedInitParams.requestedAPI != RenderAPI::OpenGL2 )
			std::cout << "Compatibility profile: " << _usedInitParams.forceGLCompatibilityProfile << std::endl;

		std::cout << std::endl << "Surface parameters:" << std::endl;
		std::cout << "Bits: r - " << _usedInitParams.redBits << " g - " << _usedInitParams.greenBits << " b - " << _usedInitParams.blueBits <<
			" a - " << _usedInitParams.alphaBits << " depth - " << _usedInitParams.depthBits << std::endl;
		std::cout << "Sample count: " << _usedInitParams.sampleCount << std::endl;

		return nullptr;
	}

	_ctx = SDL_GL_CreateContext( _wnd );
	if ( !_ctx ) return nullptr;

	// Enable/Disable vertical synchronization
	if ( SDL_GL_SetSwapInterval( params.swapInterval ) < 0 )
	{
		std::cout << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
	}

	// Store window size
	_currentWidth = params.width; _currentHeight = params.height;

	// Get correct mouse position
	int tempX, tempY;
	SDL_PumpEvents();
	SDL_GetMouseState( &tempX, &tempY );

	_prevMouseX = tempX; _prevMouseY = tempY;

	return ( void * ) _wnd;
}

bool SDLBackend::destroyWindow( void *handle )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_DestroyWindow( wnd );

	return true;
}

void SDLBackend::setWindowTitle( void *handle, const char *title )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_SetWindowTitle( wnd, title );
}

void SDLBackend::setCursorVisible( void *handle, bool visible )
{
//	SDL_ShowCursor( visible );
	SDL_SetRelativeMouseMode( visible ? SDL_FALSE : SDL_TRUE );
}

void SDLBackend::getSize( void *handle, int *width, int *height )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_GetWindowSize( wnd, width, height );

	_currentWidth = *width; _currentHeight = *height;
}

bool SDLBackend::checkKeyDown( void *handle, int key )
{
	const Uint8 *state = SDL_GetKeyboardState( NULL );
	if ( state[ keyToSDLKey.at( key ) ] ) return true;
	
	return false;
}

void SDLBackend::swapBuffers( void *handle )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_GL_SwapWindow( wnd );
}

void SDLBackend::processEvents()
{
	// Event handler
	SDL_Event e;

	static int prevWheelx = 0;
	static int prevWheely = 0;
	static int touchLocationX = 0;
	static int touchLocationY = 0;
	static int prevTouchLocationX = 0;
	static int prevTouchLocationY = 0;
	static int posX = _prevMouseX;
	static int posY = _prevMouseY;

	// Generic touch handling 
	auto touch = [&]( int touchType )
	{
		touchLocationX = e.tfinger.x * _currentWidth;
		touchLocationY = e.tfinger.y * _currentHeight;

		if ( _touchEventHandler.isInitialized() )
			_touchEventHandler.invoke( touchType, touchLocationX, touchLocationY, (int) e.tfinger.fingerId );

		prevTouchLocationX = touchLocationX;
		prevTouchLocationY = touchLocationY;
	};

	// Handle events on queue
	while ( SDL_PollEvent( &e ) != 0 )
	{
		switch ( e.type )
		{
			case SDL_MOUSEMOTION:
			{
				posX += + e.motion.xrel;
				posY += e.motion.yrel;
				if ( _mouseMoveEventHandler.isInitialized() )
					_mouseMoveEventHandler.invoke( ( float ) posX, ( float ) posY, _prevMouseX, _prevMouseY );

				_prevMouseX = ( float ) posX;
				_prevMouseY = ( float ) posY;
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
				if ( _mouseButtonEventHandler.isInitialized() )
					_mouseButtonEventHandler.invoke( e.button.button, MOUSE_PRESS, e.button.clicks );
				
				break;
			case SDL_MOUSEBUTTONUP:
				if ( _mouseButtonEventHandler.isInitialized() )
					_mouseButtonEventHandler.invoke( e.button.button, MOUSE_RELEASE, e.button.clicks );

				break;
			case SDL_MOUSEWHEEL:
				if ( _mouseWheelEventHandler.isInitialized() )
					_mouseWheelEventHandler.invoke( e.wheel.x, e.wheel.y, prevWheelx, prevWheely );

				prevWheelx = e.wheel.x;
				prevWheely = e.wheel.y;

				break;
			case SDL_KEYDOWN:
// 				const auto modkeys = KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI;
// 				const bool MOD_NONE = !( SDL_GetModState() & modkeys );

				if ( _keyEventHandler.isInitialized() )
					_keyEventHandler.invoke( sdlKeyToKey.at( e.key.keysym.scancode ), KEY_PRESS, e.key.keysym.mod );

				break;
			case SDL_KEYUP:
				if ( _keyEventHandler.isInitialized() )
					_keyEventHandler.invoke( sdlKeyToKey.at( e.key.keysym.scancode ), KEY_RELEASE, e.key.keysym.mod );

				break;
			case SDL_FINGERDOWN:
				touch( (int) TouchEvents::FingerDown );
				break;
			case SDL_FINGERUP:
				touch( (int) TouchEvents::FingerUp );
				break;
			case SDL_FINGERMOTION:
				touch( (int) TouchEvents::FingerMove );
				break;
			case SDL_MULTIGESTURE:
				touchLocationX = e.mgesture.x * _currentWidth;
				touchLocationY = e.mgesture.y * _currentHeight;

				if ( _multiTouchHandler.isInitialized() )
					_multiTouchHandler.invoke( touchLocationX, touchLocationY, e.mgesture.dDist, e.mgesture.dTheta, 
											   prevTouchLocationX, prevTouchLocationY );

				prevTouchLocationX = touchLocationX;
				prevTouchLocationY = touchLocationY;

				break;
			case SDL_QUIT:
				if ( _quitEventHandler.isInitialized() ) _quitEventHandler.invoke();

				break;

			default:
				break;
		}
// 		// User requests quit
// 		if ( e.type == SDL_QUIT )
// 		{
// 			quit = true;
// 		}
// 		// Handle keypress with current mouse position
// 		else if ( e.type == SDL_TEXTINPUT )
// 		{
// 			int x = 0, y = 0;
// 			SDL_GetMouseState( &x, &y );
// 			handleKeys( e.text.text[ 0 ], x, y );
// 		}
	}
}

void SDLBackend::logMessage( LogMessageLevel messageLevel, const char *msg ) 
{
	// SDL allows logging messages on android and ios
	static std::array< const char *, 4 > messageTypes = { "Error:", "Warning:", "Info:", "Debug:" };
	if ( (int) messageLevel < 0 || (int) messageLevel > 3 ) messageLevel = LogMessageLevel::Info;

	SDL_Log( "%s %s\n", messageTypes[ (int) messageLevel ], msg );
}

bool SDLBackend::loadResources( const char *contentDir )
{
#if defined( PLATFORM_ANDROID ) || defined ( PLATFORM_IOS )
    return loadResourcesBySDL( contentDir );
#else
    return h3dutLoadResourcesFromDisk( contentDir );
#endif
}