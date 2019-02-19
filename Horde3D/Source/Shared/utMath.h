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

// -------------------------------------------------------------------------------------------------
//
// Math library
//
// Conventions:
//
// - Coordinate system is right-handed with positive y as up axis
// - All rotation angles are counter-clockwise when looking from the positive end of the rotation
//	 axis towards the origin
// - An unrotated view vector points along the negative z-axis
//
// -------------------------------------------------------------------------------------------------

#ifndef _utMath_H_
#define _utMath_H_

#include <cmath>
#include <cfloat>

namespace Horde3D {

// Constants
namespace Math
{
	const unsigned int MaxUInt32 = 0xFFFFFFFF;
	const int MinInt32 = 0x80000000;
	const int MaxInt32 = 0x7FFFFFFF;
	const float MaxFloat = 3.402823466e+38F;
	const float MinPosFloat = 1.175494351e-38F;
	const double MaxDouble = DBL_MAX;
	const double MinPosDouble = DBL_MIN;
	
	const float Pi = 3.141592654f;
	const float TwoPi = 6.283185307f;
	const float PiHalf = 1.570796327f;

	const float Epsilon = 0.000001f;
	const float ZeroEpsilon = 32.0f * MinPosFloat;  // Very small epsilon for checking against 0.0f
#ifdef __GNUC__
	const float NaN = __builtin_nanf("");
#else
	const float NaN = *(float *)&MaxUInt32;
#endif

	enum NoInitHint
	{
		NO_INIT
	};
};


// -------------------------------------------------------------------------------------------------
// General
// -------------------------------------------------------------------------------------------------

static inline float degToRad( float f ) 
{
	return f * 0.017453293f;
}

static inline double degToRad( double d )
{
	return d * 0.017453293;
}

static inline float radToDeg( float f ) 
{
	return f * 57.29577951f;
}

static inline double radToDeg( double d )
{
	return d * 57.29577951;
}

static inline float clamp( float f, float min, float max )
{
	if( f < min ) f = min;
	else if( f > max ) f = max;

	return f;
}

static inline double clamp( double d, double min, double max )
{
	if( d < min ) d = min;
	else if( d > max ) d = max;

	return d;
}

static inline float minf( float a, float b )
{
	return a < b ? a : b;
}

static inline double minf( double a, double b )
{
	return a < b ? a : b;
}

static inline float maxf( float a, float b )
{
	return a > b ? a : b;
}

static inline double maxf( double a, double b )
{
	return a > b ? a : b;
}

static inline float fsel( float test, float a, float b )
{
	// Branchless selection
	return test >= 0 ? a : b;
}

static inline double fsel( double test, double a, double b )
{
	// Branchless selection
	return test >= 0 ? a : b;
}

// Computes a/b, rounded up
// To be used for positive a and b and small numbers (beware of overflows)
static inline int idivceil( int a, int b )
{
	return (a + b - 1) / b;
}


// -------------------------------------------------------------------------------------------------
// Conversion
// -------------------------------------------------------------------------------------------------

static inline int ftoi_t( double val )
{
	// Float to int conversion using truncation
	
	return (int)val;
}

static inline int ftoi_r( double val )
{
	// Fast round (banker's round) using Sree Kotay's method
	// This function is much faster than a naive cast from float to int

	union
	{
		double dval;
		int ival[2];
	} u;

	u.dval = val + 6755399441055744.0;  // Magic number: 2^52 * 1.5;
	return u.ival[0];         // Needs to be [1] for big-endian
}


// -------------------------------------------------------------------------------------------------
// Vector
// -------------------------------------------------------------------------------------------------

class Vec2f
{
public:
	float x, y;
	
	
	// ------------
	// Constructors
	// ------------
	Vec2f() : x( 0.0f ), y( 0.0f )
	{
	}
	
	explicit Vec2f( Math::NoInitHint )
	{
		// Constructor without default initialization
	}
	
	Vec2f( const float x, const float y ) : x( x ), y( y )
	{
	}
	
	Vec2f( const Vec2f &v ) : x( v.x ), y( v.y )
	{
	}
	
	// ------
	// Access
	// ------
	float operator[]( unsigned int index ) const
	{
		return *(&x + index);
	}

	float &operator[]( unsigned int index )
	{
		return *(&x + index);
	}
	
	// -----------
	// Comparisons
	// -----------
	bool operator==( const Vec2f &v ) const
	{
		return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon &&
				y > v.y - Math::Epsilon && y < v.y + Math::Epsilon);
	}
	
	bool operator!=( const Vec2f &v ) const
	{
		return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon ||
				y < v.y - Math::Epsilon || y > v.y + Math::Epsilon);
	}
	
	// ---------------------
	// Arithmetic operations
	// ---------------------
	Vec2f operator-() const
	{
		return Vec2f( -x, -y );
	}
	
	Vec2f operator+( const Vec2f &v ) const
	{
		return Vec2f( x + v.x, y + v.y );
	}
	
	Vec2f &operator+=( const Vec2f &v )
	{
		return *this = *this + v;
	}
	
	Vec2f operator-( const Vec2f &v ) const
	{
		return Vec2f( x - v.x, y - v.y );
	}
	
	Vec2f &operator-=( const Vec2f &v )
	{
		return *this = *this - v;
	}
	
	Vec2f operator*( const float f ) const
	{
		return Vec2f( x * f, y * f );
	}
	
	Vec2f &operator*=( const float f )
	{
		return *this = *this * f;
	}
	
	Vec2f operator/( const float f ) const
	{
		return Vec2f( x / f, y / f );
	}
	
	Vec2f &operator/=( const float f )
	{
		return *this = *this / f;
	}
	
	// ----------------
	// Special products
	// ----------------
	float dot( const Vec2f &v ) const
	{
		return x * v.x + y * v.y;
	}
	
	// ----------------
	// Other operations
	// ----------------
	float length() const
	{
		return sqrtf( x * x + y * y );
	}
	
	Vec2f normalized() const
	{
		float invLen = 1.0f / length();
		return Vec2f( x * invLen, y * invLen );
	}
	
	void normalize()
	{
		float invLen = 1.0f / length();
		x *= invLen;
		y *= invLen;
	}
	
	Vec2f lerp( const Vec2f &v, float f ) const
	{
		return Vec2f( x + (v.x - x) * f, y + (v.y - y) * f );
	}
};


class Vec2d
{
public:
	double x, y;


	// ------------
	// Constructors
	// ------------
	Vec2d() : x( 0.0 ), y( 0.0 )
	{
	}

	explicit Vec2d( Math::NoInitHint )
	{
		// Constructor without default initialization
	}

	Vec2d( const double x, const double y ) : x( x ), y( y )
	{
	}

	Vec2d( const Vec2d &v ) : x( v.x ), y( v.y )
	{
	}

	Vec2d( const Vec2f &v ) : x( v.x ), y( v.y )
	{
	}

	// ------
	// Access
	// ------
	double operator[]( unsigned int index ) const
	{
		return *(&x + index);
	}

	double &operator[]( unsigned int index )
	{
		return *(&x + index);
	}

	// -----------
	// Comparisons
	// -----------
	bool operator==( const Vec2d &v ) const
	{
		return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon &&
				y > v.y - Math::Epsilon && y < v.y + Math::Epsilon);
	}

	bool operator==( const Vec2f &v ) const
	{
		return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon &&
				y > v.y - Math::Epsilon && y < v.y + Math::Epsilon);
	}

	bool operator!=( const Vec2d &v ) const
	{
		return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon ||
				y < v.y - Math::Epsilon || y > v.y + Math::Epsilon);
	}

	bool operator!=( const Vec2f &v ) const
	{
		return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon ||
				y < v.y - Math::Epsilon || y > v.y + Math::Epsilon);
	}

	// ---------------------
	// Arithmetic operations
	// ---------------------
	Vec2d operator-() const
	{
		return Vec2d( -x, -y );
	}

	Vec2d operator+( const Vec2d &v ) const
	{
		return Vec2d( x + v.x, y + v.y );
	}

	Vec2d operator+( const Vec2f &v ) const
	{
		return Vec2d( x + v.x, y + v.y );
	}

	Vec2d &operator+=( const Vec2d &v )
	{
		return *this = *this + v;
	}

	Vec2d &operator+=( const Vec2f &v )
	{
		return *this = *this + v;
	}

	Vec2d operator-( const Vec2d &v ) const
	{
		return Vec2d( x - v.x, y - v.y );
	}

	Vec2d operator-( const Vec2f &v ) const
	{
		return Vec2d( x - v.x, y - v.y );
	}

	Vec2d &operator-=( const Vec2d &v )
	{
		return *this = *this - v;
	}

	Vec2d &operator-=( const Vec2f &v )
	{
		return *this = *this - v;
	}

	Vec2d operator*( const double f ) const
	{
		return Vec2d( x * f, y * f );
	}

	Vec2d operator*( const float f ) const
	{
		return Vec2d( x * f, y * f );
	}

	Vec2d &operator*=( const double f )
	{
		return *this = *this * f;
	}

	Vec2d &operator*=( const float f )
	{
		return *this = *this * f;
	}

	Vec2d operator/( const double f ) const
	{
		return Vec2d( x / f, y / f );
	}

	Vec2d operator/( const float f ) const
	{
		return Vec2d( x / f, y / f );
	}

	Vec2d &operator/=( const double f )
	{
		return *this = *this / f;
	}

	Vec2d &operator/=( const float f )
	{
		return *this = *this / f;
	}

	// ----------------
	// Special products
	// ----------------
	double dot( const Vec2d &v ) const
	{
		return x * v.x + y * v.y;
	}

	double dot( const Vec2f &v ) const
	{
		return x * v.x + y * v.y;
	}

	// ----------------
	// Other operations
	// ----------------
	double length() const
	{
		return sqrt( x * x + y * y );
	}

	Vec2d normalized() const
	{
		double invLen = 1.0 / length();
		return Vec2d( x * invLen, y * invLen );
	}

	void normalize()
	{
		double invLen = 1.0 / length();
		x *= invLen;
		y *= invLen;
	}

	Vec2d lerp( const Vec2d &v, double f ) const
	{
		return Vec2d( x + (v.x - x) * f, y + (v.y - y) * f );
	}
};


class Vec3f
{
public:
	float x, y, z;
	
	
	// ------------
	// Constructors
	// ------------
	Vec3f() : x( 0.0f ), y( 0.0f ), z( 0.0f )
	{ 
	}

	explicit Vec3f( Math::NoInitHint )
	{
		// Constructor without default initialization
	}
	
	Vec3f( const float x, const float y, const float z ) : x( x ), y( y ), z( z ) 
	{
	}

	Vec3f( const Vec3f &v ) : x( v.x ), y( v.y ), z( v.z )
	{
	}

	// ------
	// Access
	// ------
	float operator[]( unsigned int index ) const
	{
		return *(&x + index);
	}

	float &operator[]( unsigned int index )
	{
		return *(&x + index);
	}
	
