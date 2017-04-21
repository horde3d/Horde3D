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

uniform sampler2D gbuf0;
uniform sampler2D gbuf1;
uniform sampler2D gbuf2;
uniform sampler2D gbuf3;

float getMatID( const vec2 coord ) { return texture2D( gbuf0, coord ).a; }
vec3 getPos( const vec2 coord ) { return texture2D( gbuf0, coord ).rgb; }
vec3 getNormal( const vec2 coord ) { return texture2D( gbuf1, coord ).rgb; }
vec3 getAlbedo( const vec2 coord ) { return texture2D( gbuf2, coord ).rgb; }
vec4 getSpecParams( const vec2 coord ) { return texture2D( gbuf3, coord ).rgba; }
