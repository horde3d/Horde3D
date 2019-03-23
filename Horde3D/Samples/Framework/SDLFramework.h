#pragma once

#include "FrameworkBackend.h"
#include "SDL.h"

class SDLBackend : public FrameworkBackend
{

	virtual bool Init( const BackendInitParameters &params ) override;

	virtual void Release() override;

	virtual void * CreateWindow( const WindowCreateParameters &params ) override;

	virtual bool DestroyWindow( void *handle ) override;

	virtual void SetWindowTitle( void *handle, const char *title ) override;

	virtual void SetCursorVisible( void *handle, bool visible ) override;

	virtual void SwapBuffers( void *handle ) override;

	virtual void ProcessEvents() override;

	virtual void GetSize( void *handle, int *width, int *height ) override;

protected:

	BackendInitParameters _usedInitParams;
	SDL_Window * _wnd = nullptr;
	SDL_GLContext _ctx = nullptr;
};