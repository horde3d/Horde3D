// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Particle functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

uniform mat4 viewMatInv;
uniform vec3 parPosArray[64];
uniform vec2 parSizeAndRotArray[64];
uniform vec4 parColorArray[64];

attribute float parIdx;


vec4 getParticleColor()
{
	return parColorArray[int( parIdx )];
}

vec3 calcParticlePos( const vec2 texCoords )
{
	int index = int( parIdx );
	vec3 camAxisX = viewMatInv[0].xyz;
	vec3 camAxisY = viewMatInv[1].xyz;
	
	vec2 cornerPos = texCoords - vec2( 0.5, 0.5 );
	
	// Apply rotation
	float s = sin( parSizeAndRotArray[index].y );
	float c = cos( parSizeAndRotArray[index].y );
	cornerPos = mat2( c, -s, s, c ) * cornerPos;
	
	return parPosArray[index] + (camAxisX * cornerPos.x + camAxisY * cornerPos.y) * parSizeAndRotArray[index].x;
}