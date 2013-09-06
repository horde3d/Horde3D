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

#ifndef _utils_H_
#define _utils_H_

#include "utMath.h"
#include <cstring>
#include <cstdlib>
#include <string>

using namespace Horde3D;


void removeGate( std::string &s );
std::string decodeURL( const std::string &url );
std::string extractFileName( const std::string &fullPath, bool extension );
std::string extractFilePath( const std::string &fullPath );
std::string cleanPath( const std::string &path );

void log( const std::string &msg );

Matrix4f makeMatrix4f( float *floatArray16, bool y_up );

void createDirectories( const std::string &basePath, const std::string &newPath );



inline bool parseString( char *&str, std::string &token )
{
	token.clear();
	token.reserve( 16 );
	
	// Skip whitespace
	while( *str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' )
	{
		++str;
	}
	if( *str == '\0' ) return false;

	// Copy token
	while( *str && *str != ' ' && *str != '\t' && *str != '\n' && *str != '\r' )
	{
		token += *str++;
	}

	return true;
}

inline bool parseFloat( char *&str, float &f )
{
	float sign = 1.0f;
	float value = 0.0f;
	
	// Skip whitespace
	while( *str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' )
	{
		++str;
	}
	if( *str == '\0' ) return false;

	char *firstChar = str;
	
	// Handle sign
    if( *str == '-' )
	{
		sign = -1.0;
        ++str;
    }
	else if( *str == '+' )
	{
		++str;
	}
	
    // Integral part
	while( *str >= '0' && *str <= '9' )
	{
        value = value * 10.0f + (*str++ - '0');
	}

	// Fractional part
	if( *str == '.' )
	{
        ++str;
		float pow10 = 0.1f;

        while( *str >= '0' && *str <= '9' )
		{
            value += (*str++ - '0') * pow10;
            pow10 *= 0.1f;
        }
    }
	
	// Exponent (use standard atof in this case)
	if( *str == 'e' || *str == 'E')
	{
		while( *str && *str != ' ' && *str != '\t' && *str != '\n' && *str != '\r' )
			++str;
		
		if( str - firstChar < 64 )
		{
			char buf[64];
			memcpy( buf, firstChar, str - firstChar );
			buf[str - firstChar] = '\0';

			f = (float)atof( buf );
		}
		else
			return false;
	}
	else
	{
		f = value * sign;
	}
	
	return true;
}

inline bool parseInt( char *&str, int &i )
{
	int value = 0;
	int sign = 1;
	
	// Skip whitespace
	while( *str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' )
	{
		++str;
	}
	if( *str == '\0' ) return false;
	
	// Sign
	if( *str == '-' )
	{
		sign = -1;
        ++str;
    }
	else if( *str == '+' )
	{
		++str;
	}
	
	// Value
	while( *str >= '0' && *str <= '9' )
	{
        value = (value * 10) + (*str++ - '0');
	}

	i = value * sign;
	return true;
}


#endif // _utils_H_
