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
#ifndef _utEndian_H_
#define _utEndian_H_

#include <utPlatform.h>
#include <cstring>

//for testing
//#undef PLATFORM_LITTLE_ENDINA
//#define PLATFORM_BIG_ENDIAN

inline void swap_endian(const char* first, const char* last, char* d_first)
{
    memcpy(d_first, first, (last - first));
}

inline void swap_endian(const unsigned char* first, const unsigned char* last, unsigned char* d_first)
{
    memcpy(d_first, first, (last - first));
}

inline void swap_endian(const uint16* first, const uint16* last, uint16* d_first)
{
    for(; first != last; ++first, ++d_first)
    {
        uint16 val = *first;
        *d_first = ((val >> 8) & 0x00FF) |
                   ((val << 8) & 0xFF00);
    }
}

inline void swap_endian(const int16* first, const int16* last, int16* d_first)
{
    // signed shifting performs arithmetic shift which extends the sign bit, we don't really want this.
    // so let's call the unsigned version.
    swap_endian(reinterpret_cast<const uint16*>(first),
                reinterpret_cast<const uint16*>(last),
                reinterpret_cast<uint16*>(d_first));
}

inline void swap_endian(const uint32* first, const uint32* last, uint32* d_first)
{
    for(; first != last; ++first, ++d_first)
    {
        uint32 val = *first;
        *d_first = ((val >> 24) & 0x000000FF) |
                   ((val << 8)  & 0x00FF0000) |
                   ((val >> 8)  & 0x0000FF00) |
                   ((val << 24) & 0xFF000000);
    }
}

inline void swap_endian(const int32* first, const int32* last, int32* d_first)
{
    // signed shifting performs arithmetic shift which extends the sign bit, we don't really want this.
    // so let's call the unsigned version.
    swap_endian(reinterpret_cast<const uint32*>(first),
                reinterpret_cast<const uint32*>(last),
                reinterpret_cast<uint32*>(d_first));
}

inline void swap_endian(const float* first, const float* last, float* d_first)
{
    ASSERT_STATIC(sizeof(float) == sizeof(uint32));
    swap_endian(reinterpret_cast<const uint32*>(first),
                reinterpret_cast<const uint32*>(last),
                reinterpret_cast<uint32*>(d_first));
}

template<class T>
inline char* elemcpy_le(T* dest, const T* src, size_t num_elems)
{
#if defined(PLATFORM_BIG_ENDIAN)
    swap_endian(src, src + num_elems, dest);
#elif defined(PLATFORM_LITTLE_ENDIAN)
    memcpy(dest, src, num_elems * sizeof(*src));
#else
    #error Unknown endianess
#endif
    return (char*)(src) + num_elems * sizeof(*src);
}

// Same as elemcpy_le except it returns the new 'dest' pointer instead of a new 'src'.
template<class T>
inline char* elemcpyd_le(T* dest, const T* src, size_t num_elems)
{
    elemcpy_le(dest, src, num_elems);
    return (char*)(dest) + num_elems * sizeof(*dest);
}

template<class T>
inline char* elemset_le(T* dest, T value)
{
    return elemcpy_le(dest, &value, 1);
}

template<class T>
inline T elemget_le(T* src)
{
    T value;
    elemcpy_le(&value, src, 1);
    return value;
}

#endif
