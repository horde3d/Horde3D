[[FX]]

// Samplers
sampler2D albedoMap = sampler_state
{
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Contexts


OpenGL4
{
	// context COMPUTE
	// {
		// ComputeShader = compile GLSL CS_PARTICLESOLVER;
	// }
	
	context AMBIENT
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
//		GeometryShader = compile GLSL GS_TRIANGULATE_GL4;
		PixelShader = compile GLSL FS_AMBIENT_GL4;
		
		ZWriteEnable = false;
		BlendMode = AddBlended;
	}
}

[[CS_PARTICLESOLVER]]
// =================================================================================================


[[VS_GENERAL_GL4]]
// =================================================================================================
#include "shaders/utilityLib/vertCommon.glsl"

layout (location = 0) in vec3 partPosition;
layout (location = 1) in vec3 partVelocity;

// uniform mat4 viewMat;
// uniform mat4 projMat;
uniform mat4 viewProjMat;

void main( void )
{
//	vec4 viewPos = vec4( partPosition, 1 ) * viewMat;
//	gl_Position = viewPos * projMat;

	vec4 pos = calcWorldPos( vec4( partPosition, 1 ) );
	gl_Position = viewProjMat * pos;
}

[[FS_AMBIENT_GL4]]
// =================================================================================================
uniform sampler2D albedoMap;

out vec4 fragColor;

void main()
{
	fragColor = vec4( 1.0, 1.0, 0.1, 1.0 );
}
