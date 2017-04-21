[[FX]]

// Samplers
sampler2D buf0 = sampler_state
{
	Address = Clamp;
};

sampler2D buf1 = sampler_state
{
	Address = Clamp;
};

// Uniforms
float hdrExposure = 2.0;       // Exposure (higher values make scene brighter)
float hdrBrightThres = 0.6;    // Brightpass threshold (intensity where blooming begins)
float hdrBrightOffset = 0.06;  // Brightpass offset (smaller values produce stronger blooming)

float4 blurParams = {0, 0, 0, 0};

// Contexts
context BRIGHTPASS
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_BRIGHTPASS;
	
	ZWriteEnable = false;
}

context BLUR
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_BLUR;
	
	ZWriteEnable = false;
}

context FINALPASS
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_FINALPASS;
	
	ZWriteEnable = false;
}


[[VS_FSQUAD]]
// =================================================================================================

uniform mat4 projMat;
attribute vec3 vertPos;
varying vec2 texCoords;
				
void main( void )
{
	texCoords = vertPos.xy; 
	gl_Position = projMat * vec4( vertPos, 1 );
}


[[FS_BRIGHTPASS]]
// =================================================================================================

#include "shaders/utilityLib/fragPostProcess.glsl"

uniform sampler2D buf0;
uniform vec2 frameBufSize;
//uniform float hdrExposure;
uniform float hdrBrightThres;
uniform float hdrBrightOffset;
varying vec2 texCoords;

void main( void )
{
	vec2 texSize = frameBufSize * 4.0;
	vec2 coord2 = texCoords + vec2( 2, 2 ) / texSize;
	
	// Average using bilinear filtering
	vec4 sum = getTex2DBilinear( buf0, texCoords, texSize );
	sum += getTex2DBilinear( buf0, coord2, texSize );
	sum += getTex2DBilinear( buf0, vec2( coord2.x, texCoords.y ), texSize );
	sum += getTex2DBilinear( buf0, vec2( texCoords.x, coord2.y ), texSize );
	sum /= 4.0;
	
	// Tonemap
	//sum = 1.0 - exp2( -hdrExposure * sum );
	
	// Extract bright values
	sum = max( sum - hdrBrightThres, 0.0 );
	sum /= hdrBrightOffset + sum;
	
	gl_FragColor = sum;
}

	
[[FS_BLUR]]
// =================================================================================================

#include "shaders/utilityLib/fragPostProcess.glsl"

uniform sampler2D buf0;
uniform vec2 frameBufSize;
uniform vec4 blurParams;
varying vec2 texCoords;

void main( void )
{
	gl_FragColor = blurKawase( buf0, texCoords, frameBufSize, blurParams.x );
}
	

[[FS_FINALPASS]]
// =================================================================================================

uniform sampler2D buf0, buf1;
uniform vec2 frameBufSize;
uniform float hdrExposure;
varying vec2 texCoords;

void main( void )
{
	vec4 col0 = texture2D( buf0, texCoords );	// HDR color
	vec4 col1 = texture2D( buf1, texCoords );	// Bloom
	
	// Tonemap (using photographic exposure mapping)
	vec4 col = 1.0 - exp2( -hdrExposure * col0 );
	
	gl_FragColor = col + col1;
}
