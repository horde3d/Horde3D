#pragma once

#include "FrameworkBackend.h"
#include "SDL.h"

class SDLBackend : public FrameworkBackend
{

	virtual bool init( const BackendInitParameters &params ) override;

	virtual void release() override;

	virtual void * createWindow( const WindowCreateParameters &params ) override;

	virtual bool destroyWindow( void *handle ) override;

	virtual void setWindowTitle( void *handle, const char *title ) override;

	virtual void setCursorVisible( void *handle, bool visible ) override;

	virtual void swapBuffers( void *handle ) override;

	virtual void processEvents() override;

	virtual void getSize( void *handle, int *width, int *height ) override;

	virtual bool checkKeyDown( void *handle, int key ) override;

	virtual void logMessage( LogMessageLevel messageLevel, const char *msg ) override;

	virtual bool loadResources( const char *contentDir ) override;

protected:

	BackendInitParameters	_usedInitParams;
	SDL_Window				*_wnd = nullptr;
	SDL_GLContext			_ctx = nullptr;

	float					_prevMouseX = 0;
	float					_prevMouseY = 0;
};