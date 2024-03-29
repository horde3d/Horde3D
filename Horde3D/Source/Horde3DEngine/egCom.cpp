// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2021 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#include "egCom.h"
#include "utMath.h"
#include "egModules.h"
#include "egRenderer.h"
#include <stdarg.h>
#include <stdio.h>

#include "utDebug.h"


namespace Horde3D {

// *************************************************************************************************
// Class EngineConfig
// *************************************************************************************************

EngineConfig::EngineConfig()
{
	maxLogLevel = 4;
	trilinearFiltering = true;
	maxAnisotropy = 1;
	texCompression = false;
	sRGBLinearization = false;
	loadTextures = true;
	fastAnimation = true;
	shadowMapSize = 1024;
	sampleCount = 0;
	wireframeMode = false;
	debugViewMode = false;
	dumpFailedShaders = false;
	gatherTimeStats = true;
	debugRenderBackend = false;
}


float EngineConfig::getOption( EngineOptions::List param ) const
{
	switch( param )
	{
	case EngineOptions::MaxLogLevel:
		return (float)maxLogLevel;
	case EngineOptions::MaxNumMessages:
		return (float)Modules::log().getMaxNumMessages();
	case EngineOptions::TrilinearFiltering:
		return trilinearFiltering ? 1.0f : 0.0f;
	case EngineOptions::MaxAnisotropy:
		return (float)maxAnisotropy;
	case EngineOptions::TexCompression:
		return texCompression ? 1.0f : 0.0f;
	case EngineOptions::SRGBLinearization:
		return sRGBLinearization ? 1.0f : 0.0f;
	case EngineOptions::LoadTextures:
		return loadTextures ? 1.0f : 0.0f;
	case EngineOptions::FastAnimation:
		return fastAnimation ? 1.0f : 0.0f;
	case EngineOptions::ShadowMapSize:
		return (float)shadowMapSize;
	case EngineOptions::SampleCount:
		return (float)sampleCount;
	case EngineOptions::WireframeMode:
		return wireframeMode ? 1.0f : 0.0f;
	case EngineOptions::DebugViewMode:
		return debugViewMode ? 1.0f : 0.0f;
	case EngineOptions::DumpFailedShaders:
		return dumpFailedShaders ? 1.0f : 0.0f;
	case EngineOptions::GatherTimeStats:
		return gatherTimeStats ? 1.0f : 0.0f;
	case EngineOptions::DebugRenderBackend:
		return debugRenderBackend ? 1.0f : 0.0f;
	default:
		Modules::setError( "Invalid param for h3dGetOption" );
		return Math::NaN;
	}
}


bool EngineConfig::setOption( EngineOptions::List param, float value )
{
	int size;
	
	switch( param )
	{
	case EngineOptions::MaxLogLevel:
		maxLogLevel = ftoi_r( value );
		return true;
	case EngineOptions::MaxNumMessages:
		Modules::log().setMaxNumMessages( (uint32)ftoi_r( value ) );
		return true;
	case EngineOptions::TrilinearFiltering:
		trilinearFiltering = (value != 0);
		return true;
	case EngineOptions::MaxAnisotropy:
		maxAnisotropy = ftoi_r( value );
		return true;
	case EngineOptions::TexCompression:
		texCompression = (value != 0);
		return true;
	case EngineOptions::SRGBLinearization:
		sRGBLinearization = (value != 0);
		return true;
	case EngineOptions::LoadTextures:
		loadTextures = (value != 0);
		return true;
	case EngineOptions::FastAnimation:
		fastAnimation = (value != 0);
		return true;
	case EngineOptions::ShadowMapSize:
		size = ftoi_r( value );

		if( size == shadowMapSize ) return true;
		if( size != 128 && size != 256 && size != 512 && size != 1024 && size != 2048 ) return false;

		// Update shadow map
		Modules::renderer().releaseShadowRB();
		
		if( !Modules::renderer().createShadowRB( size, size ) )
		{
			Modules::log().writeWarning( "Failed to create shadow map" );
			// Restore old buffer
			Modules::renderer().createShadowRB( shadowMapSize, shadowMapSize );
			return false;
		}
		else
		{
			shadowMapSize = size;
			return true;
		}
	case EngineOptions::SampleCount:
		sampleCount = ftoi_r( value );
		return true;
	case EngineOptions::WireframeMode:
		wireframeMode = (value != 0);
		return true;
	case EngineOptions::DebugViewMode:
		debugViewMode = (value != 0);
		return true;
	case EngineOptions::DumpFailedShaders:
		dumpFailedShaders = (value != 0);
		return true;
	case EngineOptions::GatherTimeStats:
		gatherTimeStats = (value != 0);
		return true;
	case EngineOptions::DebugRenderBackend:
	{
		debugRenderBackend = ( value != 0 );

		bool result = debugRenderBackend ? Modules::renderer().getRenderDevice()->enableDebugOutput() :
										   Modules::renderer().getRenderDevice()->disableDebugOutput();
		return result;
	}
	default:
		Modules::setError( "Invalid param for h3dSetOption" );
		return false;
	}
}


// *************************************************************************************************
// Class EngineLog
// *************************************************************************************************

EngineLog::MessageCallback EngineLog::_callback = NULL;

void EngineLog::setMessageCallback(MessageCallback f)
{
	_callback = f;
}


EngineLog::EngineLog()
{
	_timer.setEnabled( true );
	_maxNumMessages = 512;
}


void EngineLog::pushMessage( int level, const char *msg, va_list args )
{
	float time = _timer.getElapsedTimeMS() / 1000.0f;

	vsnprintf( _textBuf, 2048, msg, args );

	if( _messages.size() < _maxNumMessages - 1 )
	{
		_messages.push( LogMessage( _textBuf, level, time ) );
	}
	else if( _messages.size() == _maxNumMessages - 1 )
	{
		_messages.push( LogMessage( "Message queue is full", 1, time ) );
	}

	if (_callback) {
		_callback(level, _textBuf);
	}
#if defined( H3D_DEBUGGER_OUTPUT )
	static const char *headers[6] = { "", "  [h3d-err] ", "  [h3d-warn] ", "[h3d] ", "  [h3d-dbg] ", "[h3d- ] "};
#if defined( PLATFORM_WIN )
	OutputDebugStringA( headers[std::min( (uint32)level, (uint32)5 )] );
	OutputDebugStringA( _textBuf );
	OutputDebugString( TEXT("\r\n") );
#elif defined( PLATFORM_ANDROID )
	__android_log_print( ANDROID_LOG_DEBUG, "h3d", "%s%s\n", headers[std::min( (uint32)level, (uint32)5 )], _textBuf );
#else
	fputs( headers[std::min( (uint32)level, (uint32)5 )], stderr );
	fputs( _textBuf, stderr );
	fputs( "\n", stderr );
#endif
#endif
}


void EngineLog::writeError( const char *msg, ... )
{
	if( Modules::config().maxLogLevel < 1 ) return;

	va_list args;
	va_start( args, msg );
	pushMessage( 1, msg, args );
	va_end( args );
}


void EngineLog::writeWarning( const char *msg, ... )
{
	if( Modules::config().maxLogLevel < 2 ) return;

	va_list args;
	va_start( args, msg );
	pushMessage( 2, msg, args );
	va_end( args );
}


void EngineLog::writeInfo( const char *msg, ... )
{
	if( Modules::config().maxLogLevel < 3 ) return;

	va_list args;
	va_start( args, msg );
	pushMessage( 3, msg, args );
	va_end( args );
}


void EngineLog::writeDebugInfo( const char *msg, ... )
{
	if( Modules::config().maxLogLevel < 4 ) return;

	va_list args;
	va_start( args, msg );
	pushMessage( 4, msg, args );
	va_end( args );
}


bool EngineLog::getMessage( LogMessage &msg )
{
	if( !_messages.empty() )
	{
		msg = _messages.front();
		_messages.pop();
		return true;
	}
	else
		return false;
}


// *************************************************************************************************
// Class StatManager
// *************************************************************************************************

StatManager::StatManager() : _fwdLightsGPUTimer( 0 ), _defLightsGPUTimer( 0 ), _shadowsGPUTimer( 0 ), _particleGPUTimer( 0 ),
							 _computeGPUTimer( 0 )
{
	_statTriCount = 0;
	_statBatchCount = 0;
	_statLightPassCount = 0;

	_frameTime = 0;
}


StatManager::~StatManager()
{
	if ( _fwdLightsGPUTimer ) { delete _fwdLightsGPUTimer; _fwdLightsGPUTimer = 0; }
	if ( _defLightsGPUTimer ) { delete _defLightsGPUTimer; _defLightsGPUTimer = 0; }
	if ( _shadowsGPUTimer ) { delete _shadowsGPUTimer; _shadowsGPUTimer = 0; }
	if ( _particleGPUTimer ) { delete _particleGPUTimer; _particleGPUTimer = 0; }
	if ( _computeGPUTimer ) { delete _computeGPUTimer; _computeGPUTimer = 0; }
}


bool StatManager::init()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
	if ( !rdi )
		return false;

