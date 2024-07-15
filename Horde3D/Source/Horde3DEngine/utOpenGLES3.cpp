// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2018 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

//#define _CRT_SECURE_NO_WARNINGS
#include "utOpenGLES3.h"
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(PLATFORM_WIN)
#include "EGL/egl.h"
#endif

#if defined(PLATFORM_ANDROID)
#include <EGL/egl.h>
#endif
#if defined(PLATFORM_LINUX)
#include "EGL/egl.h"
#endif
#if defined (PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#endif

using namespace h3dGLES;

namespace glESExt
{
	bool EXT_texture_filter_anisotropic = false;
	bool EXT_disjoint_timer_query = false;
	
	bool  EXT_color_buffer_float = false;

	bool EXT_geometry_shader = false;
	bool EXT_tessellation_shader = false;

	bool EXT_texture_compression_s3tc = false;
	bool EXT_texture_compression_dxt1 = false;
	bool EXT_texture_compression_bptc = false;
	bool KHR_texture_compression_astc = false;

	bool OES_compressed_ETC1_RGB8_texture = false;

	bool EXT_texture_border_clamp = false;
	bool OES_texture_3D = false;

	bool OES_EGL_image_external = false;
	bool KHR_debug = false;
	
	int	majorVersion = 1, minorVersion = 0;
}



namespace h3dGLES
{
	PFNGLQUERYCOUNTEREXTPROC glQueryCounterEXT = 0x0;
	PFNGLGETQUERYOBJECTIVEXTPROC glGetQueryObjectivEXT = 0x0;
	PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT = 0x0;

	PFNGLTEXPARAMETERIIVEXTPROC glTexParameterIivEXT = 0x0;
	PFNGLTEXPARAMETERIUIVEXTPROC glTexParameterIuivEXT = 0x0;
	PFNGLGETTEXPARAMETERIIVEXTPROC glGetTexParameterIivEXT = 0x0;
	PFNGLGETTEXPARAMETERIUIVEXTPROC glGetTexParameterIuivEXT = 0x0;
	PFNGLSAMPLERPARAMETERIIVEXTPROC glSamplerParameterIivEXT = 0x0;
	PFNGLSAMPLERPARAMETERIUIVEXTPROC glSamplerParameterIuivEXT = 0x0;
	PFNGLGETSAMPLERPARAMETERIIVEXTPROC glGetSamplerParameterIivEXT = 0x0;
	PFNGLGETSAMPLERPARAMETERIUIVEXTPROC glGetSamplerParameterIuivEXT = 0x0;

	PFNGLFRAMEBUFFERTEXTURE3DOESPROC glFramebufferTexture3DOES = 0x0;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = 0x0;

    PFNGLPATCHPARAMETERIOESPROC glPatchParameteri = 0x0;
#ifndef __EMSCRIPTEN__
    PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = 0x0;
    PFNGLMEMORYBARRIERPROC glMemoryBarrier = 0x0;

    PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex = 0x0;
#endif
    PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceiv = 0x0;

	PFNGLDEBUGMESSAGECONTROLKHRPROC glDebugMessageControlKHR = 0x0;
	PFNGLDEBUGMESSAGEINSERTKHRPROC glDebugMessageInsertKHR = 0x0;
	PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR = 0x0;
	PFNGLGETDEBUGMESSAGELOGKHRPROC glGetDebugMessageLogKHR = 0x0;

}  // namespace h3dGLES


bool checkExtensionSupported( const char *extName )
{
	const char *extensions = ( char * ) glGetString( GL_EXTENSIONS );
	size_t nameLen = strlen( extName );
	const char *pos;
	while ( ( pos = strstr( extensions, extName ) ) != 0x0 )
	{
		char c = pos[ nameLen ];
		if ( c == ' ' || c == '\0' ) return true;
		extensions = pos + nameLen;
	}

	return false;
}


void getOpenGLESVersion()
{
	std::string version = ( char * ) glGetString( GL_VERSION );

	size_t pos1 = version.find( "." );
	size_t pos2 = version.find( ".", pos1 + 1 );
	if ( pos2 == std::string::npos ) pos2 = version.find( " ", pos1 + 1 );
	if ( pos2 == std::string::npos ) pos2 = version.length();
	size_t pos0 = version.rfind( " ", pos1 );
	if ( pos0 == std::string::npos ) pos0 = -1;

	glESExt::majorVersion = atoi( version.substr( pos0 + 1, pos1 ).c_str() );
	glESExt::minorVersion = atoi( version.substr( pos1 + 1, pos2 ).c_str() );
}

#ifdef GL_GLEXT_PROTOTYPES

#define platformGetProcAddress(funcName) ((void*)0x0)

#else

void *platformGetProcAddress( const char *funcName )
{
#if defined( PLATFORM_WIN ) || defined( PLATFORM_LINUX ) || defined( PLATFORM_ANDROID )
	return ( void * ) eglGetProcAddress( funcName );
#else
	return ( void* ) 0x0;
#endif
}

#endif

