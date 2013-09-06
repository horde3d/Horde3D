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

#include "egPrimitives.h"

#include "utDebug.h"


namespace Horde3D {

// *************************************************************************************************
// Frustum
// *************************************************************************************************

void Frustum::buildViewFrustum( const Matrix4f &transMat, float fov, float aspect, float nearPlane, float farPlane )
{
	float ymax = nearPlane * tanf( degToRad( fov / 2 ) );
	float xmax = ymax * aspect;
	
	buildViewFrustum( transMat, -xmax, xmax, -ymax, ymax, nearPlane, farPlane );
}


void Frustum::buildViewFrustum( const Matrix4f &transMat, float left, float right,
							    float bottom, float top, float nearPlane, float farPlane ) 
{
	// Use intercept theorem to get params for far plane
	float left_f = left * farPlane / nearPlane;
	float right_f = right * farPlane / nearPlane;
	float bottom_f = bottom * farPlane / nearPlane;
	float top_f = top * farPlane / nearPlane;

	// Get points on near plane
	_corners[0] = Vec3f( left, bottom, -nearPlane );
	_corners[1] = Vec3f( right, bottom, -nearPlane );
	_corners[2] = Vec3f( right, top, -nearPlane );
	_corners[3] = Vec3f( left, top, -nearPlane );

	// Get points on far plane
	_corners[4] = Vec3f( left_f, bottom_f, -farPlane );
	_corners[5] = Vec3f( right_f, bottom_f, -farPlane );
	_corners[6] = Vec3f( right_f, top_f, -farPlane );
	_corners[7] = Vec3f( left_f, top_f, -farPlane );

	// Transform points to fit camera position and rotation
	_origin = transMat * Vec3f( 0, 0, 0 );
	for( uint32 i = 0; i < 8; ++i )
		_corners[i] = transMat * _corners[i];

	// Build planes
	_planes[0] = Plane( _origin, _corners[3], _corners[0] );		// Left
	_planes[1] = Plane( _origin, _corners[1], _corners[2] );		// Right
	_planes[2] = Plane( _origin, _corners[0], _corners[1] );		// Bottom
	_planes[3] = Plane( _origin, _corners[2], _corners[3] );		// Top
	_planes[4] = Plane( _corners[0], _corners[1], _corners[2] );	// Near
	_planes[5] = Plane( _corners[5], _corners[4], _corners[7] );	// Far
}


void Frustum::buildViewFrustum( const Matrix4f &viewMat, const Matrix4f &projMat )
{
	// This routine works with the OpenGL projection matrix
	// The view matrix is the inverse camera transformation matrix
	// Note: Frustum corners are not updated!
	
	Matrix4f m = projMat * viewMat;
	
	_planes[0] = Plane( -(m.c[0][3] + m.c[0][0]), -(m.c[1][3] + m.c[1][0]),
						-(m.c[2][3] + m.c[2][0]), -(m.c[3][3] + m.c[3][0]) );	// Left
	_planes[1] = Plane( -(m.c[0][3] - m.c[0][0]), -(m.c[1][3] - m.c[1][0]),
						-(m.c[2][3] - m.c[2][0]), -(m.c[3][3] - m.c[3][0]) );	// Right
	_planes[2] = Plane( -(m.c[0][3] + m.c[0][1]), -(m.c[1][3] + m.c[1][1]),
						-(m.c[2][3] + m.c[2][1]), -(m.c[3][3] + m.c[3][1]) );	// Bottom
	_planes[3] = Plane( -(m.c[0][3] - m.c[0][1]), -(m.c[1][3] - m.c[1][1]),
						-(m.c[2][3] - m.c[2][1]), -(m.c[3][3] - m.c[3][1]) );	// Top
	_planes[4] = Plane( -(m.c[0][3] + m.c[0][2]), -(m.c[1][3] + m.c[1][2]),
						-(m.c[2][3] + m.c[2][2]), -(m.c[3][3] + m.c[3][2]) );	// Near
	_planes[5] = Plane( -(m.c[0][3] - m.c[0][2]), -(m.c[1][3] - m.c[1][2]),
						-(m.c[2][3] - m.c[2][2]), -(m.c[3][3] - m.c[3][2]) );	// Far

	_origin = viewMat.inverted() * Vec3f( 0, 0, 0 );

	// Calculate corners
	Matrix4f mm = m.inverted();
	Vec4f corner = mm * Vec4f( -1, -1,  -1, 1 );
	_corners[0] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
	corner = mm * Vec4f( 1, -1,  -1, 1 );
	_corners[1] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
	corner = mm * Vec4f( 1,  1,  -1, 1 );
	_corners[2] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
	corner = mm * Vec4f( -1,  1,  -1, 1 );
	_corners[3] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
	corner = mm * Vec4f( -1, -1, 1, 1 );
	_corners[4] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
	corner = mm * Vec4f( 1, -1, 1, 1 );
	_corners[5] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
	corner = mm * Vec4f( 1, 1, 1, 1 );
	_corners[6] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
	corner = mm * Vec4f( -1, 1, 1, 1 );
	_corners[7] = Vec3f( corner.x / corner.w, corner.y / corner.w, corner.z / corner.w );
}


void Frustum::buildBoxFrustum( const Matrix4f &transMat, float left, float right,
							   float bottom, float top, float front, float back ) 
{
	// Get points on front plane
	_corners[0] = Vec3f( left, bottom, front );
	_corners[1] = Vec3f( right, bottom, front );
	_corners[2] = Vec3f( right, top, front );
	_corners[3] = Vec3f( left, top, front );

	// Get points on far plane
	_corners[4] = Vec3f( left, bottom, back );
	_corners[5] = Vec3f( right, bottom, back );
	_corners[6] = Vec3f( right, top, back );
	_corners[7] = Vec3f( left, top, back );

	// Transform points to fit camera position and rotation
	_origin = transMat * Vec3f( 0, 0, 0 );
	for( uint32 i = 0; i < 8; ++i )
		_corners[i] = transMat * _corners[i];

	// Build planes
	_planes[0] = Plane( _corners[0], _corners[3], _corners[7] );	// Left
	_planes[1] = Plane( _corners[2], _corners[1], _corners[6] );	// Right
	_planes[2] = Plane( _corners[0], _corners[4], _corners[5] );	// Bottom
	_planes[3] = Plane( _corners[3], _corners[2], _corners[6] );	// Top
	_planes[4] = Plane( _corners[0], _corners[1], _corners[2] );	// Front
	_planes[5] = Plane( _corners[4], _corners[7], _corners[6] );	// Back
}


bool Frustum::cullSphere( Vec3f pos, float rad ) const
{
	// Check the distance of the center to the planes
	for( uint32 i = 0; i < 6; ++i )
	{
		if( _planes[i].distToPoint( pos ) > rad ) return true;
	}

	return false;
}


bool Frustum::cullBox( BoundingBox &b ) const
{
	// Idea for optimized AABB testing from www.lighthouse3d.com
	for( uint32 i = 0; i < 6; ++i )
	{
		const Vec3f &n = _planes[i].normal;
		
		Vec3f positive = b.min;
		if( n.x <= 0 ) positive.x = b.max.x;
		if( n.y <= 0 ) positive.y = b.max.y;
		if( n.z <= 0 ) positive.z = b.max.z;

		if( _planes[i].distToPoint( positive ) > 0 ) return true;
	}
	
	return false;
}


bool Frustum::cullFrustum( const Frustum &frust ) const
{
	for( uint32 i = 0; i < 6; ++i )
	{
		bool allOut = true;
		
		for( uint32 j = 0; j < 8; ++j )
		{
			if( _planes[i].distToPoint( frust._corners[j] ) < 0 )
			{
				allOut = false;
				break;
			}
		}

		if( allOut ) return true;
	}

	return false;
}


void Frustum::calcAABB( Vec3f &mins, Vec3f &maxs ) const
{
	mins.x = Math::MaxFloat; mins.y = Math::MaxFloat; mins.z = Math::MaxFloat;
	maxs.x = -Math::MaxFloat; maxs.y = -Math::MaxFloat; maxs.z = -Math::MaxFloat;
	
	for( uint32 i = 0; i < 8; ++i )
	{
		if( _corners[i].x < mins.x ) mins.x = _corners[i].x;
		if( _corners[i].y < mins.y ) mins.y = _corners[i].y;
		if( _corners[i].z < mins.z ) mins.z = _corners[i].z;
		if( _corners[i].x > maxs.x ) maxs.x = _corners[i].x;
		if( _corners[i].y > maxs.y ) maxs.y = _corners[i].y;
		if( _corners[i].z > maxs.z ) maxs.z = _corners[i].z;
	}
}

}  // namespace
