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

#ifndef _utImage_H_
#define _utImage_H_

// Configuration
#define STBI_NO_STDIO	1
#define STBI_NO_WRITE	1

// Limitations:
//    - no 16-bit-per-channel PNG
//    - no 12-bit-per-channel JPEG
//    - no JPEGs with arithmetic coding
//    - no 1-bit BMP
//    - GIF always returns *comp=4
//
// Basic usage (see HDR discussion below for HDR usage):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x       -- outputs image width in pixels
//    int *y       -- outputs image height in pixels
//    int *comp    -- outputs # of image components in image file
//    int req_comp -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data, or NULL on an allocation failure or if the image is
// corrupt or invalid. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'req_comp' if req_comp is non-zero, or *comp otherwise.
// If req_comp is non-zero, *comp has the number of components that _would_
// have been output otherwise. E.g. if you set req_comp to 4, you will always
// get RGBA output, but you can check *comp to see if it's trivially opaque
// because e.g. there were only 3 channels in the source image.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *comp will be unchanged. The function stbi_failure_reason()
// can be queried for an extremely brief, end-user unfriendly explanation
// of why the load failed. Define STBI_NO_FAILURE_STRINGS to avoid
// compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG, BMP, GIF, and PIC images are automatically depalettized.
//
// ===========================================================================
//
// Philosophy
//
// stb libraries are designed with the following priorities:
//
//    1. easy to use
//    2. easy to maintain
//    3. good performance
//
// Sometimes I let "good performance" creep up in priority over "easy to maintain",
// and for best performance I may provide less-easy-to-use APIs that give higher
// performance, in addition to the easy to use ones. Nevertheless, it's important
// to keep in mind that from the standpoint of you, a client of this library,
// all you care about is #1 and #3, and stb libraries do not emphasize #3 above all.
//
// Some secondary priorities arise directly from the first two, some of which
// make more explicit reasons why performance can't be emphasized.
//
//    - Portable ("ease of use")
//    - Small footprint ("easy to maintain")
//    - No dependencies ("ease of use")
//
// ===========================================================================
//
// I/O callbacks
//
// I/O callbacks allow you to read from arbitrary sources, like packaged
// files or some other source. Data read from callbacks are processed
// through a small internal buffer (currently 128 bytes) to try to reduce
// overhead.
//
// The three functions you must define are "read" (reads some bytes of data),
// "skip" (skips some bytes of data), "eof" (reports if the stream is at the end).
//
// ===========================================================================
//
// SIMD support
//
// The JPEG decoder will try to automatically use SIMD kernels on x86 when
// supported by the compiler. For ARM Neon support, you must explicitly
// request it.
//
// (The old do-it-yourself SIMD API is no longer supported in the current
// code.)
//
// On x86, SSE2 will automatically be used when available based on a run-time
// test; if not, the generic C versions are used as a fall-back. On ARM targets,
// the typical path is to have separate builds for NEON and non-NEON devices
// (at least this is true for iOS and Android). Therefore, the NEON support is
// toggled by a build flag: define STBI_NEON to get NEON loops.
//
// The output of the JPEG decoder is slightly different from versions where
// SIMD support was introduced (that is, for versions before 1.49). The
// difference is only +-1 in the 8-bit RGB channels, and only on a small
// fraction of pixels. You can force the pre-1.49 behavior by defining
// STBI_JPEG_OLD, but this will disable some of the SIMD decoding path
// and hence cost some performance.
//
// If for some reason you do not want to use any of SIMD code, or if
// you have issues compiling it, you can disable it entirely by
// defining STBI_NO_SIMD.
//
// ===========================================================================
//
// HDR image support   (disable by defining STBI_NO_HDR)
//
// stb_image now supports loading HDR images in general, and currently
// the Radiance .HDR file format, although the support is provided
// generically. You can still load any file through the existing interface;
// if you attempt to load an HDR file, it will be automatically remapped to
// LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
// both of these constants can be reconfigured through this interface:
//
//     stbi_hdr_to_ldr_gamma(2.2f);
//     stbi_hdr_to_ldr_scale(1.0f);
//
// (note, do not use _inverse_ constants; stbi_image will invert them
// appropriately).
//
// Additionally, there is a new, parallel interface for loading files as
// (linear) floats to preserve the full dynamic range:
//
//    float *data = stbi_loadf(filename, &x, &y, &n, 0);
//
// If you load LDR images through this interface, those images will
// be promoted to floating point values, run through the inverse of
// constants corresponding to the above:
//
//     stbi_ldr_to_hdr_scale(1.0f);
//     stbi_ldr_to_hdr_gamma(2.2f);
//
// Finally, given a filename (or an open file or memory block--see header
// file for details) containing image data, you can query for the "most
// appropriate" interface to use (that is, whether the image is HDR or
// not), using:
//
//     stbi_is_hdr(char *filename);
//
// ===========================================================================
//
// iPhone PNG support:
//
// By default we convert iphone-formatted PNGs back to RGB, even though
// they are internally encoded differently. You can disable this conversion
// by by calling stbi_convert_iphone_png_to_rgb(0), in which case
// you will always just get the native iphone "format" through (which
// is BGR stored in RGB).
//
// Call stbi_set_unpremultiply_on_load(1) as well to force a divide per
// pixel to remove any premultiplied alpha *only* if the image file explicitly
// says there's premultiplied data (currently only happens in iPhone images,
// and only if iPhone convert-to-rgb processing is on).
//

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

