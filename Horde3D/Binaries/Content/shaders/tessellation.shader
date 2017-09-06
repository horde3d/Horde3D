[[FX]]

// Supported Flags
/* ---------------
	
	
*/

// Samplers
sampler2D albedoMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

// Uniforms
float tessLevelInner = 1;
float tessLevelOuter = 1;

float4 matDiffuseCol <
	string desc_abc = "abc: diffuse color";
	string desc_d   = "d: alpha for opacity";
> = {1.0, 1.0, 1.0, 1.0};
float4 matAmbientCol <
	string desc_abc = "abc: ambient color";
	string desc_d   = "d: alpha for opacity";
> = {0.0, 0.0, 1.0, 1.0};
float4 matSpecParams <
	string desc_abc = "abc: specular color";
	string desc_d   = "d: gloss";
> = {0.4, 0.4, 0.4, 0.5};


// Contexts
OpenGL4
{
	context LIGHTING
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		GeometryShader = compile GLSL GS_WIREFRAME_GL4;
		TessControlShader = compile GLSL TS_CONTROL_GL4;
		TessEvalShader = compile GLSL TS_EVAL_GL4;
		PixelShader = compile GLSL FS_LIGHTING_GL4;
	
		ZWriteEnable = false;
		BlendMode = Add;
		TessPatchVertices = 3;
	}
	
	context SHADOWMAP
	{
		VertexShader = compile GLSL VS_SHADOWMAP_GL4;
		GeometryShader = compile GLSL GS_WIREFRAME_SHADOW_GL4;
		TessControlShader = compile GLSL TS_CONTROL_SHADOW_GL4;
		TessEvalShader = compile GLSL TS_EVAL_SHADOW_GL4;
		PixelShader = compile GLSL FS_SHADOWMAP_GL4;
		
		TessPatchVertices = 3;
	}
	
	context AMBIENT
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		GeometryShader = compile GLSL GS_WIREFRAME_GL4;
		TessControlShader = compile GLSL TS_CONTROL_GL4;
		TessEvalShader = compile GLSL TS_EVAL_GL4;
		PixelShader = compile GLSL FS_AMBIENT_GL4;
		
		TessPatchVertices = 3;
	}
}


[[VS_GENERAL_GL4]]
// =================================================================================================
#include "shaders/utilityLib/vertCommon.glsl"

layout( location = 0 ) in vec3 vertPos;
layout( location = 1 ) in vec3 normal;
//layout( location = 5 ) in vec2 texCoords0;

out vec4 pos;
out vec3 normalVS;
//out vec2 texCoords;


void main( void )
{
	pos = vec4( vertPos, 1 );
	normalVS = normal;
//	texCoords = texCoords0;
}


[[VS_SHADOWMAP_GL4]]
// =================================================================================================
	
#include "shaders/utilityLib/vertCommon.glsl"
#include "shaders/utilityLib/vertSkinningGL4.glsl"

uniform mat4 viewProjMat;
uniform vec4 lightPos;

layout( location = 0 ) in vec3 vertPos;
layout( location = 1 ) in vec3 normal;

out vec4 pos;
out vec3 lightVec;
out vec3 normalVS;

#ifdef _F05_AlphaTest
	layout( location = 5 ) in vec2 texCoords0;
	out vec2 texCoords;
#endif

void main( void )
{
	vec4 pos = vec4( vertPos, 1.0 );

#ifdef _F05_AlphaTest
	texCoords = texCoords0;
#endif

	normalVS = normal;
	lightVec = lightPos.xyz - pos.xyz;
//	gl_Position = viewProjMat * pos;
}


[[TS_CONTROL_GL4]]
// =================================================================================================
layout( vertices = 3 ) out;
in vec4 pos[];
in vec3 normalVS[];
out vec4 controlPos[];
out vec3 normalTsc[];

uniform float tessLevelInner;
uniform float tessLevelOuter;


void main()
{
    controlPos[ gl_InvocationID ] = pos[ gl_InvocationID ];
	normalTsc[ gl_InvocationID ] = normalVS[ gl_InvocationID ];
	
    if ( gl_InvocationID == 0) 
	{
        gl_TessLevelInner[0] = tessLevelInner;
        gl_TessLevelOuter[0] = tessLevelOuter;
        gl_TessLevelOuter[1] = tessLevelOuter;
        gl_TessLevelOuter[2] = tessLevelOuter;
    }
}


[[TS_CONTROL_SHADOW_GL4]]
// =================================================================================================
layout( vertices = 3 ) out;
in vec4 pos[];
in vec3 normalVS[];
in vec3 lightVec[];
out vec4 controlPos[];
out vec3 normalTsc[];
out vec3 lightVecTsc[];

uniform float tessLevelInner;
uniform float tessLevelOuter;


