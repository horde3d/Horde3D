[[FX]]

// Samplers
sampler2D albedoMap;

// Contexts
context OVERLAY
{
	VertexShader = compile GLSL VS_OVERLAY;
	PixelShader = compile GLSL FS_OVERLAY;
	
	ZWriteEnable = false;
	BlendMode = Blend;
}

OpenGL4
{
	context OVERLAY
	{
		VertexShader = compile GLSL VS_OVERLAY_GL4;
		PixelShader = compile GLSL FS_OVERLAY_GL4;
		
		ZWriteEnable = false;
		BlendMode = Blend;
	}
}

[[VS_OVERLAY]]

uniform mat4 projMat;
attribute vec2 vertPos;
attribute vec2 texCoords0;
varying vec2 texCoords;

void main( void )
{
	texCoords = vec2( texCoords0.s, -texCoords0.t ); 
	gl_Position = projMat * vec4( vertPos.x, vertPos.y, 1, 1 );
}

[[VS_OVERLAY_GL4]]

layout( location = 0 ) in vec2 vertPos;
layout( location = 1 ) in vec2 texCoords0;
out vec2 texCoords;

uniform mat4 projMat;
void main( void )
{
	texCoords = vec2( texCoords0.s, -texCoords0.t ); 
	gl_Position = projMat * vec4( vertPos.x, vertPos.y, 1, 1 );
}


[[FS_OVERLAY]]

uniform vec4 olayColor;
uniform sampler2D albedoMap;
varying vec2 texCoords;

void main( void )
{
	vec4 albedo = texture2D( albedoMap, texCoords );
	
	gl_FragColor = albedo * olayColor;
}

[[FS_OVERLAY_GL4]]

uniform vec4 olayColor;
uniform sampler2D albedoMap;
in vec2 texCoords;

out vec4 fragColor;

void main( void )
{
	vec4 albedo = texture2D( albedoMap, texCoords );
	
	fragColor = albedo * olayColor;
}