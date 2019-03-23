#include "SDLFramework.h"

#include <iostream>

bool SDLBackend::Init( const BackendInitParameters &params )
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
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

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

void SDLBackend::Release()
{
	SDL_Quit();
}

void * SDLBackend::CreateWindow( const WindowCreateParameters &params )
{
	if ( params.fullScreen )
	{
		SDL_DisplayMode display;
		SDL_GetDesktopDisplayMode( 0, &display );

		_wnd = SDL_CreateWindow( params.windowTitle.c_str(), 0, 0, display.w, display.h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN );
	}
	else
	{
		_wnd = SDL_CreateWindow( params.windowTitle.c_str(), 0, 0, params.width, params.height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
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

	return ( void * ) _wnd;
}

bool SDLBackend::DestroyWindow( void *handle )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_DestroyWindow( wnd );

	return true;
}

void SDLBackend::SetWindowTitle( void *handle, const char *title )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_SetWindowTitle( wnd, title );
}

void SDLBackend::SetCursorVisible( void *handle, bool visible )
{
	SDL_ShowCursor( visible );
}

void SDLBackend::GetSize( void *handle, int *width, int *height )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_GetWindowSize( wnd, width, height );
}

void SDLBackend::SwapBuffers( void *handle )
{
	SDL_Window *wnd = ( SDL_Window * ) handle;

	SDL_GL_SwapWindow( wnd );
}

void SDLBackend::ProcessEvents()
{
	// Event handler
	SDL_Event e;

	// Handle events on queue
	while ( SDL_PollEvent( &e ) != 0 )
	{
		switch ( e.type )
		{
			case SDL_MOUSEMOTION:

				break;
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			case SDL_MOUSEWHEEL:
				break;
			case SDL_KEYDOWN:
				break;
			case SDL_KEYUP:
				break;
			case SDL_FINGERDOWN:
				break;
			case SDL_FINGERUP:
				break;
			case SDL_FINGERMOTION:
				break;
			case SDL_QUIT:
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
