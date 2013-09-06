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

#define _CRT_SECURE_NO_WARNINGS
#include "utOpenGL.h"
#include <cstdlib>
#include <cstring>


namespace glExt
{
	bool EXT_framebuffer_object = false;
	bool EXT_framebuffer_multisample = false;
	bool EXT_texture_filter_anisotropic = false;
	bool EXT_texture_compression_s3tc = false;
	bool EXT_texture_sRGB = false;
	bool ARB_texture_float = false;
	bool ARB_texture_non_power_of_two = false;
	bool ARB_timer_query = false;

	int	majorVersion = 1, minorVersion = 0;
}


namespace h3dGL
{
// GL 1.2
PFNGLBLENDCOLORPROC glBlendColor = 0x0;
PFNGLBLENDEQUATIONPROC glBlendEquation = 0x0;
PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements = 0x0;
PFNGLTEXIMAGE3DPROC glTexImage3D = 0x0;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = 0x0;
PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D = 0x0;

// GL 1.3
PFNGLACTIVETEXTUREPROC glActiveTexture = 0x0;
PFNGLSAMPLECOVERAGEPROC glSampleCoverage = 0x0;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D = 0x0;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = 0x0;
PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D = 0x0;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage = 0x0;

// GL 1.4
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = 0x0;
PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays = 0x0;
PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements = 0x0;
PFNGLPOINTPARAMETERFPROC glPointParameterf = 0x0;
PFNGLPOINTPARAMETERFVPROC glPointParameterfv = 0x0;
PFNGLPOINTPARAMETERIPROC glPointParameteri = 0x0;
PFNGLPOINTPARAMETERIVPROC glPointParameteriv = 0x0;

// GL 1.5
PFNGLGENQUERIESPROC glGenQueries = 0x0;
PFNGLDELETEQUERIESPROC glDeleteQueries = 0x0;
PFNGLISQUERYPROC glIsQuery = 0x0;
PFNGLBEGINQUERYPROC glBeginQuery = 0x0;
PFNGLENDQUERYPROC glEndQuery = 0x0;
PFNGLGETQUERYIVPROC glGetQueryiv = 0x0;
PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv = 0x0;
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv = 0x0;
PFNGLBINDBUFFERPROC glBindBuffer = 0x0;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = 0x0;
PFNGLGENBUFFERSPROC glGenBuffers = 0x0;
PFNGLISBUFFERPROC glIsBuffer = 0x0;
PFNGLBUFFERDATAPROC glBufferData = 0x0;
PFNGLBUFFERSUBDATAPROC glBufferSubData = 0x0;
PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData = 0x0;
PFNGLMAPBUFFERPROC glMapBuffer = 0x0;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = 0x0;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = 0x0;
PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv = 0x0;

// GL 2.0
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = 0x0;
PFNGLDRAWBUFFERSPROC glDrawBuffers = 0x0;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate = 0x0;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate = 0x0;
PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate = 0x0;
PFNGLATTACHSHADERPROC glAttachShader = 0x0;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = 0x0;
PFNGLCOMPILESHADERPROC glCompileShader = 0x0;
PFNGLCREATEPROGRAMPROC glCreateProgram = 0x0;
PFNGLCREATESHADERPROC glCreateShader = 0x0;
PFNGLDELETEPROGRAMPROC glDeleteProgram = 0x0;
PFNGLDELETESHADERPROC glDeleteShader = 0x0;
PFNGLDETACHSHADERPROC glDetachShader = 0x0;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = 0x0;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = 0x0;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib = 0x0;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = 0x0;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders = 0x0;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = 0x0;
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0x0;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0x0;
PFNGLGETSHADERIVPROC glGetShaderiv = 0x0;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0x0;
PFNGLGETSHADERSOURCEPROC glGetShaderSource = 0x0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0x0;
PFNGLGETUNIFORMFVPROC glGetUniformfv = 0x0;
PFNGLGETUNIFORMIVPROC glGetUniformiv = 0x0;
PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv = 0x0;
PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv = 0x0;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv = 0x0;
PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv = 0x0;
PFNGLISPROGRAMPROC glIsProgram = 0x0;
PFNGLISSHADERPROC glIsShader = 0x0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0x0;
PFNGLSHADERSOURCEPROC glShaderSource = 0x0;
PFNGLUSEPROGRAMPROC glUseProgram = 0x0;
PFNGLUNIFORM1FPROC glUniform1f = 0x0;
PFNGLUNIFORM2FPROC glUniform2f = 0x0;
PFNGLUNIFORM3FPROC glUniform3f = 0x0;
PFNGLUNIFORM4FPROC glUniform4f = 0x0;
PFNGLUNIFORM1IPROC glUniform1i = 0x0;
PFNGLUNIFORM2IPROC glUniform2i = 0x0;
PFNGLUNIFORM3IPROC glUniform3i = 0x0;
PFNGLUNIFORM4IPROC glUniform4i = 0x0;
PFNGLUNIFORM1FVPROC glUniform1fv = 0x0;
PFNGLUNIFORM2FVPROC glUniform2fv = 0x0;
PFNGLUNIFORM3FVPROC glUniform3fv = 0x0;
PFNGLUNIFORM4FVPROC glUniform4fv = 0x0;
PFNGLUNIFORM1IVPROC glUniform1iv = 0x0;
PFNGLUNIFORM2IVPROC glUniform2iv = 0x0;
PFNGLUNIFORM3IVPROC glUniform3iv = 0x0;
PFNGLUNIFORM4IVPROC glUniform4iv = 0x0;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv = 0x0;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = 0x0;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = 0x0;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = 0x0;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = 0x0;

// GL 2.1
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv = 0x0;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv = 0x0;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv = 0x0;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv = 0x0;
PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv = 0x0;
PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv = 0x0;

// GL 3.0
PFNGLGETSTRINGIPROC glGetStringi = 0x0;

// GL_EXT_framebuffer_object
PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT = 0x0;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = 0x0;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = 0x0;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = 0x0;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = 0x0;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT = 0x0;
PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT = 0x0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0x0;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = 0x0;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = 0x0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = 0x0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = 0x0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = 0x0;
PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = 0x0;

// GL_EXT_framebuffer_blit
PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = 0x0;

// GL_EXT_framebuffer_multisample
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = 0x0;

// GL_ARB_timer_query
PFNGLQUERYCOUNTERPROC glQueryCounter = 0x0;
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v = 0x0;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v = 0x0;
}  // namespace h3dGL