	// -----------
	// Comparisons
	// -----------
	bool operator==( const Vec3f &v ) const
	{
		return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon && 
				y > v.y - Math::Epsilon && y < v.y + Math::Epsilon &&
				z > v.z - Math::Epsilon && z < v.z + Math::Epsilon);
	}

	bool operator!=( const Vec3f &v ) const
	{
		return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon || 
				y < v.y - Math::Epsilon || y > v.y + Math::Epsilon ||
				z < v.z - Math::Epsilon || z > v.z + Math::Epsilon);
	}
	
	// ---------------------
	// Arithmetic operations
	// ---------------------
	Vec3f operator-() const
	{
		return Vec3f( -x, -y, -z );
	}

	Vec3f operator+( const Vec3f &v ) const
	{
		return Vec3f( x + v.x, y + v.y, z + v.z );
	}

	Vec3f &operator+=( const Vec3f &v )
	{
		return *this = *this + v;
	}

	Vec3f operator-( const Vec3f &v ) const 
	{
		return Vec3f( x - v.x, y - v.y, z - v.z );
	}

	Vec3f &operator-=( const Vec3f &v )
	{
		return *this = *this - v;
	}

	Vec3f operator*( const float f ) const
	{
		return Vec3f( x * f, y * f, z * f );
	}

	Vec3f &operator*=( const float f )
	{
		return *this = *this * f;
	}

	Vec3f operator/( const float f ) const
	{
		return Vec3f( x / f, y / f, z / f );
	}

	Vec3f &operator/=( const float f )
	{
		return *this = *this / f;
	}

	// ----------------
	// Special products
	// ----------------
	float dot( const Vec3f &v ) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vec3f cross( const Vec3f &v ) const
	{
		return Vec3f( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
	}

	// ----------------
	// Other operations
	// ----------------
	float length() const 
	{
		return sqrtf( x * x + y * y + z * z );
	}

	Vec3f normalized() const
	{
		float invLen = 1.0f / length();
		return Vec3f( x * invLen, y * invLen, z * invLen );
	}

	void normalize()
	{
		float invLen = 1.0f / length();
		x *= invLen;
		y *= invLen;
		z *= invLen;
	}

	/*void fromRotation( float angleX, float angleY )
	{
		x = cosf( angleX ) * sinf( angleY ); 
		y = -sinf( angleX );
		z = cosf( angleX ) * cosf( angleY );
	}*/

	Vec3f toRotation() const
	{
		// Assumes that the unrotated view vector is (0, 0, -1)
		Vec3f v;
		
		if( y != 0 ) v.x = atan2f( y, sqrtf( x*x + z*z ) );
		if( x != 0 || z != 0 ) v.y = atan2f( -x, -z );

		return v;
	}

	Vec3f lerp( const Vec3f &v, float f ) const
	{
		return Vec3f( x + (v.x - x) * f, y + (v.y - y) * f, z + (v.z - z) * f ); 
	}
};


class Vec3d
{
public:
	double x, y, z;


	// ------------
	// Constructors
	// ------------
	Vec3d() : x( 0.0 ), y( 0.0 ), z( 0.0 )
	{
	}

	explicit Vec3d( Math::NoInitHint )
	{
		// Constructor without default initialization
	}

	Vec3d( const double x, const double y, const double z ) : x( x ), y( y ), z( z )
	{
	}

	Vec3d( const Vec3d &v ) : x( v.x ), y( v.y ), z( v.z )
	{
	}

	explicit Vec3d( const Vec3f &v ) : x( v.x ), y( v.y ), z( v.z )
	{
	}

	Vec3d& operator=( const Vec3f& v )
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Vec3d& operator=( const Vec3d& v )
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	// ------
	// Access
	// ------
	double operator[]( unsigned int index ) const
	{
		return *(&x + index);
	}

	double &operator[]( unsigned int index )
	{
		return *(&x + index);
	}

	// -----------
	// Comparisons
	// -----------
	bool operator==( const Vec3d &v ) const
	{
		return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon &&
				y > v.y - Math::Epsilon && y < v.y + Math::Epsilon &&
				z > v.z - Math::Epsilon && z < v.z + Math::Epsilon);
	}

	bool operator!=( const Vec3d &v ) const
	{
		return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon ||
				y < v.y - Math::Epsilon || y > v.y + Math::Epsilon ||
				z < v.z - Math::Epsilon || z > v.z + Math::Epsilon);
	}

	// ---------------------
	// Arithmetic operations
	// ---------------------
	Vec3d operator-() const
	{
		return Vec3d( -x, -y, -z );
	}

	Vec3d operator+( const Vec3d &v ) const
	{
		return Vec3d( x + v.x, y + v.y, z + v.z );
	}

	Vec3d operator+( const Vec3f &v ) const
	{
		return Vec3d( x + v.x, y + v.y, z + v.z );
	}

	Vec3d &operator+=( const Vec3d &v )
	{
		return *this = *this + v;
	}

	Vec3d &operator+=( const Vec3f &v )
	{
		return *this = *this + v;
	}

	Vec3d operator-( const Vec3d &v ) const
	{
		return Vec3d( x - v.x, y - v.y, z - v.z );
	}

	Vec3d operator-( const Vec3f &v ) const
	{
		return Vec3d( x - v.x, y - v.y, z - v.z );
	}

	Vec3d &operator-=( const Vec3d &v )
	{
		return *this = *this - v;
	}

	Vec3d &operator-=( const Vec3f &v )
	{
		return *this = *this - v;
	}

	Vec3d operator*( const double f ) const
	{
		return Vec3d( x * f, y * f, z * f );
	}

	Vec3d operator*( const float f ) const
	{
		return Vec3d( x * f, y * f, z * f );
	}

	Vec3d &operator*=( const double f )
	{
		return *this = *this * f;
	}

	Vec3d &operator*=( const float f )
	{
		return *this = *this * f;
	}

	Vec3d operator/( const double f ) const
	{
		return Vec3d( x / f, y / f, z / f );
	}

	Vec3d operator/( const float f ) const
	{
		return Vec3d( x / f, y / f, z / f );
	}

	Vec3d &operator/=( const double f )
	{
		return *this = *this / f;
	}

	Vec3d &operator/=( const float f )
	{
		return *this = *this / f;
	}

	// ----------------
	// Special products
	// ----------------
	double dot( const Vec3d &v ) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	double dot( const Vec3f &v ) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vec3d cross( const Vec3d &v ) const
	{
		return Vec3d( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
	}

	Vec3d cross( const Vec3f &v ) const
	{
		return Vec3d( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
	}

	// ----------------
	// Other operations
	// ----------------
	double length() const
	{
		return sqrt( x * x + y * y + z * z );
	}

	Vec3d normalized() const
	{
		double invLen = 1.0 / length();
		return Vec3d( x * invLen, y * invLen, z * invLen );
	}

	void normalize()
	{
		double invLen = 1.0 / length();
		x *= invLen;
		y *= invLen;
		z *= invLen;
	}

	/*void fromRotation( double angleX, double angleY )
	{
		x = cos( angleX ) * sin( angleY );
		y = -sin( angleX );
		z = cos( angleX ) * cos( angleY );
	}*/

	Vec3d toRotation() const
	{
		// Assumes that the unrotated view vector is (0, 0, -1)
		Vec3d v;

		if( y != 0 ) v.x = atan2( y, sqrt( x*x + z*z ) );
		if( x != 0 || z != 0 ) v.y = atan2( -x, -z );

		return v;
	}

	Vec3d lerp( const Vec3d &v, double f ) const
	{
		return Vec3d( x + (v.x - x) * f, y + (v.y - y) * f, z + (v.z - z) * f );
	}

	Vec3f toVec3f() const
	{
		return Vec3f(
				static_cast<float>(x),
				static_cast<float>(y),
				static_cast<float>(z));
	}
};


class Vec4f
{
public:
	
	float x, y, z, w;


	Vec4f() : x( 0 ), y( 0 ), z( 0 ), w( 0 )
	{
	}
	
	explicit Vec4f( const float x, const float y, const float z, const float w ) :
		x( x ), y( y ), z( z ), w( w )
	{
	}

	explicit Vec4f( Vec3f v ) : x( v.x ), y( v.y ), z( v.z ), w( 1.0f )
	{
	}

	// ------
	// Access
	// ------
	float operator[]( unsigned int index ) const
	{
		return *(&x + index);
	}

	float &operator[]( unsigned int index )
	{
		return *(&x + index);
	}

	// ---------------------
	// Arithmetic operations
	// ---------------------
	Vec4f operator+( const Vec4f &v ) const
	{
		return Vec4f( x + v.x, y + v.y, z + v.z, w + v.w );
	}

	Vec4f operator-() const
	{
		return Vec4f( -x, -y, -z, -w );
	}
	
	Vec4f operator*( const float f ) const
	{
		return Vec4f( x * f, y * f, z * f, w * f );
	}
};


class Vec4d
{
public:

	double x, y, z, w;


	Vec4d() : x( 0 ), y( 0 ), z( 0 ), w( 0 )
	{
	}

	explicit Vec4d( const double x, const double y, const double z, const double w ) :
			x( x ), y( y ), z( z ), w( w )
	{
	}

	Vec4d( const Vec4d &v ) : x( v.x ), y( v.y ), z( v.z ), w( v.w )
	{
	}

	Vec4d( const Vec4f &v ) : x( v.x ), y( v.y ), z( v.z ), w( v.w )
	{
	}

	explicit Vec4d( Vec3d v ) : x( v.x ), y( v.y ), z( v.z ), w( 1.0 )
	{
	}

	explicit Vec4d( Vec3f v ) : x( v.x ), y( v.y ), z( v.z ), w( 1.0 )
	{
	}

	// ------
	// Access
	// ------
	double operator[]( unsigned int index ) const
	{
		return *(&x + index);
	}

	double &operator[]( unsigned int index )
	{
		return *(&x + index);
	}

	// ---------------------
	// Arithmetic operations
	// ---------------------
	Vec4d operator+( const Vec4d &v ) const
	{
		return Vec4d( x + v.x, y + v.y, z + v.z, w + v.w );
	}

	Vec4d operator+( const Vec4f &v ) const
	{
		return Vec4d( x + v.x, y + v.y, z + v.z, w + v.w );
	}

	Vec4d operator-() const
	{
		return Vec4d( -x, -y, -z, -w );
	}

	Vec4d operator*( const double f ) const
	{
		return Vec4d( x * f, y * f, z * f, w * f );
	}

	Vec4d operator*( const float f ) const
	{
		return Vec4d( x * f, y * f, z * f, w * f );
	}

	Vec4f toVec4f() const {
		return Vec4f(
				static_cast<float>(x),
				static_cast<float>(y),
				static_cast<float>(z),
				static_cast<float>(w));
	}
};

// -------------------------------------------------------------------------------------------------
// Quaternion
// -------------------------------------------------------------------------------------------------

class Quaternion
{
public:	
	
	float x, y, z, w;

	// ------------
	// Constructors
	// ------------
	Quaternion() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f ) 
	{ 
	}
	
	explicit Quaternion( const float x, const float y, const float z, const float w ) :
		x( x ), y( y ), z( z ), w( w )
	{
	}
	
	Quaternion( const float eulerX, const float eulerY, const float eulerZ )
	{
		Quaternion roll( sinf( eulerX * 0.5f ), 0, 0, cosf( eulerX * 0.5f ) );
		Quaternion pitch( 0, sinf( eulerY * 0.5f ), 0, cosf( eulerY * 0.5f ) );
		Quaternion yaw( 0, 0, sinf( eulerZ * 0.5f ), cosf( eulerZ * 0.5f ) );
	
		// Order: y * x * z
		*this = pitch * roll * yaw;
	}

	// ---------------------
	// Arithmetic operations
	// ---------------------
	Quaternion operator*( const Quaternion &q ) const
	{
		return Quaternion(
			y * q.z - z * q.y + q.x * w + x * q.w,
			z * q.x - x * q.z + q.y * w + y * q.w,
			x * q.y - y * q.x + q.z * w + z * q.w,
			w * q.w - (x * q.x + y * q.y + z * q.z) );
	}

	Quaternion &operator*=( const Quaternion &q )
	{
		return *this = *this * q;
	}

	// ----------------
	// Other operations
	// ----------------

	Quaternion slerp( const Quaternion &q, const float t ) const
	{
		// Spherical linear interpolation between two quaternions
		// Note: SLERP is not commutative
		
		Quaternion q1( q );

		// Calculate cosine
		float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;

		// Use the shortest path
		if( cosTheta < 0 )
		{
			cosTheta = -cosTheta; 
			q1.x = -q.x; q1.y = -q.y;
			q1.z = -q.z; q1.w = -q.w;
		}

		// Initialize with linear interpolation
		float scale0 = 1 - t, scale1 = t;
		
		// Use spherical interpolation only if the quaternions are not very close
		if( (1 - cosTheta) > 0.001f )
		{
			// SLERP
			float theta = acosf( cosTheta );
			float sinTheta = sinf( theta );
			scale0 = sinf( (1 - t) * theta ) / sinTheta;
			scale1 = sinf( t * theta ) / sinTheta;
		} 
		
		// Calculate final quaternion
		return Quaternion( x * scale0 + q1.x * scale1, y * scale0 + q1.y * scale1,
						   z * scale0 + q1.z * scale1, w * scale0 + q1.w * scale1 );
	}

	Quaternion nlerp( const Quaternion &q, const float t ) const
	{
		// Normalized linear quaternion interpolation
		// Note: NLERP is faster than SLERP and commutative but does not yield constant velocity

		Quaternion qt;
		float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;
		
		// Use the shortest path and interpolate linearly
		if( cosTheta < 0 )
			qt = Quaternion( x + (-q.x - x) * t, y + (-q.y - y) * t,
							 z + (-q.z - z) * t, w + (-q.w - w) * t );
		else
			qt = Quaternion( x + (q.x - x) * t, y + (q.y - y) * t,
							 z + (q.z - z) * t, w + (q.w - w) * t );

		// Return normalized quaternion
		float invLen = 1.0f / sqrtf( qt.x * qt.x + qt.y * qt.y + qt.z * qt.z + qt.w * qt.w );
		return Quaternion( qt.x * invLen, qt.y * invLen, qt.z * invLen, qt.w * invLen );
	}

	Quaternion inverted() const
	{
		float len = x * x + y * y + z * z + w * w;
		if( len > 0 )
		{
			float invLen = 1.0f / len;
			return Quaternion( -x * invLen, -y * invLen, -z * invLen, w * invLen );
		}
		else return Quaternion();
	}
};