	_fwdLightsGPUTimer = rdi->createGPUTimer();
	_defLightsGPUTimer = rdi->createGPUTimer();
	_shadowsGPUTimer = rdi->createGPUTimer();
	_particleGPUTimer = rdi->createGPUTimer();
	_computeGPUTimer = rdi->createGPUTimer();

	return true;
}

float StatManager::getStat( int param, bool reset )
{
	float value;	
	
	switch( param )
	{
	case EngineStats::TriCount:
		value = (float)_statTriCount;
		if( reset ) _statTriCount = 0;
		return value;
	case EngineStats::BatchCount:
		value = (float)_statBatchCount;
		if( reset ) _statBatchCount = 0;
		return value;
	case EngineStats::LightPassCount:
		value = (float)_statLightPassCount;
		if( reset ) _statLightPassCount = 0;
		return value;
	case EngineStats::FrameTime:
		value = _frameTime;
		if( reset ) _frameTime = 0;
		return value;
	case EngineStats::AnimationTime:
		value = _animTimer.getElapsedTimeMS();
		if( reset ) _animTimer.reset();
		return value;
	case EngineStats::GeoUpdateTime:
		value = _geoUpdateTimer.getElapsedTimeMS();
		if( reset ) _geoUpdateTimer.reset();
		return value;
	case EngineStats::ParticleSimTime:
		value = _particleSimTimer.getElapsedTimeMS();
		if( reset ) _particleSimTimer.reset();
		return value;
	case EngineStats::FwdLightsGPUTime:
		value = _fwdLightsGPUTimer->getTimeMS();
		if( reset ) _fwdLightsGPUTimer->reset();
		return value;
	case EngineStats::DefLightsGPUTime:
		value = _defLightsGPUTimer->getTimeMS();
		if( reset ) _defLightsGPUTimer->reset();
		return value;
	case EngineStats::ParticleGPUTime:
		value = _particleGPUTimer->getTimeMS();
		if( reset ) _particleGPUTimer->reset();
		return value;
	case EngineStats::ShadowsGPUTime:
		value = _shadowsGPUTimer->getTimeMS();
		if( reset ) _shadowsGPUTimer->reset();
		return value;
	case EngineStats::TextureVMem:
		return ( Modules::renderer().getRenderDevice()->getTextureMem() / 1024) / 1024.0f;
	case EngineStats::GeometryVMem:
		return ( Modules::renderer().getRenderDevice()->getBufferMem() / 1024 ) / 1024.0f;
	case EngineStats::ComputeGPUTime:
		value = _computeGPUTimer->getTimeMS();
		if ( reset ) _computeGPUTimer->reset();
		return value;
	case EngineStats::CullingTime:
		value = _cullingTimer.getElapsedTimeMS();
		if ( reset ) _cullingTimer.reset();
		return value;
	default:
		Modules::setError( "Invalid param for h3dGetStat" );
		return Math::NaN;
	}
}