bool isExtensionSupported( const char *extName )
{
	if( glExt::majorVersion < 3 )
	{
		const char *extensions = (char *)glGetString( GL_EXTENSIONS );
		size_t nameLen = strlen( extName );
		const char *pos;
		while( ( pos = strstr( extensions, extName ) ) != 0x0 )
		{
			char c = pos[nameLen];
			if( c == ' ' || c == '\0' ) return true;
			extensions = pos + nameLen;
		}
	}
	else
	{
		GLint numExts;
		glGetIntegerv( GL_NUM_EXTENSIONS, &numExts );
		for( int i = 0; i < numExts; ++i )
		{
			if( strcmp( extName, (char *)glGetStringi( GL_EXTENSIONS, i ) ) == 0 )
				return true;
		}
	}

	return false;
}


void getOpenGLVersion()
{
	char version[8];
	size_t len = strlen( (char *)glGetString( GL_VERSION ) );
	if( len >= 8 ) len = 7;
	
	strncpy( version, (char *)glGetString( GL_VERSION ), len );
	version[len] = '\0';

	char *pos1 = strtok( version, "." );
	if( pos1 )
	{
		glExt::majorVersion = atoi( pos1 );
		char *pos2 = strtok( 0x0, ". " );
		if( pos2 ) glExt::minorVersion = atoi( pos2 );
	}
}


void *platGetProcAddress( const char *funcName )
{
#if defined( PLATFORM_WIN )
	return (void *)wglGetProcAddress( funcName );
#elif defined( PLATFORM_WIN_CE )
	return (void *)eglGetProcAddress( funcName );
#elif defined( PLATFORM_MAC )
	CFStringRef functionName = CFStringCreateWithCString( kCFAllocatorDefault, funcName, kCFStringEncodingASCII );
	CFURLRef bundleURL = CFURLCreateWithFileSystemPath(
		kCFAllocatorDefault, CFSTR( "/System/Library/Frameworks/OpenGL.framework" ), kCFURLPOSIXPathStyle, true );
	CFBundleRef bundle = CFBundleCreate( kCFAllocatorDefault, bundleURL );
   
	void *function = CFBundleGetFunctionPointerForName( bundle, functionName );
   
	CFRelease( bundle );
	CFRelease( bundleURL );
	CFRelease( functionName );
   
	return function; 
#else
	return (void *)glXGetProcAddressARB( (const GLubyte *)funcName );
#endif
}


