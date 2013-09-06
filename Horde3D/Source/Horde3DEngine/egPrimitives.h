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

#ifndef _egPrimitives_H_
#define _egPrimitives_H_

#include "egPrerequisites.h"
#include "utMath.h"


namespace Horde3D {

// =================================================================================================
// Bounding Box
// =================================================================================================

struct BoundingBox
{	
	Vec3f  min, max;

	
	void clear()
	{
		min = Vec3f( 0, 0, 0 );
		max = Vec3f( 0, 0, 0 );
	}

	Vec3f getCorner( uint32 index ) const
	{
		switch( index )
		{
		case 0:
			return Vec3f( min.x, min.y, max.z );
		case 1:
			return Vec3f( max.x, min.y, max.z );
		case 2:
			return Vec3f( max.x, max.y, max.z );
		case 3:
			return Vec3f( min.x, max.y, max.z );
		case 4:
			return Vec3f( min.x, min.y, min.z );
		case 5:
			return Vec3f( max.x, min.y, min.z );
		case 6:
			return Vec3f( max.x, max.y, min.z );
		case 7:
			return Vec3f( min.x, max.y, min.z );
		default:
			return Vec3f();
		}
	}


	void transform( const Matrix4f &m )
	{
		// Efficient algorithm for transforming an AABB, taken from Graphics Gems
		
		float minA[3] = { min.x, min.y, min.z }, minB[3];
		float maxA[3] = { max.x, max.y, max.z }, maxB[3];

		for( uint32 i = 0; i < 3; ++i )
		{
			minB[i] = m.c[3][i];
			maxB[i] = m.c[3][i];
			
			for( uint32 j = 0; j < 3; ++j )
			{
				float x = minA[j] * m.c[j][i];
				float y = maxA[j] * m.c[j][i];
				minB[i] += minf( x, y );
				maxB[i] += maxf( x, y );
			}
		}

		min = Vec3f( minB[0], minB[1], minB[2] );
		max = Vec3f( maxB[0], maxB[1], maxB[2] );
	}


	bool makeUnion( BoundingBox &b )
	{
		bool changed = false;

		// Ignore zero-size boxes
		if( min == max )
		{
			changed = true;
			min = b.min;
			max = b.max;
		}
		else if( b.min != b.max )
		{
			if( b.min.x < min.x ) { changed = true; min.x = b.min.x; }
			if( b.min.y < min.y ) { changed = true; min.y = b.min.y; }
			if( b.min.z < min.z ) { changed = true; min.z = b.min.z; }

			if( b.max.x > max.x ) { changed = true; max.x = b.max.x; }
			if( b.max.y > max.y ) { changed = true; max.y = b.max.y; }
			if( b.max.z > max.z ) { changed = true; max.z = b.max.z; }
		}

		return changed;
	}
};


// =================================================================================================
// Frustum
// =================================================================================================

class Frustum
{
public:
	const Vec3f &getOrigin() const { return _origin; }
	const Vec3f &getCorner( uint32 index ) const { return _corners[index]; }
	
	void buildViewFrustum( const Matrix4f &transMat, float fov, float aspect, float nearPlane, float farPlane );
	void buildViewFrustum( const Matrix4f &transMat, float left, float right,
	                       float bottom, float top, float nearPlane, float farPlane );
	void buildViewFrustum( const Matrix4f &viewMat, const Matrix4f &projMat );
	void buildBoxFrustum( const Matrix4f &transMat, float left, float right,
	                      float bottom, float top, float front, float back );
	bool cullSphere( Vec3f pos, float rad ) const;
	bool cullBox( BoundingBox &b ) const;
	bool cullFrustum( const Frustum &frust ) const;

	void calcAABB( Vec3f &mins, Vec3f &maxs ) const;

private:
	Plane  _planes[6];  // Planes of frustum
	Vec3f  _origin;
	Vec3f  _corners[8];  // Corner points
};

}
#endif // _egPrimitives_H_
