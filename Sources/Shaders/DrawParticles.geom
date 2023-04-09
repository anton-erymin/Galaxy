#include "ParticleseData.h"
#include "ShadeLayout.h"

layout(points) in;
layout(points, max_vertices = 1) out;

layout(std140) buffer ParticlesData
{
	Particle g_particles[];
};

void main()
{
    gl_Position = g_camera.projection_view_transform * vec4(g_particles[gl_PrimitiveIDIn].position.xyz, 1.0);
    EmitVertex();
    EndPrimitive();
}