class Quaterniond
{
public:

	double x, y, z, w;

	// ------------
	// Constructors
	// ------------
	Quaterniond() : x( 0.0 ), y( 0.0 ), z( 0.0 ), w( 0.0 )
	{
	}

	explicit Quaterniond( const double x, const double y, const double z, const double w ) :
			x( x ), y( y ), z( z ), w( w )
	{
	}

	Quaterniond( const Quaterniond &q ) : x( q.x ), y( q.y ), z( q.z ), w( q.w )
	{
	}

	Quaterniond( const Quaternion &q ) : x( q.x ), y( q.y ), z( q.z ), w( q.w )
	{
	}

	Quaterniond( const double eulerX, const double eulerY, const double eulerZ )
	{
		Quaterniond roll( sin( eulerX * 0.5 ), 0, 0, cos( eulerX * 0.5 ) );
		Quaterniond pitch( 0, sin( eulerY * 0.5 ), 0, cos( eulerY * 0.5 ) );
		Quaterniond yaw( 0, 0, sin( eulerZ * 0.5 ), cos( eulerZ * 0.5 ) );

		// Order: y * x * z
		*this = pitch * roll * yaw;
	}

	// ---------------------
	// Arithmetic operations
	// ---------------------
	Quaterniond operator*( const Quaterniond &q ) const
	{
		return Quaterniond(
				y * q.z - z * q.y + q.x * w + x * q.w,
				z * q.x - x * q.z + q.y * w + y * q.w,
				x * q.y - y * q.x + q.z * w + z * q.w,
				w * q.w - (x * q.x + y * q.y + z * q.z) );
	}

	Quaterniond operator*( const Quaternion &q ) const
	{
		return Quaterniond(
				y * q.z - z * q.y + q.x * w + x * q.w,
				z * q.x - x * q.z + q.y * w + y * q.w,
				x * q.y - y * q.x + q.z * w + z * q.w,
				w * q.w - (x * q.x + y * q.y + z * q.z) );
	}

	Quaterniond &operator*=( const Quaterniond &q )
	{
		return *this = *this * q;
	}

	Quaterniond &operator*=( const Quaternion &q )
	{
		return *this = *this * q;
	}

	// ----------------
	// Other operations
	// ----------------

	Quaterniond slerp( const Quaterniond &q, const double t ) const
	{
		// Spherical linear interpolation between two Quaternionds
		// Note: SLERP is not commutative

		Quaterniond q1( q );

		// Calculate cosine
		double cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;

		// Use the shortest path
		if( cosTheta < 0 )
		{
			cosTheta = -cosTheta;
			q1.x = -q.x; q1.y = -q.y;
			q1.z = -q.z; q1.w = -q.w;
		}

		// Initialize with linear interpolation
		double scale0 = 1 - t, scale1 = t;

		// Use spherical interpolation only if the Quaternionds are not very close
		if( (1 - cosTheta) > 0.001 )
		{
			// SLERP
			double theta = acos( cosTheta );
			double sinTheta = sin( theta );
			scale0 = sin( (1 - t) * theta ) / sinTheta;
			scale1 = sin( t * theta ) / sinTheta;
		}

		// Calculate final Quaterniond
		return Quaterniond( x * scale0 + q1.x * scale1, y * scale0 + q1.y * scale1,
							z * scale0 + q1.z * scale1, w * scale0 + q1.w * scale1 );
	}

	Quaterniond nlerp( const Quaterniond &q, const double t ) const
	{
		// Normalized linear Quaterniond interpolation
		// Note: NLERP is faster than SLERP and commutative but does not yield constant velocity

		Quaterniond qt;
		double cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;

		// Use the shortest path and interpolate linearly
		if( cosTheta < 0 )
			qt = Quaterniond( x + (-q.x - x) * t, y + (-q.y - y) * t,
							  z + (-q.z - z) * t, w + (-q.w - w) * t );
		else
			qt = Quaterniond( x + (q.x - x) * t, y + (q.y - y) * t,
							  z + (q.z - z) * t, w + (q.w - w) * t );

		// Return normalized Quaterniond
		double invLen = 1.0 / sqrt( qt.x * qt.x + qt.y * qt.y + qt.z * qt.z + qt.w * qt.w );
		return Quaterniond( qt.x * invLen, qt.y * invLen, qt.z * invLen, qt.w * invLen );
	}

	Quaterniond inverted() const
	{
		double len = x * x + y * y + z * z + w * w;
		if( len > 0 )
		{
			double invLen = 1.0 / len;
			return Quaterniond( -x * invLen, -y * invLen, -z * invLen, w * invLen );
		}
		else return Quaterniond();
	}
};


// -------------------------------------------------------------------------------------------------
// Matrix
// -------------------------------------------------------------------------------------------------

class Matrix4f
{
public:
	
	union
	{
		float c[4][4];	// Column major order for OpenGL: c[column][row]
		float x[16];
	};
	
	// --------------
	// Static methods
	// --------------
	static Matrix4f TransMat( float x, float y, float z )
	{
		Matrix4f m;

		m.c[3][0] = x;
		m.c[3][1] = y;
		m.c[3][2] = z;

		return m;
	}

	static Matrix4f ScaleMat( float x, float y, float z )
	{
		Matrix4f m;
		
		m.c[0][0] = x;
		m.c[1][1] = y;
		m.c[2][2] = z;

		return m;
	}

	static Matrix4f RotMat( float x, float y, float z )
	{
		// Rotation order: YXZ [* Vector]
		return Matrix4f( Quaternion( x, y, z ) );
	}

	static Matrix4f RotMat( Vec3f axis, float angle )
	{
		axis = axis * sinf( angle * 0.5f );
		return Matrix4f( Quaternion( axis.x, axis.y, axis.z, cosf( angle * 0.5f ) ) );
	}

	static Matrix4f PerspectiveMat( float l, float r, float b, float t, float n, float f )
	{
		Matrix4f m;

		m.x[0] = 2 * n / (r - l);
		m.x[5] = 2 * n / (t - b);
		m.x[8] = (r + l) / (r - l);
		m.x[9] = (t + b) / (t - b);
		m.x[10] = -(f + n) / (f - n);
		m.x[11] = -1;
		m.x[14] = -2 * f * n / (f - n);
		m.x[15] = 0;

		return m;
	}

	static Matrix4f OrthoMat( float l, float r, float b, float t, float n, float f )
	{
		Matrix4f m;

		m.x[0] = 2 / (r - l);
		m.x[5] = 2 / (t - b);
		m.x[10] = -2 / (f - n);
		m.x[12] = -(r + l) / (r - l);
		m.x[13] = -(t + b) / (t - b);
		m.x[14] = -(f + n) / (f - n);

		return m;
	}

	static void fastMult43( Matrix4f &dst, const Matrix4f &m1, const Matrix4f &m2 )
	{
		// Note: dst may not be the same as m1 or m2

		float *dstx = dst.x;
		const float *m1x = m1.x;
		const float *m2x = m2.x;
		
		dstx[0] = m1x[0] * m2x[0] + m1x[4] * m2x[1] + m1x[8] * m2x[2];
		dstx[1] = m1x[1] * m2x[0] + m1x[5] * m2x[1] + m1x[9] * m2x[2];
		dstx[2] = m1x[2] * m2x[0] + m1x[6] * m2x[1] + m1x[10] * m2x[2];
		dstx[3] = 0.0f;

		dstx[4] = m1x[0] * m2x[4] + m1x[4] * m2x[5] + m1x[8] * m2x[6];
		dstx[5] = m1x[1] * m2x[4] + m1x[5] * m2x[5] + m1x[9] * m2x[6];
		dstx[6] = m1x[2] * m2x[4] + m1x[6] * m2x[5] + m1x[10] * m2x[6];
		dstx[7] = 0.0f;

		dstx[8] = m1x[0] * m2x[8] + m1x[4] * m2x[9] + m1x[8] * m2x[10];
		dstx[9] = m1x[1] * m2x[8] + m1x[5] * m2x[9] + m1x[9] * m2x[10];
		dstx[10] = m1x[2] * m2x[8] + m1x[6] * m2x[9] + m1x[10] * m2x[10];
		dstx[11] = 0.0f;

		dstx[12] = m1x[0] * m2x[12] + m1x[4] * m2x[13] + m1x[8] * m2x[14] + m1x[12] * m2x[15];
		dstx[13] = m1x[1] * m2x[12] + m1x[5] * m2x[13] + m1x[9] * m2x[14] + m1x[13] * m2x[15];
		dstx[14] = m1x[2] * m2x[12] + m1x[6] * m2x[13] + m1x[10] * m2x[14] + m1x[14] * m2x[15];
		dstx[15] = 1.0f;
	}

