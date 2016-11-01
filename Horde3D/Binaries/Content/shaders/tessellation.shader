[[FX]]

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

// Contexts
OpenGL4
{
/*	context LIGHTING
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		GeometryShader = compile GLSL GS_WIREFRAME_GL4;
		TessControlShader = compile GLSL TS_CONTROL_GL4;
		TessEvalShader = compile GLSL TS_EVAL_GL4;
		PixelShader = compile GLSL FS_LIGHTING_GL4;
	
		ZWriteEnable = false;
		BlendMode = Add;
		TessPatchVertices = 16;
	}
	*/
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
//layout( location = 5 ) in vec2 texCoords0;

out vec4 pos;
//out vec2 texCoords;


void main( void )
{
	pos = vec4( vertPos, 1 );	
//	texCoords = texCoords0;
}

[[TS_CONTROL_GL4]]
// =================================================================================================
layout( vertices = 3 ) out;
in vec4 pos[];
out vec4 controlPos[];

uniform float tessLevelInner;
uniform float tessLevelOuter;


void main()
{
    controlPos[ gl_InvocationID ] = pos[ gl_InvocationID ];
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

layout(triangles, equal_spacing, cw) in;
in vec4 controlPos[];
out vec4 evalPos;
out vec3 patchDistance;

uniform mat4 viewProjMat;
uniform vec3 viewerPos;

void main()
{
    vec3 p0 = gl_TessCoord.x * controlPos[0].xyz;
    vec3 p1 = gl_TessCoord.y * controlPos[1].xyz;
    vec3 p2 = gl_TessCoord.z * controlPos[2].xyz;
    
	patchDistance = gl_TessCoord;
    evalPos = vec4( normalize(p0 + p1 + p2), 1 );
    gl_Position = viewProjMat * calcWorldPos( evalPos );
}

[[GS_WIREFRAME_GL4]]
// =================================================================================================

uniform mat3 worldNormalMat;
uniform mat4 viewMat;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec4 evalPos[3];
in vec3 patchDistance[3];
out vec3 facetNormal;
out vec3 patchDistanceGS;
out vec3 triDistance;
out vec4 vertPosGS;

void main()
{
    vec3 A = evalPos[2].xyz - evalPos[0].xyz;
    vec3 B = evalPos[1].xyz - evalPos[0].xyz;
	mat4 transposedMat = transpose( viewMat );
	
    facetNormal = /*mat3( transposedMat )*/ worldNormalMat * normalize(cross(A, B));
    
    patchDistanceGS = patchDistance[0];
    triDistance = vec3(1, 0, 0);
	vertPosGS = gl_in[0].gl_Position;
    gl_Position = gl_in[0].gl_Position; EmitVertex();

    patchDistanceGS = patchDistance[1];
    triDistance = vec3(0, 1, 0);
	vertPosGS = gl_in[1].gl_Position;
    gl_Position = gl_in[1].gl_Position; EmitVertex();

    patchDistanceGS = patchDistance[2];
    triDistance = vec3(0, 0, 1);
	vertPosGS = gl_in[2].gl_Position;
    gl_Position = gl_in[2].gl_Position; EmitVertex();

    EndPrimitive();
}

[[FS_LIGHTING_GL4]]
// =================================================================================================

#include "shaders/utilityLib/fragLightingGL4.glsl" 

uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform vec4 matAmbientCol;
uniform mat4 viewMat;

in vec4 vertPosGS;
in vec2 texCoords;

in vec3 facetNormal;

out vec4 fragColor;

void main( void )
{
	vec3 N = normalize( facetNormal );
    float df = abs( dot( N, lightPos.xyz ) );
    vec3 albedo = matAmbientCol.rgb + df * matDiffuseCol.rgb;

	vec3 newPos = vertPosGS.xyz;
	vec4 vsPos = viewMat * vertPosGS;
	
	fragColor.rgb = calcPhongSpotLight( newPos, N, albedo.rgb, matSpecParams.rgb,
										matSpecParams.a, -vsPos.z, 0.3 );
}

[[FS_AMBIENT_GL4]]
// =================================================================================================
in vec3 facetNormal;
in vec3 triDistance;
in vec3 patchDistanceGS;
in vec4 vertPosGS;

uniform vec4 lightPos;
uniform vec4 matDiffuseCol;
uniform vec4 matAmbientCol;
uniform vec3 viewerPos;

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
    vec3 N = normalize( facetNormal );
    float df = abs( dot( N, /*vec3( 1, 1, 1 )*/ viewerPos.xyz ) );
    vec3 color = matAmbientCol.rgb + /*df * */ matDiffuseCol.rgb;

    float d1 = min( min( triDistance.x, triDistance.y ), triDistance.z );
    float d2 = min( min( patchDistanceGS.x, patchDistanceGS.y ), patchDistanceGS.z );
    color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;

//    fragColor = vec4(1.0, 1.0, 1.0, 1.0);

    fragColor = vec4(color, 1.0);
}