bool initOpenGLExtensions()
{
	bool r = true;
	
	getOpenGLVersion();
	
	// GL 1.2
	r &= (glBlendColor = (PFNGLBLENDCOLORPROC) platGetProcAddress( "glBlendColor" )) != 0x0;
	r &= (glBlendEquation = (PFNGLBLENDEQUATIONPROC) platGetProcAddress( "glBlendEquation" )) != 0x0;
	r &= (glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) platGetProcAddress( "glDrawRangeElements" )) != 0x0;
	r &= (glTexImage3D = (PFNGLTEXIMAGE3DPROC) platGetProcAddress( "glTexImage3D" )) != 0x0;
	r &= (glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) platGetProcAddress( "glTexSubImage3D" )) != 0x0;
	r &= (glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC) platGetProcAddress( "glCopyTexSubImage3D" )) != 0x0;

	// GL 1.3
	r &= (glActiveTexture = (PFNGLACTIVETEXTUREPROC) platGetProcAddress( "glActiveTexture" )) != 0x0;
	r &= (glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) platGetProcAddress( "glSampleCoverage" )) != 0x0;
	r &= (glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC) platGetProcAddress( "glCompressedTexImage3D" )) != 0x0;
	r &= (glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) platGetProcAddress( "glCompressedTexImage2D" )) != 0x0;
	r &= (glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC) platGetProcAddress( "glCompressedTexImage1D" )) != 0x0;
	r &= (glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) platGetProcAddress( "glCompressedTexSubImage3D" )) != 0x0;
	r &= (glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) platGetProcAddress( "glCompressedTexSubImage2D" )) != 0x0;
	r &= (glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) platGetProcAddress( "glCompressedTexSubImage1D" )) != 0x0;
	r &= (glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC) platGetProcAddress( "glGetCompressedTexImage" )) != 0x0;
	
	// GL 1.4
	r &= (glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) platGetProcAddress( "glBlendFuncSeparate" )) != 0x0;
	r &= (glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC) platGetProcAddress( "glMultiDrawArrays" )) != 0x0;
	r &= (glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC) platGetProcAddress( "glMultiDrawElements" )) != 0x0;
	r &= (glPointParameterf = (PFNGLPOINTPARAMETERFPROC) platGetProcAddress( "glPointParameterf" )) != 0x0;
	r &= (glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC) platGetProcAddress( "glPointParameterfv" )) != 0x0;
	r &= (glPointParameteri = (PFNGLPOINTPARAMETERIPROC) platGetProcAddress( "glPointParameteri" )) != 0x0;
	r &= (glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC) platGetProcAddress( "glPointParameteriv" )) != 0x0;

	// GL 1.5
	r &= (glGenQueries = (PFNGLGENQUERIESPROC) platGetProcAddress( "glGenQueries" )) != 0x0;
	r &= (glDeleteQueries = (PFNGLDELETEQUERIESPROC) platGetProcAddress( "glDeleteQueries" )) != 0x0;
	r &= (glIsQuery = (PFNGLISQUERYPROC) platGetProcAddress( "glIsQuery" )) != 0x0;
	r &= (glBeginQuery = (PFNGLBEGINQUERYPROC) platGetProcAddress( "glBeginQuery" )) != 0x0;
	r &= (glEndQuery = (PFNGLENDQUERYPROC) platGetProcAddress( "glEndQuery" )) != 0x0;
	r &= (glGetQueryiv = (PFNGLGETQUERYIVPROC) platGetProcAddress( "glGetQueryiv" )) != 0x0;
	r &= (glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC) platGetProcAddress( "glGetQueryObjectiv" )) != 0x0;
	r &= (glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC) platGetProcAddress( "glGetQueryObjectuiv" )) != 0x0;
	r &= (glBindBuffer = (PFNGLBINDBUFFERPROC) platGetProcAddress( "glBindBuffer" )) != 0x0;
	r &= (glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) platGetProcAddress( "glDeleteBuffers" )) != 0x0;
	r &= (glGenBuffers = (PFNGLGENBUFFERSPROC) platGetProcAddress( "glGenBuffers" )) != 0x0;
	r &= (glIsBuffer = (PFNGLISBUFFERPROC) platGetProcAddress( "glIsBuffer" )) != 0x0;
	r &= (glBufferData = (PFNGLBUFFERDATAPROC) platGetProcAddress( "glBufferData" )) != 0x0;
	r &= (glBufferSubData = (PFNGLBUFFERSUBDATAPROC) platGetProcAddress( "glBufferSubData" )) != 0x0;
	r &= (glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC) platGetProcAddress( "glGetBufferSubData" )) != 0x0;
	r &= (glMapBuffer = (PFNGLMAPBUFFERPROC) platGetProcAddress( "glMapBuffer" )) != 0x0;
	r &= (glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) platGetProcAddress( "glUnmapBuffer" )) != 0x0;
	r &= (glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) platGetProcAddress( "glGetBufferParameteriv" )) != 0x0;
	r &= (glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC) platGetProcAddress( "glGetBufferPointerv" )) != 0x0;
	
	// GL 2.0
	r &= (glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) platGetProcAddress( "glBlendEquationSeparate" )) != 0x0;
	r &= (glDrawBuffers = (PFNGLDRAWBUFFERSPROC) platGetProcAddress( "glDrawBuffers" )) != 0x0;
	r &= (glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) platGetProcAddress( "glStencilOpSeparate" )) != 0x0;
	r &= (glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) platGetProcAddress( "glStencilFuncSeparate" )) != 0x0;
	r &= (glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) platGetProcAddress( "glStencilMaskSeparate" )) != 0x0;
	r &= (glAttachShader = (PFNGLATTACHSHADERPROC) platGetProcAddress( "glAttachShader" )) != 0x0;
	r &= (glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) platGetProcAddress( "glBindAttribLocation" )) != 0x0;
	r &= (glCompileShader = (PFNGLCOMPILESHADERPROC) platGetProcAddress( "glCompileShader" )) != 0x0;
	r &= (glCreateProgram = (PFNGLCREATEPROGRAMPROC) platGetProcAddress( "glCreateProgram" )) != 0x0;
	r &= (glCreateShader = (PFNGLCREATESHADERPROC) platGetProcAddress( "glCreateShader" )) != 0x0;
	r &= (glDeleteProgram = (PFNGLDELETEPROGRAMPROC) platGetProcAddress( "glDeleteProgram" )) != 0x0;
	r &= (glDeleteShader = (PFNGLDELETESHADERPROC) platGetProcAddress( "glDeleteShader" )) != 0x0;
	r &= (glDetachShader = (PFNGLDETACHSHADERPROC) platGetProcAddress( "glDetachShader" )) != 0x0;
	r &= (glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) platGetProcAddress( "glDisableVertexAttribArray" )) != 0x0;
	r &= (glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) platGetProcAddress( "glEnableVertexAttribArray" )) != 0x0;
	r &= (glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) platGetProcAddress( "glGetActiveAttrib" )) != 0x0;
	r &= (glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) platGetProcAddress( "glGetActiveUniform" )) != 0x0;
	r &= (glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) platGetProcAddress( "glGetAttachedShaders" )) != 0x0;
	r &= (glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) platGetProcAddress( "glGetAttribLocation" )) != 0x0;
	r &= (glGetProgramiv = (PFNGLGETPROGRAMIVPROC) platGetProcAddress( "glGetProgramiv" )) != 0x0;
	r &= (glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) platGetProcAddress( "glGetProgramInfoLog" )) != 0x0;
	r &= (glGetShaderiv = (PFNGLGETSHADERIVPROC) platGetProcAddress( "glGetShaderiv" )) != 0x0;
	r &= (glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) platGetProcAddress( "glGetShaderInfoLog" )) != 0x0;
	r &= (glGetShaderSource = (PFNGLGETSHADERSOURCEPROC) platGetProcAddress( "glGetShaderSource" )) != 0x0;
	r &= (glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) platGetProcAddress( "glGetUniformLocation" )) != 0x0;
	r &= (glGetUniformfv = (PFNGLGETUNIFORMFVPROC) platGetProcAddress( "glGetUniformfv" )) != 0x0;
	r &= (glGetUniformiv = (PFNGLGETUNIFORMIVPROC) platGetProcAddress( "glGetUniformiv" )) != 0x0;
	r &= (glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC) platGetProcAddress( "glGetVertexAttribdv" )) != 0x0;
	r &= (glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) platGetProcAddress( "glGetVertexAttribfv" )) != 0x0;
	r &= (glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) platGetProcAddress( "glGetVertexAttribiv" )) != 0x0;
	r &= (glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) platGetProcAddress( "glGetVertexAttribPointerv" )) != 0x0;
	r &= (glIsProgram = (PFNGLISPROGRAMPROC) platGetProcAddress( "glIsProgram" )) != 0x0;
	r &= (glIsShader = (PFNGLISSHADERPROC) platGetProcAddress( "glIsShader" )) != 0x0;
	r &= (glLinkProgram = (PFNGLLINKPROGRAMPROC) platGetProcAddress( "glLinkProgram" )) != 0x0;
	r &= (glShaderSource = (PFNGLSHADERSOURCEPROC) platGetProcAddress( "glShaderSource" )) != 0x0;
	r &= (glUseProgram = (PFNGLUSEPROGRAMPROC) platGetProcAddress( "glUseProgram" )) != 0x0;
	r &= (glUniform1f = (PFNGLUNIFORM1FPROC) platGetProcAddress( "glUniform1f" )) != 0x0;
	r &= (glUniform2f = (PFNGLUNIFORM2FPROC) platGetProcAddress( "glUniform2f" )) != 0x0;
	r &= (glUniform3f = (PFNGLUNIFORM3FPROC) platGetProcAddress( "glUniform3f" )) != 0x0;
	r &= (glUniform4f = (PFNGLUNIFORM4FPROC) platGetProcAddress( "glUniform4f" )) != 0x0;
	r &= (glUniform1i = (PFNGLUNIFORM1IPROC) platGetProcAddress( "glUniform1i" )) != 0x0;
	r &= (glUniform2i = (PFNGLUNIFORM2IPROC) platGetProcAddress( "glUniform2i" )) != 0x0;
	r &= (glUniform3i = (PFNGLUNIFORM3IPROC) platGetProcAddress( "glUniform3i" )) != 0x0;
	r &= (glUniform4i = (PFNGLUNIFORM4IPROC) platGetProcAddress( "glUniform4i" )) != 0x0;
	r &= (glUniform1fv = (PFNGLUNIFORM1FVPROC) platGetProcAddress( "glUniform1fv" )) != 0x0;
	r &= (glUniform2fv = (PFNGLUNIFORM2FVPROC) platGetProcAddress( "glUniform2fv" )) != 0x0;
	r &= (glUniform3fv = (PFNGLUNIFORM3FVPROC) platGetProcAddress( "glUniform3fv" )) != 0x0;
	r &= (glUniform4fv = (PFNGLUNIFORM4FVPROC) platGetProcAddress( "glUniform4fv" )) != 0x0;
	r &= (glUniform1iv = (PFNGLUNIFORM1IVPROC) platGetProcAddress( "glUniform1iv" )) != 0x0;
	r &= (glUniform2iv = (PFNGLUNIFORM2IVPROC) platGetProcAddress( "glUniform2iv" )) != 0x0;
	r &= (glUniform3iv = (PFNGLUNIFORM3IVPROC) platGetProcAddress( "glUniform3iv" )) != 0x0;
	r &= (glUniform4iv = (PFNGLUNIFORM4IVPROC) platGetProcAddress( "glUniform4iv" )) != 0x0;
	r &= (glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) platGetProcAddress( "glUniformMatrix2fv" )) != 0x0;
	r &= (glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) platGetProcAddress( "glUniformMatrix3fv" )) != 0x0;
	r &= (glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) platGetProcAddress( "glUniformMatrix4fv" )) != 0x0;
	r &= (glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) platGetProcAddress( "glValidateProgram" )) != 0x0;
	r &= (glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) platGetProcAddress( "glVertexAttribPointer" )) != 0x0;

	/*// GL 2.1
	r &= (glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) platGetProcAddress( "glUniformMatrix2x3fv" )) != 0x0;
	r &= (glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) platGetProcAddress( "glUniformMatrix3x2fv" )) != 0x0;
	r &= (glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) platGetProcAddress( "glUniformMatrix2x4fv" )) != 0x0;
	r &= (glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) platGetProcAddress( "glUniformMatrix4x2fv" )) != 0x0;
	r &= (glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) platGetProcAddress( "glUniformMatrix3x4fv" )) != 0x0;
	r &= (glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) platGetProcAddress( "glUniformMatrix4x3fv" )) != 0x0;*/

	// GL 3.0 
	if( glExt::majorVersion >= 3 )
	{
		r &= (glGetStringi = (PFNGLGETSTRINGIPROC) platGetProcAddress( "glGetStringi" )) != 0x0;
	}
	
	// Extensions
	glExt::EXT_framebuffer_object = isExtensionSupported( "GL_EXT_framebuffer_object" );
	if( glExt::EXT_framebuffer_object )
	{
		r &= (glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC) platGetProcAddress( "glIsRenderbufferEXT" )) != 0x0;
		r &= (glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) platGetProcAddress( "glBindRenderbufferEXT" )) != 0x0;
		r &= (glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) platGetProcAddress( "glDeleteRenderbuffersEXT" )) != 0x0;
		r &= (glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) platGetProcAddress( "glGenRenderbuffersEXT" )) != 0x0;
		r &= (glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) platGetProcAddress( "glRenderbufferStorageEXT" )) != 0x0;
		r &= (glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) platGetProcAddress( "glGetRenderbufferParameterivEXT" )) != 0x0;
		r &= (glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC) platGetProcAddress( "glIsFramebufferEXT" )) != 0x0;
		r &= (glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) platGetProcAddress( "glBindFramebufferEXT" )) != 0x0;
		r &= (glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) platGetProcAddress( "glDeleteFramebuffersEXT" )) != 0x0;
		r &= (glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) platGetProcAddress( "glGenFramebuffersEXT" )) != 0x0;
		r &= (glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) platGetProcAddress( "glCheckFramebufferStatusEXT" )) != 0x0;
		r &= (glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) platGetProcAddress( "glFramebufferTexture1DEXT" )) != 0x0;
		r &= (glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) platGetProcAddress( "glFramebufferTexture2DEXT" )) != 0x0;
		r &= (glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) platGetProcAddress( "glFramebufferTexture3DEXT" )) != 0x0;
		r &= (glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) platGetProcAddress( "glFramebufferRenderbufferEXT" )) != 0x0;
		r &= (glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) platGetProcAddress( "glGetFramebufferAttachmentParameterivEXT" )) != 0x0;
		r &= (glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) platGetProcAddress( "glGenerateMipmapEXT" )) != 0x0;
	}
	
	glExt::EXT_texture_filter_anisotropic = isExtensionSupported( "GL_EXT_texture_filter_anisotropic" );

	glExt::EXT_texture_compression_s3tc = isExtensionSupported( "GL_EXT_texture_compression_s3tc" );

	glExt::EXT_texture_sRGB = isExtensionSupported( "GL_EXT_texture_sRGB" );

	glExt::ARB_texture_float = isExtensionSupported( "GL_ARB_texture_float" ) ||
	                           isExtensionSupported( "GL_ATI_texture_float" );

	glExt::ARB_texture_non_power_of_two = isExtensionSupported( "GL_ARB_texture_non_power_of_two" );

	glExt::EXT_framebuffer_multisample = isExtensionSupported( "GL_EXT_framebuffer_multisample" ) &&
	                                     isExtensionSupported( "GL_EXT_framebuffer_blit" );
	if( glExt::EXT_framebuffer_multisample )
	{
		// From GL_EXT_framebuffer_blit
		r &= (glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC) platGetProcAddress( "glBlitFramebufferEXT" )) != 0x0;
		// From GL_EXT_framebuffer_multisample
		r &= (glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) platGetProcAddress( "glRenderbufferStorageMultisampleEXT" )) != 0x0;
	}

	glExt::ARB_timer_query = isExtensionSupported( "GL_ARB_timer_query" );
	if( glExt::ARB_timer_query )
	{
		r &= (glQueryCounter = (PFNGLQUERYCOUNTERPROC) platGetProcAddress( "glQueryCounter" )) != 0x0;
		r &= (glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC) platGetProcAddress( "glGetQueryObjecti64v" )) != 0x0;
		r &= (glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC) platGetProcAddress( "glGetQueryObjectui64v" )) != 0x0;
	}

	return r;
}