namespace Horde3D {

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif // STBI_NO_STDIO

#define STBI_VERSION 1

	enum
	{
		STBI_default = 0, // only used for req_comp

		STBI_grey       = 1,
		STBI_grey_alpha = 2,
		STBI_rgb        = 3,
		STBI_rgb_alpha  = 4
	};

	typedef unsigned char stbi_uc;

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef STB_IMAGE_STATIC
#define STBIDEF static
#else
#define STBIDEF extern
#endif

		//////////////////////////////////////////////////////////////////////////////
		//
		// PRIMARY API - works on images of any type
		//

		//
		// load image by filename, open file, or memory buffer
		//

		typedef struct
		{
			int      (*read)  (void *user,char *data,int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read
			void     (*skip)  (void *user,int n);                 // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
			int      (*eof)   (void *user);                       // returns nonzero if we are at end of file/data
		} stbi_io_callbacks;

		STBIDEF stbi_uc *stbi_load( char              const *filename, int *x, int *y, int *comp, int req_comp );
		STBIDEF stbi_uc *stbi_load_from_memory( stbi_uc           const *buffer, int len, int *x, int *y, int *comp, int req_comp );
		STBIDEF stbi_uc *stbi_load_from_callbacks( stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp );

#ifndef STBI_NO_STDIO
		STBIDEF stbi_uc *stbi_load_from_file( FILE *f, int *x, int *y, int *comp, int req_comp );
		// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

#ifndef STBI_NO_LINEAR
		STBIDEF float *stbi_loadf( char const *filename, int *x, int *y, int *comp, int req_comp );
		STBIDEF float *stbi_loadf_from_memory( stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp );
		STBIDEF float *stbi_loadf_from_callbacks( stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp );

#ifndef STBI_NO_STDIO
		STBIDEF float *stbi_loadf_from_file( FILE *f, int *x, int *y, int *comp, int req_comp );
#endif
#endif

#ifndef STBI_NO_HDR
		STBIDEF void   stbi_hdr_to_ldr_gamma( float gamma );
		STBIDEF void   stbi_hdr_to_ldr_scale( float scale );
#endif // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
		STBIDEF void   stbi_ldr_to_hdr_gamma( float gamma );
		STBIDEF void   stbi_ldr_to_hdr_scale( float scale );
#endif // STBI_NO_LINEAR

		// stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
		STBIDEF int    stbi_is_hdr_from_callbacks( stbi_io_callbacks const *clbk, void *user );
		STBIDEF int    stbi_is_hdr_from_memory( stbi_uc const *buffer, int len );
#ifndef STBI_NO_STDIO
		STBIDEF int      stbi_is_hdr( char const *filename );
		STBIDEF int      stbi_is_hdr_from_file( FILE *f );
#endif // STBI_NO_STDIO


		// get a VERY brief reason for failure
		// NOT THREADSAFE
		STBIDEF const char *stbi_failure_reason( void );

		// free the loaded image -- this is just free()
		STBIDEF void     stbi_image_free( void *retval_from_stbi_load );

		// get image dimensions & components without fully decoding
		STBIDEF int      stbi_info_from_memory( stbi_uc const *buffer, int len, int *x, int *y, int *comp );
		STBIDEF int      stbi_info_from_callbacks( stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp );

#ifndef STBI_NO_STDIO
		STBIDEF int      stbi_info( char const *filename, int *x, int *y, int *comp );
		STBIDEF int      stbi_info_from_file( FILE *f, int *x, int *y, int *comp );

#endif



		// for image formats that explicitly notate that they have premultiplied alpha,
		// we just return the colors as stored in the file. set this flag to force
		// unpremultiplication. results are undefined if the unpremultiply overflow.
		STBIDEF void stbi_set_unpremultiply_on_load( int flag_true_if_should_unpremultiply );

		// indicate whether we should process iphone images back to canonical format,
		// or just pass them through "as-is"
		STBIDEF void stbi_convert_iphone_png_to_rgb( int flag_true_if_should_convert );

		// flip the image vertically, so the first pixel in the output array is the bottom left
		STBIDEF void stbi_set_flip_vertically_on_load( int flag_true_if_should_flip );

		// ZLIB client - used by PNG, available for other purposes

		STBIDEF char *stbi_zlib_decode_malloc_guesssize( const char *buffer, int len, int initial_size, int *outlen );
		STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag( const char *buffer, int len, int initial_size, int *outlen, int parse_header );
		STBIDEF char *stbi_zlib_decode_malloc( const char *buffer, int len, int *outlen );
		STBIDEF int   stbi_zlib_decode_buffer( char *obuffer, int olen, const char *ibuffer, int ilen );

		STBIDEF char *stbi_zlib_decode_noheader_malloc( const char *buffer, int len, int *outlen );
		STBIDEF int   stbi_zlib_decode_noheader_buffer( char *obuffer, int olen, const char *ibuffer, int ilen );


#ifdef __cplusplus
}
#endif

}
#endif // _utImage_H_
