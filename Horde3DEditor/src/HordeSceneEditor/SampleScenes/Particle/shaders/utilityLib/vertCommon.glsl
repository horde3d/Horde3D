// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Common functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

uniform mat4 viewMat;
uniform mat4 worldMat;
uniform	mat3 worldNormalMat;


vec4 calcWorldPos( const vec4 pos )
{
	return worldMat * pos;
}

vec4 calcViewPos( const vec4 pos )
{
	return viewMat * pos;
}

vec3 calcWorldVec( const vec3 vec )
{
	return worldNormalMat * vec;
}

mat3 calcTanToWorldMat( const vec3 tangent, const vec3 bitangent, const vec3 normal )
{
	return mat3( tangent, bitangent, normal );
}

vec3 calcTanVec( const vec3 vec, const vec3 tangent, const vec3 bitangent, const vec3 normal )
{
	vec3 v;
	v.x = dot( vec, tangent );
	v.y = dot( vec, bitangent );
	v.z = dot( vec, normal );
	return v;
}