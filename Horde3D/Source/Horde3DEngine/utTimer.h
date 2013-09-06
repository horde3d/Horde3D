// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _utTimer_H_
#define _utTimer_H_

#include "utPlatform.h"

#if defined( PLATFORM_WIN ) || defined( PLATFORM_WIN_CE )
#   define WIN32_LEAN_AND_MEAN 1
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#   include <windows.h>
#else
#	include <sys/time.h>
#endif


namespace Horde3D {

class Timer
{
public:

	Timer() : _elapsedTime( 0 ), _enabled( false )
	{
	#if defined( PLATFORM_WIN ) 
		// Find first available CPU
		DWORD_PTR procMask, sysMask;
		GetProcessAffinityMask( GetCurrentProcess(), &procMask, &sysMask );
		_affMask = 1;
		while( (_affMask & procMask) == 0 ) _affMask <<= 1;
		
		// Get timer frequency
		DWORD_PTR threadAffMask = SetThreadAffinityMask( GetCurrentThread(), _affMask );
		QueryPerformanceFrequency( &_timerFreq );
		SetThreadAffinityMask( GetCurrentThread(), threadAffMask );
	#elif defined( PLATFORM_WIN_CE )
		QueryPerformanceFrequency( &_timerFreq );
	#endif
	}
	
	void setEnabled( bool enabled )
	{	
		if( enabled && !_enabled )
		{
			_startTime = getTime();
			_enabled = true;
		}
		else if( !enabled && _enabled )
		{
			double endTime = getTime();
			_elapsedTime += endTime - _startTime;
			_enabled = false;
		}
	}

	void reset()
	{
		_elapsedTime = 0;
		if( _enabled ) _startTime = getTime();
	}
	
	float getElapsedTimeMS()
	{
		if( _enabled )
		{
			double endTime = getTime();
			_elapsedTime += endTime - _startTime;
			_startTime = endTime;
		}

		return (float)_elapsedTime;
	}

protected:

	double getTime()
	{
	#if defined( PLATFORM_WIN ) || defined( PLATFORM_WIN_CE )
		// Make sure that time is read from the same CPU
		DWORD_PTR threadAffMask = SetThreadAffinityMask( GetCurrentThread(), _affMask );
		
		// Read high performance counter
		LARGE_INTEGER curTick;
		QueryPerformanceCounter( &curTick );

		// Restore affinity mask
		SetThreadAffinityMask( GetCurrentThread(), threadAffMask );

		return (double)curTick.QuadPart / (double)_timerFreq.QuadPart * 1000.0;
	#else
		timeval tv;
		gettimeofday( &tv, 0x0 );
		return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
	#endif
	}

protected:

	double         _startTime;
	double         _elapsedTime;

#if defined( PLATFORM_WIN ) || defined( PLATFORM_WIN_CE )
	LARGE_INTEGER  _timerFreq;
	DWORD_PTR      _affMask;
#endif

	bool           _enabled;
};

}
#endif  // _utTimer_H_