void main()
{
    controlPos[ gl_InvocationID ] = pos[ gl_InvocationID ];
	normalTsc[ gl_InvocationID ] = normalVS[ gl_InvocationID ];
	lightVecTsc[ gl_InvocationID ] = lightVec[ gl_InvocationID ];
	
    if ( gl_InvocationID == 0) 
	{
        gl_TessLevelInner[0] = tessLevelInner;
        gl_TessLevelOuter[0] = tessLevelOuter;
        gl_TessLevelOuter[1] = tessLevelOuter;
        gl_TessLevelOuter[2] = tessLevelOuter;
    }
}


[[TS_EVAL_GL4]]
// =================================================================================================
#include "shaders/utilityLib/vertCommon.glsl"

layout(triangles, equal_spacing, ccw) in;
in vec4 controlPos[];
in vec3 normalTsc[];

out vec4 evalPos;
out vec3 patchDistance;
out vec3 normalTEval;

uniform mat4 viewProjMat;
uniform vec3 viewerPos;

vec3 interpolateNormals( vec3 v1, vec3 v2, vec3 v3 )
{
	return vec3(gl_TessCoord.x) * v1 + vec3(gl_TessCoord.y) * v2 + vec3(gl_TessCoord.z) * v3;
}

void main()
{
    vec3 p0 = gl_TessCoord.x * controlPos[0].xyz;
    vec3 p1 = gl_TessCoord.y * controlPos[1].xyz;
    vec3 p2 = gl_TessCoord.z * controlPos[2].xyz;
    
	normalTEval = interpolateNormals( calcWorldVec( normalTsc[ 0 ] ), calcWorldVec( normalTsc[ 1 ] ), calcWorldVec( normalTsc[ 2 ] ) );
	normalTEval = normalize( normalTEval );
	
	patchDistance = gl_TessCoord;
    evalPos = vec4( normalize(p0 + p1 + p2), 1 );
    gl_Position = viewProjMat * calcWorldPos( evalPos );
}


[[TS_EVAL_SHADOW_GL4]]
// =================================================================================================
#include "shaders/utilityLib/vertCommon.glsl"

layout(triangles, equal_spacing, ccw) in;
in vec4 controlPos[];
in vec3 normalTsc[];
in vec3 lightVecTsc[];

out vec4 evalPos;
out vec3 patchDistance;
out vec3 normalTEval;
out vec3 lightVecTEval;

uniform mat4 viewProjMat;
uniform vec3 viewerPos;

vec3 interpolateNormals( vec3 v1, vec3 v2, vec3 v3 )
{
	return vec3(gl_TessCoord.x) * v1 + vec3(gl_TessCoord.y) * v2 + vec3(gl_TessCoord.z) * v3;
}

void main()
{
    vec3 p0 = gl_TessCoord.x * controlPos[0].xyz;
    vec3 p1 = gl_TessCoord.y * controlPos[1].xyz;
    vec3 p2 = gl_TessCoord.z * controlPos[2].xyz;
    
	normalTEval = interpolateNormals( calcWorldVec( normalTsc[ 0 ] ), calcWorldVec( normalTsc[ 1 ] ), calcWorldVec( normalTsc[ 2 ] ) );
	normalTEval = normalize( normalTEval );
	
	lightVecTEval = vec3( normalize( gl_TessCoord.x * lightVecTsc[0].xyz + gl_TessCoord.y * lightVecTsc[1].xyz + gl_TessCoord.z * lightVecTsc[2].xyz) );
	
	patchDistance = gl_TessCoord;
    evalPos = vec4( normalize(p0 + p1 + p2), 1 );
    gl_Position = viewProjMat * calcWorldPos( evalPos );
}