	// ------------
	// Constructors
	// ------------
	Matrix4f()
	{
		c[0][0] = 1; c[1][0] = 0; c[2][0] = 0; c[3][0] = 0;
		c[0][1] = 0; c[1][1] = 1; c[2][1] = 0; c[3][1] = 0;
		c[0][2] = 0; c[1][2] = 0; c[2][2] = 1; c[3][2] = 0;
		c[0][3] = 0; c[1][3] = 0; c[2][3] = 0; c[3][3] = 1;
	}

	explicit Matrix4f( Math::NoInitHint )
	{
		// Constructor without default initialization
	}

	Matrix4f( const float *floatArray16 )
	{
		for( unsigned int i = 0; i < 4; ++i )
		{
			for( unsigned int j = 0; j < 4; ++j )
			{
				c[i][j] = floatArray16[i * 4 + j];
			}
		}
	}

	Matrix4f( const Quaternion &q )
	{
		// Calculate coefficients
		float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
		float xx = q.x * x2,  xy = q.x * y2,  xz = q.x * z2;
		float yy = q.y * y2,  yz = q.y * z2,  zz = q.z * z2;
		float wx = q.w * x2,  wy = q.w * y2,  wz = q.w * z2;

		c[0][0] = 1 - (yy + zz);  c[1][0] = xy - wz;	
		c[2][0] = xz + wy;        c[3][0] = 0;
		c[0][1] = xy + wz;        c[1][1] = 1 - (xx + zz);
		c[2][1] = yz - wx;        c[3][1] = 0;
		c[0][2] = xz - wy;        c[1][2] = yz + wx;
		c[2][2] = 1 - (xx + yy);  c[3][2] = 0;
		c[0][3] = 0;              c[1][3] = 0;
		c[2][3] = 0;              c[3][3] = 1;
	}

	// ----------
	// Matrix sum
	// ----------
	Matrix4f operator+( const Matrix4f &m ) const 
	{
		Matrix4f mf( Math::NO_INIT );
		
		mf.x[0] = x[0] + m.x[0];
		mf.x[1] = x[1] + m.x[1];
		mf.x[2] = x[2] + m.x[2];
		mf.x[3] = x[3] + m.x[3];
		mf.x[4] = x[4] + m.x[4];
		mf.x[5] = x[5] + m.x[5];
		mf.x[6] = x[6] + m.x[6];
		mf.x[7] = x[7] + m.x[7];
		mf.x[8] = x[8] + m.x[8];
		mf.x[9] = x[9] + m.x[9];
		mf.x[10] = x[10] + m.x[10];
		mf.x[11] = x[11] + m.x[11];
		mf.x[12] = x[12] + m.x[12];
		mf.x[13] = x[13] + m.x[13];
		mf.x[14] = x[14] + m.x[14];
		mf.x[15] = x[15] + m.x[15];

		return mf;
	}

	Matrix4f &operator+=( const Matrix4f &m )
	{
		return *this = *this + m;
	}

	// ---------------------
	// Matrix multiplication
	// ---------------------
	Matrix4f operator*( const Matrix4f &m ) const 
	{
		Matrix4f mf( Math::NO_INIT );
		
		mf.x[0] = x[0] * m.x[0] + x[4] * m.x[1] + x[8] * m.x[2] + x[12] * m.x[3];
		mf.x[1] = x[1] * m.x[0] + x[5] * m.x[1] + x[9] * m.x[2] + x[13] * m.x[3];
		mf.x[2] = x[2] * m.x[0] + x[6] * m.x[1] + x[10] * m.x[2] + x[14] * m.x[3];
		mf.x[3] = x[3] * m.x[0] + x[7] * m.x[1] + x[11] * m.x[2] + x[15] * m.x[3];

		mf.x[4] = x[0] * m.x[4] + x[4] * m.x[5] + x[8] * m.x[6] + x[12] * m.x[7];
		mf.x[5] = x[1] * m.x[4] + x[5] * m.x[5] + x[9] * m.x[6] + x[13] * m.x[7];
		mf.x[6] = x[2] * m.x[4] + x[6] * m.x[5] + x[10] * m.x[6] + x[14] * m.x[7];
		mf.x[7] = x[3] * m.x[4] + x[7] * m.x[5] + x[11] * m.x[6] + x[15] * m.x[7];

		mf.x[8] = x[0] * m.x[8] + x[4] * m.x[9] + x[8] * m.x[10] + x[12] * m.x[11];
		mf.x[9] = x[1] * m.x[8] + x[5] * m.x[9] + x[9] * m.x[10] + x[13] * m.x[11];
		mf.x[10] = x[2] * m.x[8] + x[6] * m.x[9] + x[10] * m.x[10] + x[14] * m.x[11];
		mf.x[11] = x[3] * m.x[8] + x[7] * m.x[9] + x[11] * m.x[10] + x[15] * m.x[11];

		mf.x[12] = x[0] * m.x[12] + x[4] * m.x[13] + x[8] * m.x[14] + x[12] * m.x[15];
		mf.x[13] = x[1] * m.x[12] + x[5] * m.x[13] + x[9] * m.x[14] + x[13] * m.x[15];
		mf.x[14] = x[2] * m.x[12] + x[6] * m.x[13] + x[10] * m.x[14] + x[14] * m.x[15];
		mf.x[15] = x[3] * m.x[12] + x[7] * m.x[13] + x[11] * m.x[14] + x[15] * m.x[15];

		return mf;
	}

	Matrix4f operator*( const float f ) const
	{
		Matrix4f m( *this );
		
		m.x[0]  *= f; m.x[1]  *= f; m.x[2]  *= f; m.x[3]  *= f;
		m.x[4]  *= f; m.x[5]  *= f; m.x[6]  *= f; m.x[7]  *= f;
		m.x[8]  *= f; m.x[9]  *= f; m.x[10] *= f; m.x[11] *= f;
		m.x[12] *= f; m.x[13] *= f; m.x[14] *= f; m.x[15] *= f;

		return m;
	}

	// ----------------------------
	// Vector-Matrix multiplication
	// ----------------------------
	Vec3f operator*( const Vec3f &v ) const
	{
		return Vec3f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + c[3][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + c[3][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + c[3][2] );
	}

	Vec4f operator*( const Vec4f &v ) const
	{
		return Vec4f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + v.w * c[3][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + v.w * c[3][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + v.w * c[3][2],
					  v.x * c[0][3] + v.y * c[1][3] + v.z * c[2][3] + v.w * c[3][3] );
	}

	Vec3f mult33Vec( const Vec3f &v ) const
	{
		return Vec3f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] );
	}
	
	// ---------------
	// Transformations
	// ---------------
	void translate( const float tx, const float ty, const float tz )
	{
		*this = TransMat( tx, ty, tz ) * *this;
	}

	void scale( const float sx, const float sy, const float sz )
	{
		*this = ScaleMat( sx, sy, sz ) * *this;
	}

	void rotate( const float rx, const float ry, const float rz )
	{
		*this = RotMat( rx, ry, rz ) * *this;
	}

	// ---------------
	// Other
	// ---------------

	Matrix4f transposed() const
	{
		Matrix4f m( *this );
		
		for( unsigned int maty = 0; maty < 4; ++maty )
		{
			for( unsigned int matx = maty + 1; matx < 4; ++matx ) 
			{
				float tmp = m.c[matx][maty];
				m.c[matx][maty] = m.c[maty][matx];
				m.c[maty][matx] = tmp;
			}
		}

		return m;
	}

	float determinant() const
	{
		return 
			c[0][3]*c[1][2]*c[2][1]*c[3][0] - c[0][2]*c[1][3]*c[2][1]*c[3][0] - c[0][3]*c[1][1]*c[2][2]*c[3][0] + c[0][1]*c[1][3]*c[2][2]*c[3][0] +
			c[0][2]*c[1][1]*c[2][3]*c[3][0] - c[0][1]*c[1][2]*c[2][3]*c[3][0] - c[0][3]*c[1][2]*c[2][0]*c[3][1] + c[0][2]*c[1][3]*c[2][0]*c[3][1] +
			c[0][3]*c[1][0]*c[2][2]*c[3][1] - c[0][0]*c[1][3]*c[2][2]*c[3][1] - c[0][2]*c[1][0]*c[2][3]*c[3][1] + c[0][0]*c[1][2]*c[2][3]*c[3][1] +
			c[0][3]*c[1][1]*c[2][0]*c[3][2] - c[0][1]*c[1][3]*c[2][0]*c[3][2] - c[0][3]*c[1][0]*c[2][1]*c[3][2] + c[0][0]*c[1][3]*c[2][1]*c[3][2] +
			c[0][1]*c[1][0]*c[2][3]*c[3][2] - c[0][0]*c[1][1]*c[2][3]*c[3][2] - c[0][2]*c[1][1]*c[2][0]*c[3][3] + c[0][1]*c[1][2]*c[2][0]*c[3][3] +
			c[0][2]*c[1][0]*c[2][1]*c[3][3] - c[0][0]*c[1][2]*c[2][1]*c[3][3] - c[0][1]*c[1][0]*c[2][2]*c[3][3] + c[0][0]*c[1][1]*c[2][2]*c[3][3];
	}

