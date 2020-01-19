#include "GLFWFramework.h"

#include <iostream>
#include <array>

#include "Horde3DUtils.h"

bool GLFWBackend::init( const BackendInitParameters &params )
{
	if ( !glfwInit() )
	{
		std::cout << "Cannot initialize GLFW! Exiting.";
		return false;
	}

	// Specify surface parameters
	glfwWindowHint( GLFW_RED_BITS, params.redBits );
	glfwWindowHint( GLFW_GREEN_BITS, params.greenBits );
	glfwWindowHint( GLFW_BLUE_BITS, params.blueBits );
	glfwWindowHint( GLFW_ALPHA_BITS, params.alphaBits );
	glfwWindowHint( GLFW_DEPTH_BITS, params.depthBits );
	glfwWindowHint( GLFW_SAMPLES, params.sampleCount );

	// Specify context parameters
	switch ( params.requestedAPI )
	{
		case RenderAPI::OpenGL2:
//			glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );

			break;
		case RenderAPI::OpenGL4:
			glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, params.majorVersion );
			glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, params.minorVersion );

			if ( params.debugContext ) glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_DEBUG_CONTEXT );
			else if ( params.forceGLCompatibilityProfile ) glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
			else glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

			break;
		case RenderAPI::OpenGLES3:
			glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
			glfwWindowHint( GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API );
			glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, params.majorVersion );
			glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, params.minorVersion );

			break;
		default:
			break;
	}

	// Save parameters for resetting/debug info
	_usedInitParams = params;

	return true;
// #ifdef __APPLE__
// 		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
// 		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
// 		glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
// 		glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
// #else

}

void GLFWBackend::release()
{
	glfwTerminate();
}

void * GLFWBackend::createWindow( const WindowCreateParameters &params )
{
	// Create window
	if ( params.fullScreen )
	{
		const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
		_wnd = glfwCreateWindow( mode->width, mode->height, params.windowTitle.c_str(), glfwGetPrimaryMonitor(), NULL );
	}
	else
	{
		_wnd = glfwCreateWindow( params.width, params.height, params.windowTitle.c_str(), NULL, NULL );
	}

	if ( _wnd == nullptr )
	{
		// Fake message box
		glfwDestroyWindow( _wnd );

		std::cout << "Unable to initialize window!" << std::endl;
		std::cout << "Context parameters:" << std::endl;
		std::cout << "API: " << ( _usedInitParams.requestedAPI == RenderAPI::OpenGLES3 ? "OpenGL ES" : "OpenGL" ) << std::endl;
		std::cout << "API version: " << "Major - " << _usedInitParams.majorVersion << "; Minor - " << _usedInitParams.minorVersion << std::endl;
		
		if ( _usedInitParams.requestedAPI != RenderAPI::OpenGLES3 && _usedInitParams.requestedAPI != RenderAPI::OpenGL2 )
			std::cout << "Compatibility profile: " <<  _usedInitParams.forceGLCompatibilityProfile << std::endl;

		std::cout << std::endl << "Surface parameters:" << std::endl;
		std::cout << "Bits: r - " << _usedInitParams.redBits << " g - " << _usedInitParams.greenBits << " b - " << _usedInitParams.blueBits <<
					 " a - " << _usedInitParams.alphaBits << " depth - " << _usedInitParams.depthBits << std::endl;
		std::cout << "Sample count: " << _usedInitParams.sampleCount << std::endl;

		return nullptr;
	}

	glfwSetWindowUserPointer( _wnd, this );
	glfwMakeContextCurrent( _wnd );
	glfwSetInputMode( _wnd, GLFW_STICKY_KEYS, GL_TRUE );

	// Enable/Disable vertical synchronization
	glfwSwapInterval( params.swapInterval );

	// Set listeners
	glfwSetWindowCloseCallback( _wnd, windowCloseListener );
	glfwSetWindowSizeCallback( _wnd, windowResizeListener );
	glfwSetKeyCallback( _wnd, keyPressListener );
	glfwSetCursorPosCallback( _wnd, mouseMoveListener );
	glfwSetCursorEnterCallback( _wnd, mouseEnterListener );
//	glfwSetErrorCallback();

	// Get correct mouse position
	glfwPollEvents();

	double tempX, tempY;
	glfwGetCursorPos( _wnd, &tempX, &tempY );
	_prevMouseX = (float) tempX;
	_prevMouseY = (float) tempY;

	// Init cursor
// 	showCursor( _winShowCursor );

	return ( void * ) _wnd;
}

