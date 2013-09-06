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

#ifndef _egCom_H_
#define _egCom_H_

#include "egPrerequisites.h"
#include <string>
#include <queue>
#include <cstdarg>
#include "utTimer.h"


namespace Horde3D {

class GPUTimer;

// =================================================================================================
// Engine Config
// =================================================================================================

struct EngineOptions
{
	enum List
	{
		MaxLogLevel = 1,
		MaxNumMessages,
		TrilinearFiltering,
		MaxAnisotropy,
		TexCompression,
		SRGBLinearization,
		LoadTextures,
		FastAnimation,
		ShadowMapSize,
		SampleCount,
		WireframeMode,
		DebugViewMode,
		DumpFailedShaders,
		GatherTimeStats
	};
};

// =================================================================================================

class EngineConfig
{
public:
	EngineConfig();

	float getOption( EngineOptions::List param );
	bool setOption( EngineOptions::List param, float value );

public:
	int   maxLogLevel;
	int   maxAnisotropy;
	int   shadowMapSize;
	int   sampleCount;
	bool  texCompression;
	bool  sRGBLinearization;
	bool  loadTextures;
	bool  fastAnimation;
	bool  trilinearFiltering;
	bool  wireframeMode;
	bool  debugViewMode;
	bool  dumpFailedShaders;
	bool  gatherTimeStats;
};


// =================================================================================================
// Engine Log
// =================================================================================================

struct LogMessage
{
	std::string  text;
	int          level;
	float        time;

	LogMessage()
	{
	}

	LogMessage( const std::string &text, int level, float time ) :
		text( text ), level( level ), time( time )
	{
	}
};

// =================================================================================================

class EngineLog
{
public:
	EngineLog();

	void writeError( const char *msg, ... );
	void writeWarning( const char *msg, ... );
	void writeInfo( const char *msg, ... );
	void writeDebugInfo( const char *msg, ... );

	bool getMessage( LogMessage &msg );

	uint32 getMaxNumMessages() { return _maxNumMessages; }
	void setMaxNumMessages( uint32 maxNumMessages ) { _maxNumMessages = maxNumMessages; }
	
protected:
	void pushMessage( const std::string &text, uint32 level );
	void pushMessage( int level, const char *msg, va_list ap );

protected:
	Timer                     _timer;
	char                      _textBuf[2048];
	uint32                    _maxNumMessages;
	std::queue< LogMessage >  _messages;
};


// =================================================================================================
// Engine Stats
// =================================================================================================

struct EngineStats
{
	enum List
	{
		TriCount = 100,
		BatchCount,
		LightPassCount,
		FrameTime,
		AnimationTime,
		GeoUpdateTime,
		ParticleSimTime,
		FwdLightsGPUTime,
		DefLightsGPUTime,
		ShadowsGPUTime,
		ParticleGPUTime,
		TextureVMem,
		GeometryVMem
	};
};

// =================================================================================================

class StatManager
{
public:
	StatManager();
	~StatManager();
	
	float getStat( int param, bool reset );
	void incStat( int param, float value );
	Timer *getTimer( int param );
	GPUTimer *getGPUTimer( int param );

protected:
	uint32    _statTriCount;
	uint32    _statBatchCount;
	uint32    _statLightPassCount;

	Timer     _frameTimer;
	Timer     _animTimer;
	Timer     _geoUpdateTimer;
	Timer     _particleSimTimer;
	float     _frameTime;

	GPUTimer  *_fwdLightsGPUTimer;
	GPUTimer  *_defLightsGPUTimer;
	GPUTimer  *_shadowsGPUTimer;
	GPUTimer  *_particleGPUTimer;

	friend class ProfSample;
};

}
#endif // _egCom_H_
