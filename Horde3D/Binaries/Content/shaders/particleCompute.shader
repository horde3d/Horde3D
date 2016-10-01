[[FX]]

// Samplers
sampler2D albedoMap = sampler_state
{
	Texture = "textures/particles/compParticle.png";
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
		GeometryShader = compile GLSL GS_TRIANGULATE_GL4;
		PixelShader = compile GLSL FS_AMBIENT_GL4;
		
//		ZWriteEnable = false;
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

//uniform mat4 projMat;
uniform mat4 viewProjMat;

void main( void )
{
//	vec4 viewPos = vec4( partPosition, 1 ) * viewMat;
//	gl_Position = viewPos * projMat;

	vec4 pos = calcWorldPos( vec4( partPosition, 1 ) );
//	partTexCoords = vec2( 0, 0 );
	
	gl_Position = pos;//viewProjMat * pos;
}

[[GS_TRIANGULATE_GL4]]
// =================================================================================================
uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 viewProjMat;
uniform vec3 viewerPos;
 
layout(points) in;
layout (triangle_strip) out;
layout(max_vertices = 4) out;
 
out vec2 vertTexCoords;
 
void main()
{
//    vec4 P = gl_in[0].gl_Position;
	float particle_size = 0.1;
	
	vec3 up = vec3( viewMat[0][1], viewMat[1][1], viewMat[2][1] );
				 
	vec3 P = gl_in[0].gl_Position.xyz;
	
	vec3 toCamera = normalize( viewerPos - P );
//	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross( toCamera, up );
	
	vec3 va = P - (right + up) * particle_size;
	gl_Position = viewProjMat * vec4(va, 1.0);
	vertTexCoords = vec2(0.0, 0.0);
	EmitVertex();  
	  
	vec3 vb = P - (right - up) * particle_size;
	gl_Position = viewProjMat * vec4(vb, 1.0);
	vertTexCoords = vec2(0.0, 1.0);
	EmitVertex();  

	vec3 vd = P + (right - up) * particle_size;
	gl_Position = viewProjMat * vec4(vd, 1.0);
	vertTexCoords = vec2(1.0, 0.0);
	EmitVertex();  

	vec3 vc = P + (right + up) * particle_size;
	gl_Position = viewProjMat * vec4(vc, 1.0);
	vertTexCoords = vec2(1.0, 1.0);
	EmitVertex();  
	  
	EndPrimitive();
  
	// a: left-bottom 
	// vec2 va = P.xy + vec2(-0.5, -0.5) * particle_size;
	// gl_Position = projMat * vec4(va, P.zw);
	// vertTexCoords = vec2(0.0, 0.0);
	// EmitVertex();  
	  
	// // b: left-top
	// vec2 vb = P.xy + vec2(-0.5, 0.5) * particle_size;
	// gl_Position = projMat * vec4(vb, P.zw);
	// vertTexCoords = vec2(0.0, 1.0);
	// EmitVertex();  
	  
	// // d: right-bottom
	// vec2 vd = P.xy + vec2(0.5, -0.5) * particle_size;
	// gl_Position = projMat * vec4(vd, P.zw);
	// vertTexCoords = vec2(1.0, 0.0);
	// EmitVertex();  

	// // c: right-top
	// vec2 vc = P.xy + vec2(0.5, 0.5) * particle_size;
	// gl_Position = projMat * vec4(vc, P.zw);
	// vertTexCoords = vec2(1.0, 1.0);
	// EmitVertex();  

	// EndPrimitive();  
}



[[FS_AMBIENT_GL4]]
// =================================================================================================
uniform sampler2D albedoMap;

in vec2 vertTexCoords;
out vec4 fragColor;

void main()
{
	vec4 texColor = texture( albedoMap, vec2( vertTexCoords.s, -vertTexCoords.t ) );
	fragColor = texColor;
}
