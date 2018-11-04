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

#ifndef _utPlatform_H_
#define _utPlatform_H_

#if defined( _DEBUG )
	#include <assert.h>
#endif

// Detect platform
#if defined( WINCE )
#	if !defined( PLATFORM_WIN_CE )
#		define PLATFORM_WIN_CE
#	endif
#elif defined( WIN32 ) || defined( _WINDOWS ) || defined( _WIN32 )
#	if !defined( PLATFORM_WIN )
#		define PLATFORM_WIN
#	endif
#elif defined( __APPLE__ ) || defined( __APPLE_CC__ )
#   if !defined( PLATFORM_MAC )
#		define PLATFORM_MAC
#   endif
#else
#	if !defined( PLATFORM_LINUX )
#		define PLATFORM_LINUX
#	endif
#endif

// Endianess
#if defined (__GLIBC__)
#   include <endian.h>
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#       define PLATFORM_LITTLE_ENDIAN
#   elif __BYTE_ORDER == __BIG_ENDIAN
#       define PLATFORM_BIG_ENDIAN
#   else
#       error Unknown __BYTE_ORDER on endian.h
#   endif
#elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN) ||\
      defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__) ||\
      defined(_STLP_BIG_ENDIAN) && !defined(_STLP_LITTLE_ENDIAN)
#       define PLATFORM_BIG_ENDIAN
#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN) ||\
      defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__) ||\
      defined(_STLP_LITTLE_ENDIAN) && !defined(_STLP_BIG_ENDIAN)
#       define PLATFORM_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) \
   || defined(_POWER) || defined(__powerpc__) \
   || defined(__ppc__) || defined(__hpux) || defined(__hppa) \
   || defined(_MIPSEB) || defined(_POWER) \
   || defined(__s390__)
#       define PLATFORM_BIG_ENDIAN
#elif defined(__i386__) || defined(__alpha__) \
   || defined(__ia64) || defined(__ia64__) \
   || defined(_M_IX86) || defined(_M_IA64) \
   || defined(_M_ALPHA) || defined(__amd64) \
   || defined(__amd64__) || defined(_M_AMD64) \
   || defined(__x86_64) || defined(__x86_64__) \
   || defined(_M_X64) || defined(__bfin__) \
   || defined(__arm__) || defined(__ARM_ARCH_7__) \
   || defined(__aarch64__)
#       define PLATFORM_LITTLE_ENDIAN
#else
#   error Unknown endianess.
#endif


#ifndef DLLEXP
#	ifdef PLATFORM_WIN
#		define DLLEXP extern "C" __declspec( dllexport )
#	else
#		if defined( __GNUC__ ) && __GNUC__ >= 4
#		  define DLLEXP extern "C" __attribute__ ((visibility("default")))
#   	else
#		  define DLLEXP extern "C"
#   	endif
#	endif
#endif

#if defined( PLATFORM_WIN ) || defined( PLATFORM_MAC ) || defined ( PLATFORM_LINUX )
#	define DESKTOP_OPENGL_AVAILABLE
#endif

// Shortcuts for common types
typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;


#if !defined( PLATFORM_WIN ) && !defined( PLATFORM_WIN_CE )
#	define _stricmp strcasecmp
#	define _mkdir( name ) mkdir( name, 0755 )
#endif

#ifdef PLATFORM_WIN_CE
#define GetProcessAffinityMask
#define SetThreadAffinityMask
#ifndef vsnprintf
#	define vsnprintf _vsnprintf
#endif
#undef ASSERT
#undef min
#undef max
#endif

#if (!defined( _MSC_VER ) || (defined( _MSC_VER ) && (_MSC_VER < 1400))) && !defined( __MINGW32__ )
#	define strncpy_s( dst, dstSize, src, count ) strncpy( dst, src, count < dstSize ? count : dstSize )
#endif
#if defined( _MSC_VER ) && (_MSC_VER < 1400)
#   define vsnprintf _vsnprintf
#endif

// Runtime assertion
#if defined( _DEBUG )
#	define ASSERT( exp ) assert( exp );
#else
#	define ASSERT( exp )
#endif

// Support for MSVC static code analysis
#if defined( _MSC_VER ) && defined( _PREFAST_ )
#	define ASSERT( exp ) __analysis_assume( exp );
#endif

#if !defined( _MSC_VER )
#include <strings.h>
#endif

// Static compile-time assertion
namespace StaticAssert
{
	template< bool > struct FAILED;
	template<> struct FAILED< true > { };
}
#define ASSERT_STATIC( exp ) (StaticAssert::FAILED< (exp) != 0 >())

// Misc functions
#define H3D_UNUSED_VAR( exp ) ( (void) (exp))

#endif // _utPlatform_H_