	Matrix4f inverted() const
	{
		Matrix4f m( Math::NO_INIT );

		float d = determinant();
		if( d == 0 ) return m;
		d = 1.0f / d;
		
		m.c[0][0] = d * (c[1][2]*c[2][3]*c[3][1] - c[1][3]*c[2][2]*c[3][1] + c[1][3]*c[2][1]*c[3][2] - c[1][1]*c[2][3]*c[3][2] - c[1][2]*c[2][1]*c[3][3] + c[1][1]*c[2][2]*c[3][3]);
		m.c[0][1] = d * (c[0][3]*c[2][2]*c[3][1] - c[0][2]*c[2][3]*c[3][1] - c[0][3]*c[2][1]*c[3][2] + c[0][1]*c[2][3]*c[3][2] + c[0][2]*c[2][1]*c[3][3] - c[0][1]*c[2][2]*c[3][3]);
		m.c[0][2] = d * (c[0][2]*c[1][3]*c[3][1] - c[0][3]*c[1][2]*c[3][1] + c[0][3]*c[1][1]*c[3][2] - c[0][1]*c[1][3]*c[3][2] - c[0][2]*c[1][1]*c[3][3] + c[0][1]*c[1][2]*c[3][3]);
		m.c[0][3] = d * (c[0][3]*c[1][2]*c[2][1] - c[0][2]*c[1][3]*c[2][1] - c[0][3]*c[1][1]*c[2][2] + c[0][1]*c[1][3]*c[2][2] + c[0][2]*c[1][1]*c[2][3] - c[0][1]*c[1][2]*c[2][3]);
		m.c[1][0] = d * (c[1][3]*c[2][2]*c[3][0] - c[1][2]*c[2][3]*c[3][0] - c[1][3]*c[2][0]*c[3][2] + c[1][0]*c[2][3]*c[3][2] + c[1][2]*c[2][0]*c[3][3] - c[1][0]*c[2][2]*c[3][3]);
		m.c[1][1] = d * (c[0][2]*c[2][3]*c[3][0] - c[0][3]*c[2][2]*c[3][0] + c[0][3]*c[2][0]*c[3][2] - c[0][0]*c[2][3]*c[3][2] - c[0][2]*c[2][0]*c[3][3] + c[0][0]*c[2][2]*c[3][3]);
		m.c[1][2] = d * (c[0][3]*c[1][2]*c[3][0] - c[0][2]*c[1][3]*c[3][0] - c[0][3]*c[1][0]*c[3][2] + c[0][0]*c[1][3]*c[3][2] + c[0][2]*c[1][0]*c[3][3] - c[0][0]*c[1][2]*c[3][3]);
		m.c[1][3] = d * (c[0][2]*c[1][3]*c[2][0] - c[0][3]*c[1][2]*c[2][0] + c[0][3]*c[1][0]*c[2][2] - c[0][0]*c[1][3]*c[2][2] - c[0][2]*c[1][0]*c[2][3] + c[0][0]*c[1][2]*c[2][3]);
		m.c[2][0] = d * (c[1][1]*c[2][3]*c[3][0] - c[1][3]*c[2][1]*c[3][0] + c[1][3]*c[2][0]*c[3][1] - c[1][0]*c[2][3]*c[3][1] - c[1][1]*c[2][0]*c[3][3] + c[1][0]*c[2][1]*c[3][3]);
		m.c[2][1] = d * (c[0][3]*c[2][1]*c[3][0] - c[0][1]*c[2][3]*c[3][0] - c[0][3]*c[2][0]*c[3][1] + c[0][0]*c[2][3]*c[3][1] + c[0][1]*c[2][0]*c[3][3] - c[0][0]*c[2][1]*c[3][3]);
		m.c[2][2] = d * (c[0][1]*c[1][3]*c[3][0] - c[0][3]*c[1][1]*c[3][0] + c[0][3]*c[1][0]*c[3][1] - c[0][0]*c[1][3]*c[3][1] - c[0][1]*c[1][0]*c[3][3] + c[0][0]*c[1][1]*c[3][3]);
		m.c[2][3] = d * (c[0][3]*c[1][1]*c[2][0] - c[0][1]*c[1][3]*c[2][0] - c[0][3]*c[1][0]*c[2][1] + c[0][0]*c[1][3]*c[2][1] + c[0][1]*c[1][0]*c[2][3] - c[0][0]*c[1][1]*c[2][3]);
		m.c[3][0] = d * (c[1][2]*c[2][1]*c[3][0] - c[1][1]*c[2][2]*c[3][0] - c[1][2]*c[2][0]*c[3][1] + c[1][0]*c[2][2]*c[3][1] + c[1][1]*c[2][0]*c[3][2] - c[1][0]*c[2][1]*c[3][2]);
		m.c[3][1] = d * (c[0][1]*c[2][2]*c[3][0] - c[0][2]*c[2][1]*c[3][0] + c[0][2]*c[2][0]*c[3][1] - c[0][0]*c[2][2]*c[3][1] - c[0][1]*c[2][0]*c[3][2] + c[0][0]*c[2][1]*c[3][2]);
		m.c[3][2] = d * (c[0][2]*c[1][1]*c[3][0] - c[0][1]*c[1][2]*c[3][0] - c[0][2]*c[1][0]*c[3][1] + c[0][0]*c[1][2]*c[3][1] + c[0][1]*c[1][0]*c[3][2] - c[0][0]*c[1][1]*c[3][2]);
		m.c[3][3] = d * (c[0][1]*c[1][2]*c[2][0] - c[0][2]*c[1][1]*c[2][0] + c[0][2]*c[1][0]*c[2][1] - c[0][0]*c[1][2]*c[2][1] - c[0][1]*c[1][0]*c[2][2] + c[0][0]*c[1][1]*c[2][2]);
		
		return m;
	}

	void decompose( Vec3f &trans, Vec3f &rot, Vec3f &scale ) const
	{
		// Getting translation is trivial
		trans = Vec3f( c[3][0], c[3][1], c[3][2] );

		// Scale is length of columns
		scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );

		if( scale.x == 0 || scale.y == 0 || scale.z == 0 ) return;

		// Detect negative scale with determinant and flip one arbitrary axis
		if( determinant() < 0 ) scale.x = -scale.x;

		// Combined rotation matrix YXZ
		//
		// Cos[y]*Cos[z]+Sin[x]*Sin[y]*Sin[z]   Cos[z]*Sin[x]*Sin[y]-Cos[y]*Sin[z]  Cos[x]*Sin[y]	
		// Cos[x]*Sin[z]                        Cos[x]*Cos[z]                       -Sin[x]
		// -Cos[z]*Sin[y]+Cos[y]*Sin[x]*Sin[z]  Cos[y]*Cos[z]*Sin[x]+Sin[y]*Sin[z]  Cos[x]*Cos[y]

		rot.x = asinf( -c[2][1] / scale.z );
		
		// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
		float f = fabsf( c[2][1] / scale.z );
		if( f > 0.999f && f < 1.001f )
		{
			// Pin arbitrarily one of y or z to zero
			// Mathematical equivalent of gimbal lock
			rot.y = 0;
			
			// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
			// => m[0][0] = Cos[z] and m[1][0] = Sin[z]
			rot.z = atan2f( -c[1][0] / scale.y, c[0][0] / scale.x );
		}
		// Standard case
		else
		{
			rot.y = atan2f( c[2][0] / scale.z, c[2][2] / scale.z );
			rot.z = atan2f( c[0][1] / scale.x, c[1][1] / scale.y );
		}
	}

	
	void setCol( unsigned int col, const Vec4f& v ) 
	{
		x[col * 4 + 0] = v.x;
		x[col * 4 + 1] = v.y;
		x[col * 4 + 2] = v.z;
		x[col * 4 + 3] = v.w;
	}

	Vec4f getCol( unsigned int col ) const
	{
		return Vec4f( x[col * 4 + 0], x[col * 4 + 1], x[col * 4 + 2], x[col * 4 + 3] );
	}

	Vec4f getRow( unsigned int row ) const
	{
		return Vec4f( x[row + 0], x[row + 4], x[row + 8], x[row + 12] );
	}

	Vec3f getTrans() const
	{
		return Vec3f( c[3][0], c[3][1], c[3][2] );
	}
	
	Vec3f getScale() const
	{
		Vec3f scale;
		// Scale is length of columns
		scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );
		return scale;
	}
};


class Matrix4d
{
public:

	union
	{
		double c[4][4];	// Column major order for OpenGL: c[column][row]
		double x[16];
	};

	// --------------
	// Static methods
	// --------------
	static Matrix4d TransMat( double x, double y, double z )
	{
		Matrix4d m;

		m.c[3][0] = x;
		m.c[3][1] = y;
		m.c[3][2] = z;

		return m;
	}

	static Matrix4d ScaleMat( double x, double y, double z )
	{
		Matrix4d m;

		m.c[0][0] = x;
		m.c[1][1] = y;
		m.c[2][2] = z;

		return m;
	}

	static Matrix4d RotMat( double x, double y, double z )
	{
		// Rotation order: YXZ [* Vector]
		return Matrix4d( Quaterniond( x, y, z ) );
	}

	static Matrix4d RotMat( Vec3d axis, double angle )
	{
		axis = axis * sin( angle * 0.5 );
		return Matrix4d( Quaterniond( axis.x, axis.y, axis.z, cos( angle * 0.5 ) ) );
	}

	static Matrix4d PerspectiveMat( double l, double r, double b, double t, double n, double f )
	{
		Matrix4d m;

		m.x[0] = 2 * n / (r - l);
		m.x[5] = 2 * n / (t - b);
		m.x[8] = (r + l) / (r - l);
		m.x[9] = (t + b) / (t - b);
		m.x[10] = -(f + n) / (f - n);
		m.x[11] = -1;
		m.x[14] = -2 * f * n / (f - n);
		m.x[15] = 0;

		return m;
	}

	static Matrix4d OrthoMat( double l, double r, double b, double t, double n, double f )
	{
		Matrix4d m;

		m.x[0] = 2 / (r - l);
		m.x[5] = 2 / (t - b);
		m.x[10] = -2 / (f - n);
		m.x[12] = -(r + l) / (r - l);
		m.x[13] = -(t + b) / (t - b);
		m.x[14] = -(f + n) / (f - n);

		return m;
	}

	static void fastMult43( Matrix4d &dst, const Matrix4d &m1, const Matrix4d &m2 )
	{
		// Note: dst may not be the same as m1 or m2

		double *dstx = dst.x;
		const double *m1x = m1.x;
		const double *m2x = m2.x;

		dstx[0] = m1x[0] * m2x[0] + m1x[4] * m2x[1] + m1x[8] * m2x[2];
		dstx[1] = m1x[1] * m2x[0] + m1x[5] * m2x[1] + m1x[9] * m2x[2];
		dstx[2] = m1x[2] * m2x[0] + m1x[6] * m2x[1] + m1x[10] * m2x[2];
		dstx[3] = 0.0;

		dstx[4] = m1x[0] * m2x[4] + m1x[4] * m2x[5] + m1x[8] * m2x[6];
		dstx[5] = m1x[1] * m2x[4] + m1x[5] * m2x[5] + m1x[9] * m2x[6];
		dstx[6] = m1x[2] * m2x[4] + m1x[6] * m2x[5] + m1x[10] * m2x[6];
		dstx[7] = 0.0;

		dstx[8] = m1x[0] * m2x[8] + m1x[4] * m2x[9] + m1x[8] * m2x[10];
		dstx[9] = m1x[1] * m2x[8] + m1x[5] * m2x[9] + m1x[9] * m2x[10];
		dstx[10] = m1x[2] * m2x[8] + m1x[6] * m2x[9] + m1x[10] * m2x[10];
		dstx[11] = 0.0;

		dstx[12] = m1x[0] * m2x[12] + m1x[4] * m2x[13] + m1x[8] * m2x[14] + m1x[12] * m2x[15];
		dstx[13] = m1x[1] * m2x[12] + m1x[5] * m2x[13] + m1x[9] * m2x[14] + m1x[13] * m2x[15];
		dstx[14] = m1x[2] * m2x[12] + m1x[6] * m2x[13] + m1x[10] * m2x[14] + m1x[14] * m2x[15];
		dstx[15] = 1.0;
	}

	// ------------
	// Constructors
	// ------------
	Matrix4d()
	{
		c[0][0] = 1; c[1][0] = 0; c[2][0] = 0; c[3][0] = 0;
		c[0][1] = 0; c[1][1] = 1; c[2][1] = 0; c[3][1] = 0;
		c[0][2] = 0; c[1][2] = 0; c[2][2] = 1; c[3][2] = 0;
		c[0][3] = 0; c[1][3] = 0; c[2][3] = 0; c[3][3] = 1;
	}