void StatManager::incStat( int param, float value )
{
	switch( param )
	{
	case EngineStats::TriCount:
		_statTriCount += ftoi_r( value );
		break;
	case EngineStats::BatchCount:
		_statBatchCount += ftoi_r( value );
		break;
	case EngineStats::LightPassCount:
		_statLightPassCount += ftoi_r( value );
		break;
	case EngineStats::FrameTime:
		_frameTime += value;
		break;
	}
}


Timer *StatManager::getTimer( int param )
{
	switch( param )
	{
	case EngineStats::FrameTime:
		return &_frameTimer;
	case EngineStats::AnimationTime:
		return &_animTimer;
	case EngineStats::GeoUpdateTime:
		return &_geoUpdateTimer;
	case EngineStats::ParticleSimTime:
		return &_particleSimTimer;
	case EngineStats::CullingTime:
		return &_cullingTimer;
	default:
		return 0x0;
	}
}


GPUTimer *StatManager::getGPUTimer( int param ) const
{
	switch( param )
	{
	case EngineStats::FwdLightsGPUTime:
		return _fwdLightsGPUTimer;
	case EngineStats::DefLightsGPUTime:
		return _defLightsGPUTimer;
	case EngineStats::ShadowsGPUTime:
		return _shadowsGPUTimer;
	case EngineStats::ParticleGPUTime:
		return _particleGPUTimer;
	case EngineStats::ComputeGPUTime:
		return _computeGPUTimer;
	default:
		return 0x0;
	}
}


// *************************************************************************************************
// Render device capabilities
// *************************************************************************************************

float getRenderDeviceCapabilities( int param )
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
	if ( !rdi )
	{
		Modules::setError( "Render device not initialized!" );
		return Math::NaN;
	}

	switch ( param )
	{
		case RenderDeviceCapabilities::GeometryShaders:
			return rdi->getCaps().geometryShaders ? 1.0f : 0.0f;
		case RenderDeviceCapabilities::Compute:
			return rdi->getCaps().computeShaders ? 1.0f : 0.0f;
		case RenderDeviceCapabilities::Tessellation:
			return rdi->getCaps().tesselation ? 1.0f : 0.0f;
		case RenderDeviceCapabilities::TextureFloatRenderable:
			return rdi->getCaps().texFloat ? 1.0f : 0.0f;
		case RenderDeviceCapabilities::TextureCompressionASTC:
			return rdi->getCaps().texASTC ? 1.0f : 0.0f;
		case RenderDeviceCapabilities::TextureCompressionBPTC:
			return rdi->getCaps().texBPTC ? 1.0f : 0.0f;
		case RenderDeviceCapabilities::TextureCompressionDXT:
			return rdi->getCaps().texDXT ? 1.0f : 0.0f;
		case RenderDeviceCapabilities::TextureCompressionETC2:
			return rdi->getCaps().texETC2 ? 1.0f : 0.0f;
		default:
			Modules::setError( "Invalid param for h3dGetDeviceCapabilities" );
			return Math::NaN;
	}
}

}  // namespace
