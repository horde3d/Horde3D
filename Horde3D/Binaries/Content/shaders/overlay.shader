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


[[FS_OVERLAY]]

uniform vec4 olayColor;
uniform sampler2D albedoMap;
varying vec2 texCoords;

void main( void )
{
	vec4 albedo = texture2D( albedoMap, texCoords );
	
	gl_FragColor = albedo * olayColor;
}
