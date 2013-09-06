[[FX]]

// Samplers
samplerCube albedoMap = sampler_state
{
	Address = Clamp;
};

// Contexts
context ATTRIBPASS
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_ATTRIBPASS;
}

context AMBIENT
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_AMBIENT;
}


[[VS_GENERAL]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform mat4 viewProjMat;
uniform vec3 viewerPos;
attribute vec3 vertPos;
varying vec3 viewVec;

void main(void)
{
	vec4 pos = calcWorldPos( vec4( vertPos, 1.0 ) );
	viewVec = pos.xyz - viewerPos;
	
	gl_Position = viewProjMat * pos;
}
				

[[FS_ATTRIBPASS]]
// =================================================================================================

#include "shaders/utilityLib/fragDeferredWrite.glsl"

uniform samplerCube albedoMap;
varying vec3 viewVec;

void main( void )
{
	vec3 albedo = textureCube( albedoMap, viewVec ).rgb;
	
	// Set fragment material ID to 2, meaning skybox in this case
	setMatID( 2.0 );
	setAlbedo( albedo );
}


[[FS_AMBIENT]]
// =================================================================================================

uniform samplerCube albedoMap;
varying vec3 viewVec;

void main( void )
{
	vec3 albedo = textureCube( albedoMap, viewVec ).rgb;
	
	gl_FragColor.rgb = albedo;
}
