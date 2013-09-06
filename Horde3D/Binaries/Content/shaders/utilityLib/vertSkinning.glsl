// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Skinning functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

uniform 	vec4 skinMatRows[75*3];
attribute 	vec4 joints, weights;


mat4 getJointMat( const int jointIndex )
{
	// Note: This matrix is transposed so vec/mat multiplications need to be done in reversed order
	return mat4( skinMatRows[jointIndex * 3],
				 skinMatRows[jointIndex * 3 + 1],
				 skinMatRows[jointIndex * 3 + 2],
				 vec4( 0, 0, 0, 1 ) );
}

mat4 calcSkinningMat()
{
	return weights.x * getJointMat( int( joints.x ) ) +
		   weights.y * getJointMat( int( joints.y ) ) +
		   weights.z * getJointMat( int( joints.z ) ) +
		   weights.w * getJointMat( int( joints.w ) );
}

mat3 getSkinningMatVec( const mat4 skinningMat )
{
	return mat3( skinningMat[0].xyz, skinningMat[1].xyz, skinningMat[2].xyz );
}

vec4 skinPos( const vec4 pos )
{
	return pos * getJointMat( int( joints.x ) ) * weights.x +
		   pos * getJointMat( int( joints.y ) ) * weights.y +
		   pos * getJointMat( int( joints.z ) ) * weights.z +
		   pos * getJointMat( int( joints.w ) ) * weights.w;
}

vec4 skinPos( const vec4 pos, const mat4 skinningMat )
{
	return pos * skinningMat;
}

vec3 skinVec( const vec3 vec, const mat3 skinningMatTSB )
{
	return vec * skinningMatTSB;
}
