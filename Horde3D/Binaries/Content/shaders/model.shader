[[FX]]

// Supported Flags
/* ---------------
	_F01_Skinning
	_F02_NormalMapping
	_F03_ParallaxMapping
	_F04_EnvMapping
	_F05_AlphaTest
*/


// Samplers
sampler2D albedoMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

sampler2D normalMap = sampler_state
{
	Texture = "textures/common/defnorm.tga";
};

samplerCube ambientMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

samplerCube envMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Uniforms
float4 matDiffuseCol <
	string desc_abc = "abc: diffuse color";
	string desc_d   = "d: alpha for opacity";
> = {1.0, 1.0, 1.0, 1.0};

float4 matSpecParams <
	string desc_abc = "abc: specular color";
	string desc_d   = "d: gloss";
> = {0.04, 0.04, 0.04, 0.5};

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


[[VS_GENERAL]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/vertCommon.glsl"

#ifdef _F01_Skinning
	#include "shaders/utilityLib/vertSkinning.glsl"
#endif

uniform mat4 viewProjMat;
uniform vec3 viewerPos;
attribute vec3 vertPos;
attribute vec2 texCoords0;
attribute vec3 normal;

#ifdef _F02_NormalMapping
	attribute vec4 tangent;
#endif

varying vec4 pos, vsPos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif


void main( void )
{
#ifdef _F01_Skinning
	mat4 skinningMat = calcSkinningMat();
	mat3 skinningMatVec = getSkinningMatVec( skinningMat );
#endif
	
	// Calculate normal
#ifdef _F01_Skinning
	vec3 _normal = normalize( calcWorldVec( skinVec( normal, skinningMatVec ) ) );
#else
	vec3 _normal = normalize( calcWorldVec( normal ) );
#endif

	// Calculate tangent and bitangent
#ifdef _F02_NormalMapping
	#ifdef _F01_Skinning
		vec3 _tangent = normalize( calcWorldVec( skinVec( tangent.xyz, skinningMatVec ) ) );
	#else
		vec3 _tangent = normalize( calcWorldVec( tangent.xyz ) );
	#endif
	
	vec3 _bitangent = cross( _normal, _tangent ) * tangent.w;
	tsbMat = calcTanToWorldMat( _tangent, _bitangent, _normal );
#else
	tsbNormal = _normal;
#endif

	// Calculate world space position
#ifdef _F01_Skinning	
	pos = calcWorldPos( skinPos( vec4( vertPos, 1.0 ), skinningMat ) );
#else
	pos = calcWorldPos( vec4( vertPos, 1.0 ) );
#endif

	vsPos = calcViewPos( pos );

	// Calculate tangent space eye vector
#ifdef _F03_ParallaxMapping
	eyeTS = calcTanVec( viewerPos - pos.xyz, _tangent, _bitangent, _normal );
#endif
	
	// Calculate texture coordinates and clip space position
	texCoords = texCoords0;
	gl_Position = viewProjMat * pos;
}


[[FS_ATTRIBPASS]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragDeferredWrite.glsl" 

uniform vec3 viewerPos;
uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying vec4 pos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture2D( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	setMatID( 1.0 );
	setPos( newPos - viewerPos );
	setNormal( normalize( normal ) );
	setAlbedo( albedo.rgb );
	setSpecParams( matSpecParams.rgb, matSpecParams.a );
}

	
[[VS_SHADOWMAP]]
// =================================================================================================
	
#include "shaders/utilityLib/vertCommon.glsl"
#include "shaders/utilityLib/vertSkinning.glsl"

uniform mat4 viewProjMat;
uniform vec4 lightPos;
attribute vec3 vertPos;
varying vec3 lightVec;

#ifdef _F05_AlphaTest
	attribute vec2 texCoords0;
	varying vec2 texCoords;
#endif

void main( void )
{
#ifdef _F01_Skinning	
	vec4 pos = calcWorldPos( skinPos( vec4( vertPos, 1.0 ) ) );
#else
	vec4 pos = calcWorldPos( vec4( vertPos, 1.0 ) );
#endif

#ifdef _F05_AlphaTest
	texCoords = texCoords0;
#endif

	lightVec = lightPos.xyz - pos.xyz;
	gl_Position = viewProjMat * pos;
}
	
	
[[FS_SHADOWMAP]]
// =================================================================================================

uniform vec4 lightPos;
uniform float shadowBias;
varying vec3 lightVec;

#ifdef _F05_AlphaTest
	uniform vec4 matDiffuseCol;
	uniform sampler2D albedoMap;
	varying vec2 texCoords;
#endif

void main( void )
{
#ifdef _F05_AlphaTest
	vec4 albedo = texture2D( albedoMap, texCoords * vec2( 1, -1 ) ) * matDiffuseCol;
	if( albedo.a < 0.01 ) discard;
#endif
	
	float dist = length( lightVec ) / lightPos.w;
	gl_FragDepth = dist + shadowBias;
	
	// Clearly better bias but requires SM 3.0
	//gl_FragDepth = dist + abs( dFdx( dist ) ) + abs( dFdy( dist ) ) + shadowBias;
}


[[FS_LIGHTING]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragLighting.glsl" 

uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying vec4 pos, vsPos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture2D( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	gl_FragColor.rgb =
		calcPhongSpotLight( newPos, normalize( normal ), albedo.rgb, matSpecParams.rgb,
		                    matSpecParams.a, -vsPos.z, 0.3 );
}


[[FS_AMBIENT]]	
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragLighting.glsl" 

uniform sampler2D albedoMap;
uniform samplerCube ambientMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

#ifdef _F04_EnvMapping
	uniform samplerCube envMap;
#endif

varying vec4 pos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture2D( albedoMap, newCoords.st );
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif
	
	gl_FragColor.rgb = albedo.rgb * textureCube( ambientMap, normal ).rgb;
	
#ifdef _F04_EnvMapping
	vec3 refl = textureCube( envMap, reflect( pos.xyz - viewerPos, normalize( normal ) ) ).rgb;
	gl_FragColor.rgb = refl * 1.5;
#endif
}
