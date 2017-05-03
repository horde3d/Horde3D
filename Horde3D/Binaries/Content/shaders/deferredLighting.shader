[[FX]]

// Samplers
sampler2D depthBuf = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf0 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf1 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf2 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf3 = sampler_state
{
	Address = Clamp;
};

samplerCube ambientMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Contexts
context AMBIENT
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_AMBIENT;
	
	ZWriteEnable = false;
	BlendMode = Replace;
}

context LIGHTING
{
	VertexShader = compile GLSL VS_VOLUME;
	PixelShader = compile GLSL FS_LIGHTING;
	
	ZWriteEnable = false;
	BlendMode = Add;
}

context COPY_DEPTH
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_COPY_DEPTH;
}

OpenGL4
{
	context AMBIENT
	{
		VertexShader = compile GLSL VS_FSQUAD_GL4;
		PixelShader = compile GLSL FS_AMBIENT_GL4;
		
		ZWriteEnable = false;
		BlendMode = Replace;
	}

	context LIGHTING
	{
		VertexShader = compile GLSL VS_VOLUME_GL4;
		PixelShader = compile GLSL FS_LIGHTING_GL4;
		
		ZWriteEnable = false;
		BlendMode = Add;
	}

	context COPY_DEPTH
	{
		VertexShader = compile GLSL VS_FSQUAD_GL4;
		PixelShader = compile GLSL FS_COPY_DEPTH_GL4;
	}
}


[[VS_FSQUAD]]

uniform mat4 projMat;
attribute vec3 vertPos;
varying vec2 texCoords;
				
void main( void )
{
	texCoords = vertPos.xy; 
	gl_Position = projMat * vec4( vertPos, 1 );
}

[[VS_FSQUAD_GL4]]

uniform mat4 projMat;

layout( location = 0 ) in vec3 vertPos;
out vec2 texCoords;
				
void main( void )
{
	texCoords = vertPos.xy; 
	gl_Position = projMat * vec4( vertPos, 1 );
}


[[VS_VOLUME]]

uniform mat4 viewProjMat;
uniform mat4 worldMat;
attribute vec3 vertPos;
varying vec4 vpos;
				
void main( void )
{
	vpos = viewProjMat * worldMat * vec4( vertPos, 1 );
	gl_Position = vpos;
}

[[VS_VOLUME_GL4]]

uniform mat4 viewProjMat;
uniform mat4 worldMat;

layout( location = 0 ) in vec3 vertPos;
out vec4 vpos;
				
void main( void )
{
	vpos = viewProjMat * worldMat * vec4( vertPos, 1 );
	gl_Position = vpos;
}


[[FS_AMBIENT]]

#include "shaders/utilityLib/fragDeferredRead.glsl"

uniform samplerCube ambientMap;
varying vec2 texCoords;

void main( void )
{
	if( getMatID( texCoords ) == 0.0 )	// Background
	{
		gl_FragColor.rgb = vec3( 0, 0, 0 );
	}
	else if( getMatID( texCoords ) == 2.0 )	// Sky
	{
		gl_FragColor.rgb = getAlbedo( texCoords );
	}
	else
	{
		gl_FragColor.rgb = getAlbedo( texCoords ) * textureCube( ambientMap, getNormal( texCoords ) ).rgb;
	}
}

[[FS_AMBIENT_GL4]]

#include "shaders/utilityLib/fragDeferredReadGL4.glsl"

uniform samplerCube ambientMap;
in vec2 texCoords;

out vec4 fragColor;

void main( void )
{
	if( getMatID( texCoords ) == 0.0 )	// Background
	{
		fragColor.rgb = vec3( 0, 0, 0 );
	}
	else if( getMatID( texCoords ) == 2.0 )	// Sky
	{
		fragColor.rgb = getAlbedo( texCoords );
	}
	else
	{
		fragColor.rgb = getAlbedo( texCoords ) * texture( ambientMap, getNormal( texCoords ) ).rgb;
	}
}


[[FS_LIGHTING]]

#include "shaders/utilityLib/fragLighting.glsl"
#include "shaders/utilityLib/fragDeferredRead.glsl"

uniform mat4 viewMat;
varying vec4 vpos;

void main( void )
{
	vec2 fragCoord = (vpos.xy / vpos.w) * 0.5 + 0.5;
	
	if( getMatID( fragCoord ) == 1.0 )	// Standard phong material
	{
		vec3 pos = getPos( fragCoord ) + viewerPos;
		float vsPos = (viewMat * vec4( pos, 1.0 )).z;
		vec4 specParams = getSpecParams( fragCoord );
		
		gl_FragColor.rgb =
			calcPhongSpotLight( pos, getNormal( fragCoord ),
								getAlbedo( fragCoord ), specParams.rgb, specParams.a, -vsPos, 0.3 );
	}
	else discard;
}

[[FS_LIGHTING_GL4]]

#include "shaders/utilityLib/fragLightingGL4.glsl"
#include "shaders/utilityLib/fragDeferredReadGL4.glsl"

uniform mat4 viewMat;
in vec4 vpos;

out vec4 fragColor;

void main( void )
{
	vec2 fragCoord = (vpos.xy / vpos.w) * 0.5 + 0.5;
	
	if( getMatID( fragCoord ) == 1.0 )	// Standard phong material
	{
		vec3 pos = getPos( fragCoord ) + viewerPos;
		float vsPos = (viewMat * vec4( pos, 1.0 )).z;
		vec4 specParams = getSpecParams( fragCoord );
		
		fragColor.rgb = calcPhongSpotLight( pos, getNormal( fragCoord ),
											getAlbedo( fragCoord ), specParams.rgb, specParams.a, -vsPos, 0.3 );
	}
	else discard;
}


[[FS_COPY_DEPTH]]

uniform sampler2D depthBuf;
varying vec2 texCoords;

void main( void )
{
	gl_FragDepth = texture2D( depthBuf, texCoords ).r;
}

[[FS_COPY_DEPTH_GL4]]

uniform sampler2D depthBuf;
in vec2 texCoords;

void main( void )
{
	gl_FragDepth = texture( depthBuf, texCoords ).r;
}