	explicit Matrix4d( Math::NoInitHint )
	{
		// Constructor without default initialization
	}

	Matrix4d( const double *doubleArray16 )
	{
		for( unsigned int i = 0; i < 4; ++i )
		{
			for( unsigned int j = 0; j < 4; ++j )
			{
				c[i][j] = doubleArray16[i * 4 + j];
			}
		}
	}

	Matrix4d( const float *floatArray16 )
	{
		for( unsigned int i = 0; i < 4; ++i )
		{
			for( unsigned int j = 0; j < 4; ++j )
			{
				c[i][j] = floatArray16[i * 4 + j];
			}
		}
	}

	Matrix4d( const Quaterniond &q )
	{
		// Calculate coefficients
		double x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
		double xx = q.x * x2,  xy = q.x * y2,  xz = q.x * z2;
		double yy = q.y * y2,  yz = q.y * z2,  zz = q.z * z2;
		double wx = q.w * x2,  wy = q.w * y2,  wz = q.w * z2;

		c[0][0] = 1 - (yy + zz);  c[1][0] = xy - wz;
		c[2][0] = xz + wy;        c[3][0] = 0;
		c[0][1] = xy + wz;        c[1][1] = 1 - (xx + zz);
		c[2][1] = yz - wx;        c[3][1] = 0;
		c[0][2] = xz - wy;        c[1][2] = yz + wx;
		c[2][2] = 1 - (xx + yy);  c[3][2] = 0;
		c[0][3] = 0;              c[1][3] = 0;
		c[2][3] = 0;              c[3][3] = 1;
	}

	Matrix4d( const Quaternion &q )
		: Matrix4d(Quaterniond(q))
	{
	}

	Matrix4d( const Matrix4d &m )
	{
		x[0] = m.x[0];
		x[1] = m.x[1];
		x[2] = m.x[2];
		x[3] = m.x[3];
		x[4] = m.x[4];
		x[5] = m.x[5];
		x[6] = m.x[6];
		x[7] = m.x[7];
		x[8] = m.x[8];
		x[9] = m.x[9];
		x[10] = m.x[10];
		x[11] = m.x[11];
		x[12] = m.x[12];
		x[13] = m.x[13];
		x[14] = m.x[14];
		x[15] = m.x[15];
	}

	Matrix4d( const Matrix4f &m )
	{
		x[0] = m.x[0];
		x[1] = m.x[1];
		x[2] = m.x[2];
		x[3] = m.x[3];
		x[4] = m.x[4];
		x[5] = m.x[5];
		x[6] = m.x[6];
		x[7] = m.x[7];
		x[8] = m.x[8];
		x[9] = m.x[9];
		x[10] = m.x[10];
		x[11] = m.x[11];
		x[12] = m.x[12];
		x[13] = m.x[13];
		x[14] = m.x[14];
		x[15] = m.x[15];
	}

	// ----------
	// Matrix sum
	// ----------
	Matrix4d operator+( const Matrix4d &m ) const
	{
		Matrix4d mf( Math::NO_INIT );

		mf.x[0] = x[0] + m.x[0];
		mf.x[1] = x[1] + m.x[1];
		mf.x[2] = x[2] + m.x[2];
		mf.x[3] = x[3] + m.x[3];
		mf.x[4] = x[4] + m.x[4];
		mf.x[5] = x[5] + m.x[5];
		mf.x[6] = x[6] + m.x[6];
		mf.x[7] = x[7] + m.x[7];
		mf.x[8] = x[8] + m.x[8];
		mf.x[9] = x[9] + m.x[9];
		mf.x[10] = x[10] + m.x[10];
		mf.x[11] = x[11] + m.x[11];
		mf.x[12] = x[12] + m.x[12];
		mf.x[13] = x[13] + m.x[13];
		mf.x[14] = x[14] + m.x[14];
		mf.x[15] = x[15] + m.x[15];

		return mf;
	}

	Matrix4d operator+( const Matrix4f &m ) const
	{
		Matrix4d mf( Math::NO_INIT );

		mf.x[0] = x[0] + m.x[0];
		mf.x[1] = x[1] + m.x[1];
		mf.x[2] = x[2] + m.x[2];
		mf.x[3] = x[3] + m.x[3];
		mf.x[4] = x[4] + m.x[4];
		mf.x[5] = x[5] + m.x[5];
		mf.x[6] = x[6] + m.x[6];
		mf.x[7] = x[7] + m.x[7];
		mf.x[8] = x[8] + m.x[8];
		mf.x[9] = x[9] + m.x[9];
		mf.x[10] = x[10] + m.x[10];
		mf.x[11] = x[11] + m.x[11];
		mf.x[12] = x[12] + m.x[12];
		mf.x[13] = x[13] + m.x[13];
		mf.x[14] = x[14] + m.x[14];
		mf.x[15] = x[15] + m.x[15];

		return mf;
	}

	Matrix4d &operator+=( const Matrix4d &m )
	{
		return *this = *this + m;
	}

	// ---------------------
	// Matrix multiplication
	// ---------------------
	Matrix4d operator*( const Matrix4d &m ) const
	{
		Matrix4d mf( Math::NO_INIT );

		mf.x[0] = x[0] * m.x[0] + x[4] * m.x[1] + x[8] * m.x[2] + x[12] * m.x[3];
		mf.x[1] = x[1] * m.x[0] + x[5] * m.x[1] + x[9] * m.x[2] + x[13] * m.x[3];
		mf.x[2] = x[2] * m.x[0] + x[6] * m.x[1] + x[10] * m.x[2] + x[14] * m.x[3];
		mf.x[3] = x[3] * m.x[0] + x[7] * m.x[1] + x[11] * m.x[2] + x[15] * m.x[3];

		mf.x[4] = x[0] * m.x[4] + x[4] * m.x[5] + x[8] * m.x[6] + x[12] * m.x[7];
		mf.x[5] = x[1] * m.x[4] + x[5] * m.x[5] + x[9] * m.x[6] + x[13] * m.x[7];
		mf.x[6] = x[2] * m.x[4] + x[6] * m.x[5] + x[10] * m.x[6] + x[14] * m.x[7];
		mf.x[7] = x[3] * m.x[4] + x[7] * m.x[5] + x[11] * m.x[6] + x[15] * m.x[7];

		mf.x[8] = x[0] * m.x[8] + x[4] * m.x[9] + x[8] * m.x[10] + x[12] * m.x[11];
		mf.x[9] = x[1] * m.x[8] + x[5] * m.x[9] + x[9] * m.x[10] + x[13] * m.x[11];
		mf.x[10] = x[2] * m.x[8] + x[6] * m.x[9] + x[10] * m.x[10] + x[14] * m.x[11];
		mf.x[11] = x[3] * m.x[8] + x[7] * m.x[9] + x[11] * m.x[10] + x[15] * m.x[11];

		mf.x[12] = x[0] * m.x[12] + x[4] * m.x[13] + x[8] * m.x[14] + x[12] * m.x[15];
		mf.x[13] = x[1] * m.x[12] + x[5] * m.x[13] + x[9] * m.x[14] + x[13] * m.x[15];
		mf.x[14] = x[2] * m.x[12] + x[6] * m.x[13] + x[10] * m.x[14] + x[14] * m.x[15];
		mf.x[15] = x[3] * m.x[12] + x[7] * m.x[13] + x[11] * m.x[14] + x[15] * m.x[15];

		return mf;
	}

	Matrix4d operator*( const Matrix4f &m ) const
	{
		Matrix4d mf( Math::NO_INIT );

		mf.x[0] = x[0] * m.x[0] + x[4] * m.x[1] + x[8] * m.x[2] + x[12] * m.x[3];
		mf.x[1] = x[1] * m.x[0] + x[5] * m.x[1] + x[9] * m.x[2] + x[13] * m.x[3];
		mf.x[2] = x[2] * m.x[0] + x[6] * m.x[1] + x[10] * m.x[2] + x[14] * m.x[3];
		mf.x[3] = x[3] * m.x[0] + x[7] * m.x[1] + x[11] * m.x[2] + x[15] * m.x[3];

		mf.x[4] = x[0] * m.x[4] + x[4] * m.x[5] + x[8] * m.x[6] + x[12] * m.x[7];
		mf.x[5] = x[1] * m.x[4] + x[5] * m.x[5] + x[9] * m.x[6] + x[13] * m.x[7];
		mf.x[6] = x[2] * m.x[4] + x[6] * m.x[5] + x[10] * m.x[6] + x[14] * m.x[7];
		mf.x[7] = x[3] * m.x[4] + x[7] * m.x[5] + x[11] * m.x[6] + x[15] * m.x[7];

		mf.x[8] = x[0] * m.x[8] + x[4] * m.x[9] + x[8] * m.x[10] + x[12] * m.x[11];
		mf.x[9] = x[1] * m.x[8] + x[5] * m.x[9] + x[9] * m.x[10] + x[13] * m.x[11];
		mf.x[10] = x[2] * m.x[8] + x[6] * m.x[9] + x[10] * m.x[10] + x[14] * m.x[11];
		mf.x[11] = x[3] * m.x[8] + x[7] * m.x[9] + x[11] * m.x[10] + x[15] * m.x[11];

		mf.x[12] = x[0] * m.x[12] + x[4] * m.x[13] + x[8] * m.x[14] + x[12] * m.x[15];
		mf.x[13] = x[1] * m.x[12] + x[5] * m.x[13] + x[9] * m.x[14] + x[13] * m.x[15];
		mf.x[14] = x[2] * m.x[12] + x[6] * m.x[13] + x[10] * m.x[14] + x[14] * m.x[15];
		mf.x[15] = x[3] * m.x[12] + x[7] * m.x[13] + x[11] * m.x[14] + x[15] * m.x[15];

		return mf;
	}

	Matrix4d operator*( const double f ) const
	{
		Matrix4d m( *this );

		m.x[0]  *= f; m.x[1]  *= f; m.x[2]  *= f; m.x[3]  *= f;
		m.x[4]  *= f; m.x[5]  *= f; m.x[6]  *= f; m.x[7]  *= f;
		m.x[8]  *= f; m.x[9]  *= f; m.x[10] *= f; m.x[11] *= f;
		m.x[12] *= f; m.x[13] *= f; m.x[14] *= f; m.x[15] *= f;

		return m;
	}

	Matrix4d operator*( const float f ) const
	{
		Matrix4d m( *this );

		m.x[0]  *= f; m.x[1]  *= f; m.x[2]  *= f; m.x[3]  *= f;
		m.x[4]  *= f; m.x[5]  *= f; m.x[6]  *= f; m.x[7]  *= f;
		m.x[8]  *= f; m.x[9]  *= f; m.x[10] *= f; m.x[11] *= f;
		m.x[12] *= f; m.x[13] *= f; m.x[14] *= f; m.x[15] *= f;

		return m;
	}

