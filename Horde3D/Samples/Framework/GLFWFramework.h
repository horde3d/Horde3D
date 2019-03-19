#pragma once

#include "FrameworkBackend.h"

#include "GLFW/glfw3.h"

class GLFWBackend : public FrameworkBackend
{
public:

	virtual bool Init( const BackendInitParameters &params ) override;

	virtual void Release() override;

	virtual void * CreateWindow( const WindowCreateParameters &params ) override;

	virtual bool DestroyWindow( void *handle ) override;

	virtual void Update() override;

	virtual void SetWindowTitle( void *handle, const std::string &title ) override;

	virtual void SetCursorVisible( void *handle, bool visible ) override;

	virtual void SwapBuffers( void *handle ) override;

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

};