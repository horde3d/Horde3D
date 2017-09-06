[[FX]]

// Samplers
sampler2D heightNormMap;
sampler2D detailMap;

// Uniforms
float4 sunDir = {1.0, -1.0, 0, 0};

// Contexts
context ATTRIBPASS
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_ATTRIBPASS;
}

context SHADOWMAP
{
	VertexShader = compile GLSL VS_SHADOWMAP;
	PixelShader = compile GLSL FS_SHADOWMAP;
}

context LIGHTING
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_LIGHTING;
	
	ZWriteEnable = false;
	BlendMode = Add;
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

	context SHADOWMAP
	{
		VertexShader = compile GLSL VS_SHADOWMAP_GL4;
		PixelShader = compile GLSL FS_SHADOWMAP_GL4;
	}

	context LIGHTING
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		PixelShader = compile GLSL FS_LIGHTING_GL4;
		
		ZWriteEnable = false;
		BlendMode = Add;
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
uniform vec4 terBlockParams;
attribute vec3 vertPos;
attribute float terHeight;
varying vec4 pos, vsPos;
varying vec2 texCoords;

void main( void )
{
	vec4 newPos = vec4( vertPos.x * terBlockParams.z + terBlockParams.x, terHeight,
						vertPos.z * terBlockParams.z + terBlockParams.y, 1.0 );
						
	pos = calcWorldPos( newPos );
	vsPos = calcViewPos( pos );
						
	texCoords = vec2( newPos.x, newPos.z );
	gl_Position = viewProjMat * pos;
}

[[VS_GENERAL_GL4]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform mat4 viewProjMat;
uniform vec4 terBlockParams;
layout( location = 0 ) in vec3 vertPos;
layout( location = 1 ) in float terHeight;
out vec4 pos, vsPos;
out vec2 texCoords;

void main( void )
{
	vec4 newPos = vec4( vertPos.x * terBlockParams.z + terBlockParams.x, terHeight,
						vertPos.z * terBlockParams.z + terBlockParams.y, 1.0 );
						
	pos = calcWorldPos( newPos );
	vsPos = calcViewPos( pos );
						
	texCoords = vec2( newPos.x, newPos.z );
	gl_Position = viewProjMat * pos;
}


[[FS_ATTRIBPASS]]
// =================================================================================================

#include "shaders/utilityLib/fragDeferredWrite.glsl"

uniform vec3 viewerPos;
uniform vec4 sunDir;
uniform sampler2D heightNormMap, detailMap;
varying vec4 pos;
varying vec2 texCoords;

vec3 light = -normalize( sunDir.xyz );

void main( void )
{
	vec3 detailCol = texture2D( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture2D( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	setMatID( 1.0 );
	setPos( pos.xyz - viewerPos );
	setNormal( normalize( normal ) );
	setAlbedo( detailCol );
	setSpecParams( vec3( 0.0, 0.0, 0.0 ), 0.0 );
}
				
[[FS_ATTRIBPASS_GL4]]
// =================================================================================================

#include "shaders/utilityLib/fragDeferredWriteGL4.glsl"

uniform vec3 viewerPos;
uniform vec4 sunDir;
uniform sampler2D heightNormMap, detailMap;
in vec4 pos;
in vec2 texCoords;

vec3 light = -normalize( sunDir.xyz );

void main( void )
{
	vec3 detailCol = texture( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	setMatID( 1.0 );
	setPos( pos.xyz - viewerPos );
	setNormal( normalize( normal ) );
	setAlbedo( detailCol );
	setSpecParams( vec3( 0.0, 0.0, 0.0 ), 0.0 );
}


[[VS_SHADOWMAP]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform mat4 viewProjMat;
uniform vec4 lightPos;
uniform vec4 terBlockParams;
attribute vec3 vertPos;
attribute float terHeight;
varying float dist;

void main( void )
{
	vec4 newPos = vec4( vertPos.x * terBlockParams.z + terBlockParams.x, terHeight,
						vertPos.z * terBlockParams.z + terBlockParams.y, 1.0 );
						
	vec4 pos = calcWorldPos( newPos );
	dist = length( lightPos.xyz - pos.xyz ) / lightPos.w;
						
	gl_Position = viewProjMat * pos;
}

[[VS_SHADOWMAP_GL4]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform mat4 viewProjMat;
uniform vec4 lightPos;
uniform vec4 terBlockParams;
layout( location = 0 ) in vec3 vertPos;
layout( location = 1 ) in float terHeight;
out float dist;

void main( void )
{
	vec4 newPos = vec4( vertPos.x * terBlockParams.z + terBlockParams.x, terHeight,
						vertPos.z * terBlockParams.z + terBlockParams.y, 1.0 );
						
	vec4 pos = calcWorldPos( newPos );
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

[[FS_SHADOWMAP_GL4]]
// =================================================================================================

uniform float shadowBias;
in float dist;

out float fragDepth;

void main( void )
{
	fragDepth = dist + shadowBias;
}


[[FS_LIGHTING]]
// =================================================================================================

#include "shaders/utilityLib/fragLighting.glsl"

uniform sampler2D heightNormMap, detailMap;
varying vec4 pos, vsPos;
varying vec2 texCoords;

void main( void )
{
	vec3 detailCol = texture2D( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture2D( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	gl_FragColor.rgb =
		calcPhongSpotLight( pos.xyz, normalize( normal ), detailCol, vec3( 0.0, 0.0, 0.0 ), 16.0, -vsPos.z, 0.3 );
}

[[FS_LIGHTING_GL4]]
// =================================================================================================

#include "shaders/utilityLib/fragLightingGL4.glsl"

uniform sampler2D heightNormMap, detailMap;
in vec4 pos, vsPos;
in vec2 texCoords;

out vec4 fragColor;

void main( void )
{
	vec3 detailCol = texture( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	fragColor.rgb =
		calcPhongSpotLight( pos.xyz, normalize( normal ), detailCol, vec3( 0.0, 0.0, 0.0 ), 16.0, -vsPos.z, 0.3 );
}


[[FS_AMBIENT]]
// =================================================================================================

uniform vec4 sunDir;
uniform sampler2D heightNormMap, detailMap;
varying vec2 texCoords;

vec3 light = -normalize( sunDir.xyz );

void main( void )
{
	vec3 detailCol = texture2D( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture2D( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	// Wrap lighting fur sun
	float l = max( dot( normal, light ), 0.0 ) * 0.5 + 0.5;
	
	gl_FragColor = vec4( detailCol * l, 1.0 );
}

[[FS_AMBIENT_GL4]]
// =================================================================================================

uniform vec4 sunDir;
uniform sampler2D heightNormMap, detailMap;
in vec2 texCoords;

out vec4 fragColor;

vec3 light = -normalize( sunDir.xyz );

void main( void )
{
	vec3 detailCol = texture( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	// Wrap lighting fur sun
	float l = max( dot( normal, light ), 0.0 ) * 0.5 + 0.5;
	
	fragColor = vec4( detailCol * l, 1.0 );
}