#pragma once

#include "FrameworkBackend.h"

#include "GLFW/glfw3.h"

class GLFWBackend : public FrameworkBackend
{
public:

	virtual bool init( const BackendInitParameters &params ) override;

	virtual void release() override;

	virtual void *createWindow( const WindowCreateParameters &params ) override;

	virtual bool destroyWindow( void *handle ) override;

	virtual void processEvents() override;

	virtual void setWindowTitle( void *handle, const char *title ) override;

	virtual void setCursorVisible( void *handle, bool visible ) override;

	virtual void swapBuffers( void *handle ) override;

	virtual void getSize( void *handle, int *width, int *height ) override;

	virtual bool checkKeyDown( void *handle, int key ) override;

	virtual void logMessage( LogMessageLevel messageLevel, const char *msg ) override;

	virtual bool loadResources( const char *contentDir ) override;

protected:
	
	// GLFW listeners.
	static void windowCloseListener( GLFWwindow* win );
	static void windowResizeListener( GLFWwindow* win, int width, int height );
	static void keyPressListener( GLFWwindow* win, int key, int scancode, int action, int mods );
	static void mouseMoveListener( GLFWwindow* win, double x, double y );
	static void mouseEnterListener( GLFWwindow* win, int entered );



protected:

	BackendInitParameters	_usedInitParams;
	GLFWwindow				*_wnd = nullptr;

	float					_prevMouseX = 0;
	float					_prevMouseY = 0;
};