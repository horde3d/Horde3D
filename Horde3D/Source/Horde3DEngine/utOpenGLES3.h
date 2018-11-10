// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#pragma once

#if defined( __gl_h_ ) || defined( __GL_H__ )
#   error gl.h included before utOpenGLES3.h
#endif

#include "utPlatform.h"

#if defined( PLATFORM_WIN ) || defined( PLATFORM_WIN_CE )
#   define WIN32_LEAN_AND_MEAN 1
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#   include <stddef.h>
#   define GLAPI __declspec( dllimport )
#   define GLAPIENTRY _stdcall
#   define GLAPIENTRYP _stdcall *
#   ifdef PLATFORM_WIN_CE
extern "C" GLAPI void* eglGetProcAddress( const char *procname );
#   endif
#elif defined(PLATFORM_IOS)
#   include <stddef.h>
#   define GLAPIENTRY
#   define GLAPI extern
#   define GLAPIENTRYP *
#else
#   include <stddef.h>
#	define GLAPI
#	define GLAPIENTRY
#   define GLAPIENTRYP *
#   ifdef PLATFORM_MAC
#      include <Carbon/Carbon.h>
#   else
extern "C" void( *glXGetProcAddressARB( const unsigned char *procName ) )( void );
#   endif
#endif


namespace glESExt
{
	extern bool EXT_texture_filter_anisotropic;

	extern bool EXT_color_buffer_float;
	
	extern bool IMG_texture_compression_pvrtc;
	extern bool EXT_texture_compression_s3tc;
	extern bool EXT_texture_compression_dxt1;
	extern bool EXT_texture_compression_bptc;
	extern bool KHR_texture_compression_astc;

	extern bool EXT_disjoint_timer_query;
	extern bool EXT_texture_border_clamp;
	extern bool EXT_geometry_shader;
	extern bool EXT_texture_border_clamp;
	extern bool OES_texture_3D;
	extern bool OES_EGL_image_external;

	extern int  majorVersion, minorVersion;
}

bool initOpenGLExtensions();


#if defined( PLATFORM_WIN ) || defined( PLATFORM_WIN_CE )
#include <GLES3/gl3platform.h>
#define GL_PROTOTYPES
//#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl31.h>
//#include <GLES2/gl2ext.h>
#elif defined(PLATFORM_LINUX)
#include "GLES3/gl3platform.h"
#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl31.h>
#elif defined(PLATFORM_IOS)
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define GL_GLEXT_PROTOTYPES
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_NACL) || defined(PLATFORM_QNX)
#include <GLES3/gl31.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#endif

// =================================================================================================
// Extensions
// =================================================================================================