bool GLFWBackend::destroyWindow( void *handle )
{
	GLFWwindow *wnd = ( GLFWwindow * ) handle;
	glfwDestroyWindow( wnd );

	return true;
}

void GLFWBackend::setWindowTitle( void *handle, const char *title )
{
	GLFWwindow *wnd = ( GLFWwindow * ) handle;
	
	glfwSetWindowTitle( wnd, title );
}

void GLFWBackend::setCursorVisible( void *handle, bool visible )
{
	GLFWwindow *wnd = ( GLFWwindow * ) handle;

	glfwSetInputMode( wnd, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
}

void GLFWBackend::swapBuffers( void *handle )
{
	GLFWwindow *wnd = ( GLFWwindow * ) handle;

	glfwSwapBuffers( wnd );
}

void GLFWBackend::processEvents()
{
	glfwPollEvents();
}

void GLFWBackend::getSize( void *handle, int *width, int *height )
{
	GLFWwindow *wnd = ( GLFWwindow * ) handle;

	glfwGetWindowSize( wnd, width, height );
}

void GLFWBackend::windowCloseListener( GLFWwindow* win )
{
	GLFWBackend *device = static_cast< GLFWBackend* >( glfwGetWindowUserPointer( win ) );
	if ( device->_quitEventHandler.isInitialized() ) device->_quitEventHandler.invoke();
}

void GLFWBackend::windowResizeListener( GLFWwindow* win, int width, int height )
{
	GLFWBackend *device = static_cast< GLFWBackend* >( glfwGetWindowUserPointer( win ) );
	if ( device->_windowResizeHandler.isInitialized() ) device->_windowResizeHandler.invoke( width, height );
}

void GLFWBackend::keyPressListener( GLFWwindow* win, int key, int scancode, int action, int mods )
{
	GLFWBackend *device = static_cast< GLFWBackend* >( glfwGetWindowUserPointer( win ) );
	if ( device->_keyEventHandler.isInitialized() ) device->_keyEventHandler.invoke( key, action, mods );
}

void GLFWBackend::mouseMoveListener( GLFWwindow* win, double x, double y )
{
	GLFWBackend *device = static_cast< GLFWBackend* >( glfwGetWindowUserPointer( win ) );
	if ( device->_mouseMoveEventHandler.isInitialized() ) device->_mouseMoveEventHandler.invoke( ( float ) x, ( float ) y, 
														  device->_prevMouseX, device->_prevMouseY );

	device->_prevMouseX = ( float ) x;
	device->_prevMouseY = ( float ) y;
}

void GLFWBackend::mouseEnterListener( GLFWwindow* win, int entered )
{
	GLFWBackend *device = static_cast< GLFWBackend* >( glfwGetWindowUserPointer( win ) );
	if ( device->_mouseEnterWindowEventHandler.isInitialized() ) device->_mouseEnterWindowEventHandler.invoke( entered );
}

bool GLFWBackend::checkKeyDown( void *handle, int key )
{
	return glfwGetKey( ( GLFWwindow * ) handle, key ) == GLFW_PRESS;
}

void GLFWBackend::logMessage( LogMessageLevel messageLevel, const char *msg )
{
	// As glfw is only used on desktop platforms, standard cout should be enough for most situations
	static std::array< const char *, 4 > messageTypes = { "Error:", "Warning:", "Info:", "Debug:" };

	if ( (int) messageLevel < 0 || (int) messageLevel > 3 ) messageLevel = LogMessageLevel::Info;

	printf( "%s %s\n", messageTypes[ (int) messageLevel ], msg );
}

bool GLFWBackend::loadResources( const char *contentDir )
{
	return h3dutLoadResourcesFromDisk( contentDir );
}