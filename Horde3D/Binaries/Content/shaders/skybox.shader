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

OpenGL4
{
	context ATTRIBPASS
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		PixelShader = compile GLSL FS_ATTRIBPASS_GL4;
	}

	context AMBIENT
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		PixelShader = compile GLSL FS_AMBIENT_GL4;
	}
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
	

[[VS_GENERAL_GL4]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform mat4 viewProjMat;
uniform vec3 viewerPos;

layout( location = 0 ) in vec3 vertPos;
out vec3 viewVec;

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

[[FS_ATTRIBPASS_GL4]]
// =================================================================================================

#include "shaders/utilityLib/fragDeferredWriteGL4.glsl"

uniform samplerCube albedoMap;
in vec3 viewVec;

void main( void )
{
	vec3 albedo = texture( albedoMap, viewVec ).rgb;
	
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

[[FS_AMBIENT_GL4]]
// =================================================================================================

uniform samplerCube albedoMap;
in vec3 viewVec;

out	vec4 fragColor;

void main( void )
{
	vec3 albedo = texture( albedoMap, viewVec ).rgb;
	
	fragColor.rgb = albedo;
}