	// ----------------------------
	// Vector-Matrix multiplication
	// ----------------------------
	Vec3d operator*( const Vec3d &v ) const
	{
		return Vec3d( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + c[3][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + c[3][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + c[3][2] );
	}

	Vec3d operator*( const Vec3f &v ) const
	{
		return Vec3d( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + c[3][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + c[3][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + c[3][2] );
	}

	Vec4d operator*( const Vec4d &v ) const
	{
		return Vec4d( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + v.w * c[3][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + v.w * c[3][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + v.w * c[3][2],
					  v.x * c[0][3] + v.y * c[1][3] + v.z * c[2][3] + v.w * c[3][3] );
	}

	Vec4d operator*( const Vec4f &v ) const
	{
		return Vec4d( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + v.w * c[3][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + v.w * c[3][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + v.w * c[3][2],
					  v.x * c[0][3] + v.y * c[1][3] + v.z * c[2][3] + v.w * c[3][3] );
	}

	Vec3d mult33Vec( const Vec3d &v ) const
	{
		return Vec3d( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0],
					  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1],
					  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] );
	}

	// ---------------
	// Transformations
	// ---------------
	void translate( const double tx, const double ty, const double tz )
	{
		*this = TransMat( tx, ty, tz ) * *this;
	}

	void scale( const double sx, const double sy, const double sz )
	{
		*this = ScaleMat( sx, sy, sz ) * *this;
	}

	void rotate( const double rx, const double ry, const double rz )
	{
		*this = RotMat( rx, ry, rz ) * *this;
	}

	// ---------------
	// Other
	// ---------------

	Matrix4d transposed() const
	{
		Matrix4d m( *this );

		for( unsigned int maty = 0; maty < 4; ++maty )
		{
			for( unsigned int matx = maty + 1; matx < 4; ++matx )
			{
				double tmp = m.c[matx][maty];
				m.c[matx][maty] = m.c[maty][matx];
				m.c[maty][matx] = tmp;
			}
		}

		return m;
	}

	double determinant() const
	{
		return
				c[0][3]*c[1][2]*c[2][1]*c[3][0] - c[0][2]*c[1][3]*c[2][1]*c[3][0] - c[0][3]*c[1][1]*c[2][2]*c[3][0] + c[0][1]*c[1][3]*c[2][2]*c[3][0] +
				c[0][2]*c[1][1]*c[2][3]*c[3][0] - c[0][1]*c[1][2]*c[2][3]*c[3][0] - c[0][3]*c[1][2]*c[2][0]*c[3][1] + c[0][2]*c[1][3]*c[2][0]*c[3][1] +
				c[0][3]*c[1][0]*c[2][2]*c[3][1] - c[0][0]*c[1][3]*c[2][2]*c[3][1] - c[0][2]*c[1][0]*c[2][3]*c[3][1] + c[0][0]*c[1][2]*c[2][3]*c[3][1] +
				c[0][3]*c[1][1]*c[2][0]*c[3][2] - c[0][1]*c[1][3]*c[2][0]*c[3][2] - c[0][3]*c[1][0]*c[2][1]*c[3][2] + c[0][0]*c[1][3]*c[2][1]*c[3][2] +
				c[0][1]*c[1][0]*c[2][3]*c[3][2] - c[0][0]*c[1][1]*c[2][3]*c[3][2] - c[0][2]*c[1][1]*c[2][0]*c[3][3] + c[0][1]*c[1][2]*c[2][0]*c[3][3] +
				c[0][2]*c[1][0]*c[2][1]*c[3][3] - c[0][0]*c[1][2]*c[2][1]*c[3][3] - c[0][1]*c[1][0]*c[2][2]*c[3][3] + c[0][0]*c[1][1]*c[2][2]*c[3][3];
	}

	Matrix4d inverted() const
	{
		Matrix4d m( Math::NO_INIT );

		double d = determinant();
		if( d == 0 ) return m;
		d = 1.0 / d;

		m.c[0][0] = d * (c[1][2]*c[2][3]*c[3][1] - c[1][3]*c[2][2]*c[3][1] + c[1][3]*c[2][1]*c[3][2] - c[1][1]*c[2][3]*c[3][2] - c[1][2]*c[2][1]*c[3][3] + c[1][1]*c[2][2]*c[3][3]);
		m.c[0][1] = d * (c[0][3]*c[2][2]*c[3][1] - c[0][2]*c[2][3]*c[3][1] - c[0][3]*c[2][1]*c[3][2] + c[0][1]*c[2][3]*c[3][2] + c[0][2]*c[2][1]*c[3][3] - c[0][1]*c[2][2]*c[3][3]);
		m.c[0][2] = d * (c[0][2]*c[1][3]*c[3][1] - c[0][3]*c[1][2]*c[3][1] + c[0][3]*c[1][1]*c[3][2] - c[0][1]*c[1][3]*c[3][2] - c[0][2]*c[1][1]*c[3][3] + c[0][1]*c[1][2]*c[3][3]);
		m.c[0][3] = d * (c[0][3]*c[1][2]*c[2][1] - c[0][2]*c[1][3]*c[2][1] - c[0][3]*c[1][1]*c[2][2] + c[0][1]*c[1][3]*c[2][2] + c[0][2]*c[1][1]*c[2][3] - c[0][1]*c[1][2]*c[2][3]);
		m.c[1][0] = d * (c[1][3]*c[2][2]*c[3][0] - c[1][2]*c[2][3]*c[3][0] - c[1][3]*c[2][0]*c[3][2] + c[1][0]*c[2][3]*c[3][2] + c[1][2]*c[2][0]*c[3][3] - c[1][0]*c[2][2]*c[3][3]);
		m.c[1][1] = d * (c[0][2]*c[2][3]*c[3][0] - c[0][3]*c[2][2]*c[3][0] + c[0][3]*c[2][0]*c[3][2] - c[0][0]*c[2][3]*c[3][2] - c[0][2]*c[2][0]*c[3][3] + c[0][0]*c[2][2]*c[3][3]);
		m.c[1][2] = d * (c[0][3]*c[1][2]*c[3][0] - c[0][2]*c[1][3]*c[3][0] - c[0][3]*c[1][0]*c[3][2] + c[0][0]*c[1][3]*c[3][2] + c[0][2]*c[1][0]*c[3][3] - c[0][0]*c[1][2]*c[3][3]);
		m.c[1][3] = d * (c[0][2]*c[1][3]*c[2][0] - c[0][3]*c[1][2]*c[2][0] + c[0][3]*c[1][0]*c[2][2] - c[0][0]*c[1][3]*c[2][2] - c[0][2]*c[1][0]*c[2][3] + c[0][0]*c[1][2]*c[2][3]);
		m.c[2][0] = d * (c[1][1]*c[2][3]*c[3][0] - c[1][3]*c[2][1]*c[3][0] + c[1][3]*c[2][0]*c[3][1] - c[1][0]*c[2][3]*c[3][1] - c[1][1]*c[2][0]*c[3][3] + c[1][0]*c[2][1]*c[3][3]);
		m.c[2][1] = d * (c[0][3]*c[2][1]*c[3][0] - c[0][1]*c[2][3]*c[3][0] - c[0][3]*c[2][0]*c[3][1] + c[0][0]*c[2][3]*c[3][1] + c[0][1]*c[2][0]*c[3][3] - c[0][0]*c[2][1]*c[3][3]);
		m.c[2][2] = d * (c[0][1]*c[1][3]*c[3][0] - c[0][3]*c[1][1]*c[3][0] + c[0][3]*c[1][0]*c[3][1] - c[0][0]*c[1][3]*c[3][1] - c[0][1]*c[1][0]*c[3][3] + c[0][0]*c[1][1]*c[3][3]);
		m.c[2][3] = d * (c[0][3]*c[1][1]*c[2][0] - c[0][1]*c[1][3]*c[2][0] - c[0][3]*c[1][0]*c[2][1] + c[0][0]*c[1][3]*c[2][1] + c[0][1]*c[1][0]*c[2][3] - c[0][0]*c[1][1]*c[2][3]);
		m.c[3][0] = d * (c[1][2]*c[2][1]*c[3][0] - c[1][1]*c[2][2]*c[3][0] - c[1][2]*c[2][0]*c[3][1] + c[1][0]*c[2][2]*c[3][1] + c[1][1]*c[2][0]*c[3][2] - c[1][0]*c[2][1]*c[3][2]);
		m.c[3][1] = d * (c[0][1]*c[2][2]*c[3][0] - c[0][2]*c[2][1]*c[3][0] + c[0][2]*c[2][0]*c[3][1] - c[0][0]*c[2][2]*c[3][1] - c[0][1]*c[2][0]*c[3][2] + c[0][0]*c[2][1]*c[3][2]);
		m.c[3][2] = d * (c[0][2]*c[1][1]*c[3][0] - c[0][1]*c[1][2]*c[3][0] - c[0][2]*c[1][0]*c[3][1] + c[0][0]*c[1][2]*c[3][1] + c[0][1]*c[1][0]*c[3][2] - c[0][0]*c[1][1]*c[3][2]);
		m.c[3][3] = d * (c[0][1]*c[1][2]*c[2][0] - c[0][2]*c[1][1]*c[2][0] + c[0][2]*c[1][0]*c[2][1] - c[0][0]*c[1][2]*c[2][1] - c[0][1]*c[1][0]*c[2][2] + c[0][0]*c[1][1]*c[2][2]);

		return m;
	}

	void decompose( Vec3d &trans, Vec3d &rot, Vec3d &scale ) const
	{
		// Getting translation is trivial
		trans = Vec3d( c[3][0], c[3][1], c[3][2] );

		// Scale is length of columns
		scale.x = sqrt( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrt( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrt( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );

		if( scale.x == 0 || scale.y == 0 || scale.z == 0 ) return;

		// Detect negative scale with determinant and flip one arbitrary axis
		if( determinant() < 0 ) scale.x = -scale.x;

		// Combined rotation matrix YXZ
		//
		// Cos[y]*Cos[z]+Sin[x]*Sin[y]*Sin[z]   Cos[z]*Sin[x]*Sin[y]-Cos[y]*Sin[z]  Cos[x]*Sin[y]
		// Cos[x]*Sin[z]                        Cos[x]*Cos[z]                       -Sin[x]
		// -Cos[z]*Sin[y]+Cos[y]*Sin[x]*Sin[z]  Cos[y]*Cos[z]*Sin[x]+Sin[y]*Sin[z]  Cos[x]*Cos[y]

		rot.x = asin( -c[2][1] / scale.z );

		// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
		double f = fabs( c[2][1] / scale.z );
		if( f > 0.999 && f < 1.001 )
		{
			// Pin arbitrarily one of y or z to zero
			// Mathematical equivalent of gimbal lock
			rot.y = 0;

			// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
			// => m[0][0] = Cos[z] and m[1][0] = Sin[z]
			rot.z = atan2( -c[1][0] / scale.y, c[0][0] / scale.x );
		}
			// Standard case
		else
		{
			rot.y = atan2( c[2][0] / scale.z, c[2][2] / scale.z );
			rot.z = atan2( c[0][1] / scale.x, c[1][1] / scale.y );
		}
	}


	void setCol( unsigned int col, const Vec4d& v )
	{
		x[col * 4 + 0] = v.x;
		x[col * 4 + 1] = v.y;
		x[col * 4 + 2] = v.z;
		x[col * 4 + 3] = v.w;
	}

	Vec4d getCol( unsigned int col ) const
	{
		return Vec4d( x[col * 4 + 0], x[col * 4 + 1], x[col * 4 + 2], x[col * 4 + 3] );
	}

	Vec4d getRow( unsigned int row ) const
	{
		return Vec4d( x[row + 0], x[row + 4], x[row + 8], x[row + 12] );
	}

	Vec3d getTrans() const
	{
		return Vec3d( c[3][0], c[3][1], c[3][2] );
	}

	Vec3d getScale() const
	{
		Vec3d scale;
		// Scale is length of columns
		scale.x = sqrt( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrt( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrt( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );
		return scale;
	}
};


// -------------------------------------------------------------------------------------------------
// Plane
// -------------------------------------------------------------------------------------------------

class Plane
{
public:
	Vec3f normal; 
	float dist;

	// ------------
	// Constructors
	// ------------
	Plane() 
	{ 
		normal.x = 0; normal.y = 0; normal.z = 0; dist = 0; 
	};

	explicit Plane( const float a, const float b, const float c, const float d )
	{
		normal = Vec3f( a, b, c );
		float invLen = 1.0f / normal.length();
		normal *= invLen;	// Normalize
		dist = d * invLen;
	}

	Plane( const Vec3f &v0, const Vec3f &v1, const Vec3f &v2 )
	{
		normal = v1 - v0;
		normal = normal.cross( v2 - v0 );
		normal.normalize();
		dist = -normal.dot( v0 );
	}

	// ----------------
	// Other operations
	// ----------------
	float distToPoint( const Vec3f &v ) const
	{
		return normal.dot( v ) + dist;
	}
};


class Planed
{
public:
	Vec3d normal;
	double dist;

	// ------------
	// Constructors
	// ------------
	Planed()
	{
		normal.x = 0; normal.y = 0; normal.z = 0; dist = 0;
	};

	explicit Planed( const double a, const double b, const double c, const double d )
	{
		normal = Vec3d( a, b, c );
		double invLen = 1.0 / normal.length();
		normal *= invLen;	// Normalize
		dist = d * invLen;
	}

	Planed( const Vec3d &v0, const Vec3d &v1, const Vec3d &v2 )
	{
		normal = v1 - v0;
		normal = normal.cross( v2 - v0 );
		normal.normalize();
		dist = -normal.dot( v0 );
	}

	// ----------------
	// Other operations
	// ----------------
	double distToPoint( const Vec3d &v ) const
	{
		return normal.dot( v ) + dist;
	}
};


// -------------------------------------------------------------------------------------------------
// Intersection
// -------------------------------------------------------------------------------------------------

inline bool rayTriangleIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, 
									 const Vec3f &vert0, const Vec3f &vert1, const Vec3f &vert2,
									 Vec3f &intsPoint )
{
	// Idea: Tomas Moeller and Ben Trumbore
	// in Fast, Minimum Storage Ray/Triangle Intersection 
	
	// Find vectors for two edges sharing vert0
	Vec3f edge1 = vert1 - vert0;
	Vec3f edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	Vec3f pvec = rayDir.cross( edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	float det = edge1.dot( pvec );


	// *** Culling branch ***
	/*if( det < Math::Epsilon )return false;

	// Calculate distance from vert0 to ray origin
	Vec3f tvec = rayOrig - vert0;

	// Calculate U parameter and test bounds
	float u = tvec.dot( pvec );
	if (u < 0 || u > det ) return false;

	// Prepare to test V parameter
	Vec3f qvec = tvec.cross( edge1 );

	// Calculate V parameter and test bounds
	float v = rayDir.dot( qvec );
	if (v < 0 || u + v > det ) return false;

	// Calculate t, scale parameters, ray intersects triangle
	float t = edge2.dot( qvec ) / det;*/


	// *** Non-culling branch ***
	if( det > -Math::Epsilon && det < Math::Epsilon ) return 0;
	float inv_det = 1.0f / det;

	// Calculate distance from vert0 to ray origin
	Vec3f tvec = rayOrig - vert0;

	// Calculate U parameter and test bounds
	float u = tvec.dot( pvec ) * inv_det;
	if( u < 0.0f || u > 1.0f ) return 0;

	// Prepare to test V parameter
	Vec3f qvec = tvec.cross( edge1 );

	// Calculate V parameter and test bounds
	float v = rayDir.dot( qvec ) * inv_det;
	if( v < 0.0f || u + v > 1.0f ) return 0;

	// Calculate t, ray intersects triangle
	float t = edge2.dot( qvec ) * inv_det;


	// Calculate intersection point and test ray length and direction
	intsPoint = rayOrig + rayDir * t;
	Vec3f vec = intsPoint - rayOrig;
	if( vec.dot( rayDir ) < 0 || vec.length() > rayDir.length() ) return false;

	return true;
}


inline bool rayTriangleIntersection( const Vec3d &rayOrig, const Vec3d &rayDir,
									 const Vec3d &vert0, const Vec3d &vert1, const Vec3d &vert2,
									 Vec3d &intsPoint )
{
	// Idea: Tomas Moeller and Ben Trumbore
	// in Fast, Minimum Storage Ray/Triangle Intersection

	// Find vectors for two edges sharing vert0
	Vec3d edge1 = vert1 - vert0;
	Vec3d edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	Vec3d pvec = rayDir.cross( edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	double det = edge1.dot( pvec );


	// *** Culling branch ***
	/*if( det < Math::Epsilon )return false;

	// Calculate distance from vert0 to ray origin
	Vec3d tvec = rayOrig - vert0;

	// Calculate U parameter and test bounds
	double u = tvec.dot( pvec );
	if (u < 0 || u > det ) return false;

	// Prepare to test V parameter
	Vec3d qvec = tvec.cross( edge1 );

	// Calculate V parameter and test bounds
	double v = rayDir.dot( qvec );
	if (v < 0 || u + v > det ) return false;

	// Calculate t, scale parameters, ray intersects triangle
	double t = edge2.dot( qvec ) / det;*/


	// *** Non-culling branch ***
	if( det > -Math::Epsilon && det < Math::Epsilon ) return 0;
	double inv_det = 1.0 / det;

	// Calculate distance from vert0 to ray origin
	Vec3d tvec = rayOrig - vert0;

	// Calculate U parameter and test bounds
	double u = tvec.dot( pvec ) * inv_det;
	if( u < 0.0 || u > 1.0 ) return 0;

	// Prepare to test V parameter
	Vec3d qvec = tvec.cross( edge1 );

	// Calculate V parameter and test bounds
	double v = rayDir.dot( qvec ) * inv_det;
	if( v < 0.0 || u + v > 1.0 ) return 0;

	// Calculate t, ray intersects triangle
	double t = edge2.dot( qvec ) * inv_det;


	// Calculate intersection point and test ray length and direction
	intsPoint = rayOrig + rayDir * t;
	Vec3d vec = intsPoint - rayOrig;
	if( vec.dot( rayDir ) < 0 || vec.length() > rayDir.length() ) return false;

	return true;
}


inline bool rayAABBIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, 
								 const Vec3f &mins, const Vec3f &maxs )
{
	// SLAB based optimized ray/AABB intersection routine
	// Idea taken from http://ompf.org/ray/
	
	float l1 = (mins.x - rayOrig.x) / rayDir.x;
	float l2 = (maxs.x - rayOrig.x) / rayDir.x;
	float lmin = minf( l1, l2 );
	float lmax = maxf( l1, l2 );

	l1 = (mins.y - rayOrig.y) / rayDir.y;
	l2 = (maxs.y - rayOrig.y) / rayDir.y;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );
		
	l1 = (mins.z - rayOrig.z) / rayDir.z;
	l2 = (maxs.z - rayOrig.z) / rayDir.z;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );

	if( (lmax >= 0.0f) & (lmax >= lmin) )
	{
		// Consider length
		const Vec3f rayDest = rayOrig + rayDir;
		Vec3f rayMins( minf( rayDest.x, rayOrig.x), minf( rayDest.y, rayOrig.y ), minf( rayDest.z, rayOrig.z ) );
		Vec3f rayMaxs( maxf( rayDest.x, rayOrig.x), maxf( rayDest.y, rayOrig.y ), maxf( rayDest.z, rayOrig.z ) );
		return 
			(rayMins.x < maxs.x) && (rayMaxs.x > mins.x) &&
			(rayMins.y < maxs.y) && (rayMaxs.y > mins.y) &&
			(rayMins.z < maxs.z) && (rayMaxs.z > mins.z);
	}
	else
		return false;
}


inline bool rayAABBIntersection( const Vec3d &rayOrig, const Vec3d &rayDir,
								 const Vec3d &mins, const Vec3d &maxs )
{
	// SLAB based optimized ray/AABB intersection routine
	// Idea taken from http://ompf.org/ray/

	double l1 = (mins.x - rayOrig.x) / rayDir.x;
	double l2 = (maxs.x - rayOrig.x) / rayDir.x;
	double lmin = minf( l1, l2 );
	double lmax = maxf( l1, l2 );

	l1 = (mins.y - rayOrig.y) / rayDir.y;
	l2 = (maxs.y - rayOrig.y) / rayDir.y;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );

	l1 = (mins.z - rayOrig.z) / rayDir.z;
	l2 = (maxs.z - rayOrig.z) / rayDir.z;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );

	if( (lmax >= 0.0) & (lmax >= lmin) )
	{
		// Consider length
		const Vec3d rayDest = rayOrig + rayDir;
		Vec3d rayMins( minf( rayDest.x, rayOrig.x), minf( rayDest.y, rayOrig.y ), minf( rayDest.z, rayOrig.z ) );
		Vec3d rayMaxs( maxf( rayDest.x, rayOrig.x), maxf( rayDest.y, rayOrig.y ), maxf( rayDest.z, rayOrig.z ) );
		return
				(rayMins.x < maxs.x) && (rayMaxs.x > mins.x) &&
				(rayMins.y < maxs.y) && (rayMaxs.y > mins.y) &&
				(rayMins.z < maxs.z) && (rayMaxs.z > mins.z);
	}
	else
		return false;
}


inline float nearestDistToAABB( const Vec3f &pos, const Vec3f &mins, const Vec3f &maxs )
{
	const Vec3f center = (mins + maxs) * 0.5f;
	const Vec3f extent = (maxs - mins) * 0.5f;
	
	Vec3f nearestVec;
	nearestVec.x = maxf( 0, fabsf( pos.x - center.x ) - extent.x );
	nearestVec.y = maxf( 0, fabsf( pos.y - center.y ) - extent.y );
	nearestVec.z = maxf( 0, fabsf( pos.z - center.z ) - extent.z );
	
	return nearestVec.length();
}


inline double nearestDistToAABB( const Vec3d &pos, const Vec3d &mins, const Vec3d &maxs )
{
	const Vec3d center = (mins + maxs) * 0.5;
	const Vec3d extent = (maxs - mins) * 0.5;

	Vec3d nearestVec;
	nearestVec.x = maxf( 0, fabs( pos.x - center.x ) - extent.x );
	nearestVec.y = maxf( 0, fabs( pos.y - center.y ) - extent.y );
	nearestVec.z = maxf( 0, fabs( pos.z - center.z ) - extent.z );

	return nearestVec.length();
}

}
#endif // _utMath_H_
