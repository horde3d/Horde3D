// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#ifndef _app_H_
#define _app_H_

#include "Horde3D.h"
#include <sstream>
#include <string>


class Application
{
public:
	Application( const std::string &appPath );
	
	void setKeyState( int key, bool state ) { _prevKeys[key] = _keys[key]; _keys[key] = state; }

	const char *getTitle() { return "Knight - Horde3D Sample"; }
	
	bool init();
	void mainLoop( float fps );
	void release();
	void resize( int width, int height );

	void keyStateHandler();
	void mouseMoveEvent( float dX, float dY );

private:
	bool               _keys[320], _prevKeys[320];
	
	float              _x, _y, _z, _rx, _ry;  // Viewer position and orientation
	float              _velocity;  // Velocity for movement
	float              _curFPS;
	std::stringstream  _text;

	int                _statMode;
	int                _freezeMode;
	bool               _debugViewMode, _wireframeMode;
	float              _animTime, _weight;
	
	// Engine objects
	H3DRes             _fontMatRes, _panelMatRes;
	H3DRes             _pipeRes, _logoMatRes, _hdrPipeRes, _forwardPipeRes;
	H3DNode            _cam, _knight, _particleSys;

	std::string        _contentDir;
};

#endif // _app_H_
