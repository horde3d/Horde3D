// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Deferred shading functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

uniform sampler2D buf0;
uniform sampler2D buf1;
uniform sampler2D buf2;

float getMatID( const vec2 coord ) { return texture2D( buf0, coord ).a; }
vec3 getPos( const vec2 coord ) { return texture2D( buf0, coord ).rgb; }
vec3 getNormal( const vec2 coord ) { return texture2D( buf1, coord ).rgb; }
vec3 getAlbedo( const vec2 coord ) { return texture2D( buf2, coord ).rgb; }
float getSpecMask( const vec2 coord ) { return texture2D( buf2, coord ).a; }