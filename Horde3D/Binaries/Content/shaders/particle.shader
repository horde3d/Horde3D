[[FX]]

// Samplers
sampler2D albedoMap = sampler_state
{
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Contexts
/*context SHADOWMAP
{
	VertexShader = compile GLSL VS_SHADOWMAP;
	PixelShader = compile GLSL FS_SHADOWMAP;
}*/

context TRANSLUCENT
{
	VertexShader = compile GLSL VS_TRANSLUCENT;
	PixelShader = compile GLSL FS_TRANSLUCENT;
	
	ZWriteEnable = false;
	BlendMode = AddBlended;
}


[[VS_SHADOWMAP]]
// =================================================================================================

#include "shaders/utilityLib/vertParticle.glsl"

uniform mat4 viewProjMat;
uniform vec4 lightPos;
attribute vec2 texCoords0;
varying float dist;

void main(void)
{
	vec4 pos = vec4( calcParticlePos( texCoords0 ), 1 );
	dist = length( lightPos.xyz - pos.xyz ) / lightPos.w;
	
	gl_Position = viewProjMat * pos;
}
				
				
[[FS_SHADOWMAP]]
// =================================================================================================

uniform float shadowBias;
varying float dist;

void main( void )
{
	gl_FragDepth = dist + shadowBias;
}


[[VS_TRANSLUCENT]]
// =================================================================================================

#include "shaders/utilityLib/vertParticle.glsl"

uniform mat4 viewProjMat;
attribute vec2 texCoords0;
varying vec4 color;
varying vec2 texCoords;

void main(void)
{
	color = getParticleColor();
	texCoords = vec2( texCoords0.s, -texCoords0.t );
	gl_Position = viewProjMat * vec4( calcParticlePos( texCoords0 ), 1 );
}


[[FS_TRANSLUCENT]]
// =================================================================================================

uniform sampler2D albedoMap;
varying vec4 color;
varying vec2 texCoords;

void main( void )
{
	vec4 albedo = texture2D( albedoMap, texCoords );
	
	gl_FragColor = albedo * color;
}