bool initOpenGLExtensions()
{
	bool r = true;

	getOpenGLESVersion();

	// Extensions
	glESExt::EXT_texture_filter_anisotropic = checkExtensionSupported( "GL_EXT_texture_filter_anisotropic" );
	glESExt::EXT_texture_compression_s3tc = checkExtensionSupported( "GL_EXT_texture_compression_s3tc" );
	glESExt::EXT_texture_compression_dxt1 = checkExtensionSupported( "GL_EXT_texture_compression_dxt1" );
	glESExt::EXT_texture_compression_bptc = checkExtensionSupported( "GL_EXT_texture_compression_bptc" );
	glESExt::KHR_texture_compression_astc = checkExtensionSupported( "GL_KHR_texture_compression_astc_ldr" );

	glESExt::KHR_debug = checkExtensionSupported( "GL_KHR_debug" );
	if ( glESExt::KHR_debug )
	{
		r &= ( glDebugMessageCallbackKHR = ( PFNGLDEBUGMESSAGECALLBACKKHRPROC ) platformGetProcAddress( "glDebugMessageCallbackKHR" ) ) != 0x0;
		r &= ( glDebugMessageControlKHR = ( PFNGLDEBUGMESSAGECONTROLKHRPROC ) platformGetProcAddress( "glDebugMessageControlKHR" ) ) != 0x0;
		r &= ( glDebugMessageInsertKHR = ( PFNGLDEBUGMESSAGEINSERTKHRPROC ) platformGetProcAddress( "glDebugMessageInsertKHR" ) ) != 0x0;
		r &= ( glGetDebugMessageLogKHR = ( PFNGLGETDEBUGMESSAGELOGKHRPROC ) platformGetProcAddress( "glGetDebugMessageLogKHR" ) ) != 0x0;
	}

	glESExt::EXT_disjoint_timer_query = checkExtensionSupported( "GL_EXT_disjoint_timer_query" );
	if ( glESExt::EXT_disjoint_timer_query )
	{
		r &= ( glQueryCounterEXT = ( PFNGLQUERYCOUNTEREXTPROC ) platformGetProcAddress( "glQueryCounterEXT" ) ) != 0x0;
		r &= ( glGetQueryObjectivEXT = ( PFNGLGETQUERYOBJECTIVEXTPROC ) platformGetProcAddress( "glGetQueryObjectivEXT" ) ) != 0x0;
		r &= ( glGetQueryObjectui64vEXT = ( PFNGLGETQUERYOBJECTUI64VEXTPROC ) platformGetProcAddress( "glGetQueryObjectui64vEXT" ) ) != 0x0;
	}

	glESExt::EXT_tessellation_shader = checkExtensionSupported( "GL_EXT_tessellation_shader" );
	if ( glESExt::EXT_tessellation_shader )
	{
		r &= ( glPatchParameteri = ( PFNGLPATCHPARAMETERIOESPROC ) platformGetProcAddress( "glPatchParameteri" ) ) != 0x0;
	}

	glESExt::EXT_color_buffer_float = checkExtensionSupported( "GL_EXT_color_buffer_float" ) || checkExtensionSupported( "GL_EXT_color_buffer_half_float" );
	glESExt::OES_compressed_ETC1_RGB8_texture = checkExtensionSupported( "GL_OES_compressed_ETC1_RGB8_texture" );
	glESExt::EXT_texture_border_clamp = checkExtensionSupported( "GL_EXT_texture_border_clamp" ) || checkExtensionSupported( "GL_OES_texture_border_clamp" );
	// 	if ( glESExt::EXT_texture_border_clamp )
	// 	{
	// 		r &= ( glTexParameterIivEXT = ( PFNGLTEXPARAMETERIIVEXTPROC ) platformGetProcAddress( "glTexParameterIivEXT" ) ) != 0x0;
	// 		r &= ( glTexParameterIuivEXT = ( PFNGLTEXPARAMETERIUIVEXTPROC ) platformGetProcAddress( "glTexParameterIuivEXT" ) ) != 0x0;
	// 		r &= ( glGetTexParameterIivEXT = ( PFNGLGETTEXPARAMETERIIVEXTPROC ) platformGetProcAddress( "glGetTexParameterIivEXT" ) ) != 0x0;
	// 		r &= ( glGetTexParameterIuivEXT = ( PFNGLGETTEXPARAMETERIUIVEXTPROC ) platformGetProcAddress( "glGetTexParameterIuivEXT" ) ) != 0x0;
	// 		r &= ( glSamplerParameterIivEXT = ( PFNGLSAMPLERPARAMETERIIVEXTPROC ) platformGetProcAddress( "glSamplerParameterIivEXT" ) ) != 0x0;
	// 		r &= ( glSamplerParameterIuivEXT = ( PFNGLSAMPLERPARAMETERIUIVEXTPROC ) platformGetProcAddress( "glSamplerParameterIuivEXT" ) ) != 0x0;
	// 		r &= ( glGetSamplerParameterIivEXT = ( PFNGLGETSAMPLERPARAMETERIIVEXTPROC ) platformGetProcAddress( "glGetSamplerParameterIivEXT" ) ) != 0x0;
	// 		r &= ( glGetSamplerParameterIuivEXT = ( PFNGLGETSAMPLERPARAMETERIUIVEXTPROC ) platformGetProcAddress( "glGetSamplerParameterIuivEXT" ) ) != 0x0;
	// 	}

	glESExt::EXT_geometry_shader = checkExtensionSupported( "GL_EXT_geometry_shader" ) || checkExtensionSupported( "GL_OES_geometry_shader" );
	glESExt::OES_texture_3D = checkExtensionSupported( "GL_OES_texture_3D" );
	glESExt::OES_EGL_image_external = checkExtensionSupported( "GL_OES_EGL_image_external" );
	

	// if ( glESExt::OES_texture_3D )
	// {
	// 	r &= ( glFramebufferTexture3DOES = ( PFNGLFRAMEBUFFERTEXTURE3DOESPROC) platformGetProcAddress( "glFramebufferTexture3DOES" ) ) != 0x0;
	// }

	// if( glESExt::OES_EGL_image_external )
	// {
	// 	r &= ( glEGLImageTargetTexture2DOES = ( PFNGLEGLIMAGETARGETTEXTURE2DOESPROC ) platformGetProcAddress( "glEGLImageTargetTexture2DOES" ) ) != 0x0;
	// }

	return r;
}