[[GS_WIREFRAME_GL4]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

//uniform mat3 worldNormalMat;
// uniform mat4 viewMat;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec4 evalPos[3];
in vec3 patchDistance[3];
in vec3 normalTEval[ 3 ];

out vec3 facetNormal;
out vec3 patchDistanceGS;
out vec3 triDistance;
out vec4 vertPosGS;
out vec4 vsPosGS;

void main()
{
//    vec3 A = evalPos[2].xyz - evalPos[0].xyz;
//    vec3 B = evalPos[1].xyz - evalPos[0].xyz;
// vec3 A = evalPos[1].xyz - evalPos[0].xyz;
// vec3 B = evalPos[2].xyz - evalPos[0].xyz;

	
	facetNormal = normalTEval[ 0 ];
	
    patchDistanceGS = patchDistance[0];
    triDistance = vec3(1, 0, 0);
	vertPosGS = gl_in[0].gl_Position;
	vsPosGS = calcViewPos( vertPosGS );
    gl_Position = gl_in[0].gl_Position; EmitVertex();

    patchDistanceGS = patchDistance[1];
    triDistance = vec3(0, 1, 0);
	vertPosGS = gl_in[1].gl_Position;
	vsPosGS = calcViewPos( vertPosGS );
    gl_Position = gl_in[1].gl_Position; EmitVertex();

    patchDistanceGS = patchDistance[2];
    triDistance = vec3(0, 0, 1);
	vertPosGS = gl_in[2].gl_Position;
	vsPosGS = calcViewPos( vertPosGS );
    gl_Position = gl_in[2].gl_Position; EmitVertex();

    EndPrimitive();
}


[[GS_WIREFRAME_SHADOW_GL4]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

//uniform mat3 worldNormalMat;
// uniform mat4 viewMat;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec4 evalPos[3];
in vec3 patchDistance[3];
in vec3 normalTEval[ 3 ];
in vec3 lightVecTEval[ 3 ];

out vec3 facetNormal;
out vec3 patchDistanceGS;
out vec3 triDistance;
out vec4 vertPosGS;
out vec4 vsPosGS;
out vec3 lightVec;

void main()
{
//    vec3 A = evalPos[2].xyz - evalPos[0].xyz;
//    vec3 B = evalPos[1].xyz - evalPos[0].xyz;
// vec3 A = evalPos[1].xyz - evalPos[0].xyz;
// vec3 B = evalPos[2].xyz - evalPos[0].xyz;

	
	facetNormal = normalTEval[ 0 ];
	
    patchDistanceGS = patchDistance[0];
    triDistance = vec3(1, 0, 0);
	vertPosGS = gl_in[0].gl_Position;
	vsPosGS = calcViewPos( vertPosGS );
	lightVec = lightVecTEval[ 0 ];
    gl_Position = gl_in[0].gl_Position; EmitVertex();

    patchDistanceGS = patchDistance[1];
    triDistance = vec3(0, 1, 0);
	vertPosGS = gl_in[1].gl_Position;
	vsPosGS = calcViewPos( vertPosGS );
	lightVec = lightVecTEval[ 1 ];
    gl_Position = gl_in[1].gl_Position; EmitVertex();

    patchDistanceGS = patchDistance[2];
    triDistance = vec3(0, 0, 1);
	vertPosGS = gl_in[2].gl_Position;
	vsPosGS = calcViewPos( vertPosGS );
	lightVec = lightVecTEval[ 2 ];
    gl_Position = gl_in[2].gl_Position; EmitVertex();

    EndPrimitive();
}


[[FS_LIGHTING_GL4]]
// =================================================================================================

#include "shaders/utilityLib/fragLightingGL4.glsl" 

uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform vec4 matAmbientCol;

in vec3 facetNormal;
in vec3 triDistance;
in vec3 patchDistanceGS;
in vec4 vertPosGS;
in vec4 vsPosGS;
//in vec3 vertNormGS;

out vec4 fragColor;

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

void main( void )
{
	vec3 normal = normalize( facetNormal );
	vec3 newPos = vertPosGS.xyz;
	
//	float df = abs( dot( facetNormal, lightPos.xyz ) );
	vec3 color = matAmbientCol.rgb + /* df * */ matDiffuseCol.rgb;
		
	float d1 = min( min( triDistance.x, triDistance.y ), triDistance.z );
	float d2 = min( min( patchDistanceGS.x, patchDistanceGS.y ), patchDistanceGS.z );
	color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;
		
	//fragColor.rgb = color;

	// vec3 color = matAmbientCol.rgb + matDiffuseCol.rgb;
		
	// float d1 = min( min( triDistance.x, triDistance.y ), triDistance.z );
	// float d2 = min( min( patchDistanceGS.x, patchDistanceGS.y ), patchDistanceGS.z );
	// color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;
		
	fragColor.rgb = calcPhongSpotLight( newPos, normal, color.rgb, matSpecParams.rgb,
										matSpecParams.a, -vsPosGS.z, 0.3 );

}

[[FS_SHADOWMAP_GL4]]
// =================================================================================================

uniform vec4 lightPos;
uniform float shadowBias;
in vec3 lightVec;

#ifdef _F05_AlphaTest
	uniform vec4 matDiffuseCol;
	uniform sampler2D albedoMap;
	in vec2 texCoords;
#endif

void main( void )
{
#ifdef _F05_AlphaTest
	vec4 albedo = texture2D( albedoMap, texCoords * vec2( 1, -1 ) ) * matDiffuseCol;
	if( albedo.a < 0.01 ) discard;
#endif
	
	float dist = length( lightVec ) / lightPos.w;
//	gl_FragDepth = dist + shadowBias;
	
	// Clearly better bias but requires SM 3.0
	gl_FragDepth = dist + abs( dFdx( dist ) ) + abs( dFdy( dist ) ) + shadowBias;
}

[[FS_AMBIENT_GL4]]
// =================================================================================================
in vec3 facetNormal;
in vec3 triDistance;
in vec3 patchDistanceGS;
in vec4 vertPosGS;
in vec3 vertNormGS;

uniform vec4 matDiffuseCol;
uniform vec4 matAmbientCol;

out vec4 fragColor;

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

void main()
{
    vec3 color = matAmbientCol.rgb + matDiffuseCol.rgb;

    float d1 = min( min( triDistance.x, triDistance.y ), triDistance.z );
    float d2 = min( min( patchDistanceGS.x, patchDistanceGS.y ), patchDistanceGS.z );
    color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;

    fragColor = vec4(color, 1.0);
}