namespace h3dGLES
{

// EXT_texture_filter_anisotropic
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1

#define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF

#endif


// EXT_texture_compression_s3tc
#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc 1

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3

#endif

// EXT_texture_compression_s3tc_srgb
#ifndef GL_EXT_texture_compression_s3tc_srgb
#define GL_EXT_texture_compression_s3tc_srgb 1

#define	GL_COMPRESSED_SRGB_S3TC_DXT1_EXT                  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT            0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT            0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT            0x8C4F

#endif

// EXT_texture_compression_bptc
#ifndef GL_EXT_texture_compression_bptc
#define GL_EXT_texture_compression_bptc 1

#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT                     0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT               0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT               0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT             0x8E8F

#endif

// GL_OES_compressed_ETC1
#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_OES_compressed_ETC1_RGB8_texture
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif 

// GL_OES_texture_3D 
#ifndef GL_OES_texture_3D
#define GL_OES_texture_3D 1

#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES        0x8CD4

typedef void ( GLAPIENTRYP PFNGLFRAMEBUFFERTEXTURE3DOESPROC ) ( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, int level, int zoffset );

extern PFNGLFRAMEBUFFERTEXTURE3DOESPROC glFramebufferTexture3DOES;

#endif

// EXT_disjoint_timer_query
#ifndef GL_EXT_disjoint_timer_query
#define GL_EXT_disjoint_timer_query 1

#define GL_TIMESTAMP_EXT                                0x8E28
#define GL_QUERY_RESULT_EXT                             0x8866
#define	GL_QUERY_RESULT_AVAILABLE_EXT                   0x8867

typedef void ( GLAPIENTRYP PFNGLQUERYCOUNTEREXTPROC ) ( GLuint id, GLenum target );
typedef void ( GLAPIENTRYP PFGLGETQUERYOBJECTIVEXTPROC ) ( GLuint id, GLenum pname, GLint *params );
typedef void ( GLAPIENTRYP PFGLGETQUERYOBJECTUI64VEXTPROC ) ( GLuint id, GLenum pname, GLuint64 *params );

extern PFNGLQUERYCOUNTEREXTPROC glQueryCounterEXT;
extern PFGLGETQUERYOBJECTIVEXTPROC glGetQueryObjectivEXT;
extern PFGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT;

#endif	//GL_EXT_disjoint_timer_query


//	EXT_geometry_shader
#ifndef GL_EXT_geometry_shader
#define GL_EXT_geometry_shader 1

#define GL_GEOMETRY_SHADER_EXT							   0x8DD9
#define GL_GEOMETRY_SHADER_BIT_EXT                         0x00000004
#define GL_GEOMETRY_LINKED_VERTICES_OUT_EXT                0x8916
#define GL_GEOMETRY_LINKED_INPUT_TYPE_EXT                  0x8917
#define GL_GEOMETRY_LINKED_OUTPUT_TYPE_EXT                 0x8918
#define GL_GEOMETRY_SHADER_INVOCATIONS_EXT                 0x887F
#define GL_LAYER_PROVOKING_VERTEX_EXT                      0x825E
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT             0x8DDF
#define GL_vMAX_GEOMETRY_UNIFORM_BLOCKS_EXT                0x8A2C
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT    0x8A32
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT               0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT              0x9124
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT                0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT        0x8DE1
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT             0x8E5A
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT            0x8C29
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT         0x92CF
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT                0x92D5
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT                 0x90CD
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT          0x90D7
#define GL_FIRST_VERTEX_CONVENTION_EXT                     0x8E4D
#define GL_LAST_VERTEX_CONVENTION_EXT                      0x8E4E
#define GL_UNDEFINED_VERTEX_EXT                            0x8260
#define GL_PRIMITIVES_GENERATED_EXT                        0x8C87
#define GL_LINES_ADJACENCY_EXT                             0xA
#define GL_LINE_STRIP_ADJACENCY_EXT                        0xB
#define GL_TRIANGLES_ADJACENCY_EXT                         0xC
#define GL_TRIANGLE_STRIP_ADJACENCY_EXT                    0xD
#define GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT                  0x9312
#define GL_MAX_FRAMEBUFFER_LAYERS_EXT                      0x9317
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT        0x8DA8
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT              0x8DA7
#define GL_REFERENCED_BY_GEOMETRY_SHADER_EXT               0x9309
#endif //	EXT_geometry_shader


#ifndef GL_EXT_texture_border_clamp
#define GL_EXT_texture_border_clamp 1
 
#define GL_TEXTURE_BORDER_COLOR_EXT                         0x1004
#define GL_CLAMP_TO_BORDER_EXT                              0x812D

//
typedef void ( GLAPIENTRYP PFNGLTEXPARAMETERIIVEXTPROC ) ( GLenum target, GLenum pname, const GLint *params );
typedef void ( GLAPIENTRYP PFNGLTEXPARAMETERIUIVEXTPROC ) ( GLenum target, GLenum pname, const GLuint *params );
typedef void ( GLAPIENTRYP PFNGLGETTEXPARAMETERIIVEXTPROC ) ( GLenum target, GLenum pname, GLint *params );
typedef void ( GLAPIENTRYP PFNGLGETTEXPARAMETERIUIVEXTPROC ) ( GLenum target, GLenum pname, GLuint *params );
typedef void ( GLAPIENTRYP PFNGLSAMPLERPARAMETERIIVEXTPROC ) ( GLuint sampler, GLenum pname, const GLint *params );
typedef void ( GLAPIENTRYP PFNGLSAMPLERPARAMETERIUIVEXTPROC ) ( GLuint sampler, GLenum pname, const GLuint *params );
typedef void ( GLAPIENTRYP PFNGLGETSAMPLERPARAMETERIIVEXTPROC ) ( GLuint sampler, GLenum pname, GLint *params );
typedef void ( GLAPIENTRYP PFNGLGETSAMPLERPARAMETERIUIVEXTPROC ) ( GLuint sampler, GLenum pname, GLuint *params );

extern PFNGLTEXPARAMETERIIVEXTPROC glTexParameterIivEXT;
extern PFNGLTEXPARAMETERIUIVEXTPROC glTexParameterIuivEXT;
extern PFNGLGETTEXPARAMETERIIVEXTPROC glGetTexParameterIivEXT;
extern PFNGLGETTEXPARAMETERIUIVEXTPROC glGetTexParameterIuivEXT;
extern PFNGLSAMPLERPARAMETERIIVEXTPROC glSamplerParameterIivEXT;
extern PFNGLSAMPLERPARAMETERIUIVEXTPROC glSamplerParameterIuivEXT;
extern PFNGLGETSAMPLERPARAMETERIIVEXTPROC glGetSamplerParameterIivEXT;
extern PFNGLGETSAMPLERPARAMETERIUIVEXTPROC glGetSamplerParameterIuivEXT;

#endif

/* GL_OES_EGL_image_external */
#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1

typedef void* GLeglImageOES;

typedef void (GLAPIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
typedef void (GLAPIENTRYP PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) (GLenum target, GLeglImageOES image);


extern PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
#endif

#ifndef GL_KHR_texture_compression_astc_ldr
#define GL_KHR_texture_compression_astc_ldr 1
#define GL_KHR_texture_compression_astc_hdr 1

#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR            0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR            0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR            0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR            0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR            0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR            0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR            0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR            0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR           0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR           0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR           0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR          0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR          0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR          0x93BD

#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR    0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR    0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR    0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR    0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR    0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR    0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR    0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR    0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR   0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR   0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR   0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR  0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR  0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR  0x93DD

#endif
}  // namespace h3dGLES

