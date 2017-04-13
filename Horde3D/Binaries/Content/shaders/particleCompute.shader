[[FX]]

// Samplers
sampler2D albedoMap = sampler_state
{
	Texture = "textures/particles/compParticle.png";
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Buffers
buffer ParticleData;

// Uniforms
float totalParticles = 1000;
float deltaTime = 0;
float4 attractor = {0.0, 0.0, 0.0, 0.0};

// Contexts
OpenGL4
{
	context COMPUTE
	{
		ComputeShader = compile GLSL CS_PARTICLESOLVER;
	}
	
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

uniform float totalParticles;
uniform float deltaTime;
uniform vec4 attractor;

struct Particle
{
	vec4 position;
	vec4 velocity;
};

const int maxThreadsInGroup = 1024;
 
layout (std430, binding = 1) buffer ParticleData
{ 
	Particle particlesBuf[];
} data;

layout(local_size_x = 32, local_size_y = 32) in;

/////////////////////
vec3 calculate(vec3 anchor, vec3 position)
{
	vec3 direction = anchor - position;
	float dist = length(direction);
	direction /= dist;

	return direction * max(0.01, (1.0 / (dist * dist)));
}

void main()
{
	uint index = gl_WorkGroupID.x * maxThreadsInGroup + gl_WorkGroupID.y * gl_NumWorkGroups.x * maxThreadsInGroup + gl_LocalInvocationIndex; 

	if( index >= totalParticles ) 
		return;

	Particle particle = data.particlesBuf[ index ];

	vec4 position = particle.position; 
	vec4 velocity = particle.velocity; 

	velocity += vec4( calculate( attractor.xyz, position.xyz ), 0 );
	velocity += vec4( calculate( -attractor.xyz, position.xyz ), 0 ) ;
	
	particle.position = position + velocity * deltaTime;
	particle.velocity = velocity;

	data.particlesBuf[index] = particle;
}

[[VS_GENERAL_GL4]]
// =================================================================================================
#include "shaders/utilityLib/vertCommon.glsl"

layout (location = 0) in vec4 partPosition;
layout (location = 1) in vec4 partVelocity;

//uniform mat4 projMat;
uniform mat4 viewProjMat;

out vec3 partColor;

void main( void )
{
	vec4 pos = calcWorldPos( partPosition );
	
	float speed = length( partVelocity.xyz );
	partColor = mix(vec3(0.1, 0.5, 1.0), vec3(1.0, 0.5, 0.1), speed * 0.1 );
	
	gl_Position = pos;
}

[[GS_TRIANGULATE_GL4]]
// =================================================================================================
uniform mat4 viewMat;
uniform mat4 viewProjMat;
uniform vec3 viewerPos;
 
layout(points) in;
layout (triangle_strip) out;
layout(max_vertices = 4) out;
 
in vec3 partColor[];
 
out vec2 vertTexCoords;
out vec3 color;
 
void main()
{
// create billboards from points
	color = partColor[0];
	
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
    
}



[[FS_AMBIENT_GL4]]
// =================================================================================================
uniform sampler2D albedoMap;

in vec2 vertTexCoords;
in vec3 color;

out vec4 fragColor;

void main()
{
	vec4 texColor = texture( albedoMap, vec2( vertTexCoords.s, -vertTexCoords.t ) );
	if ( texColor.a < 0.1 ) discard;
	
	fragColor = vec4( color * texColor.xyz, texColor.a );
}
