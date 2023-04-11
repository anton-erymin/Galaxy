#include "ShadeLayout.h"
#include "ParticlesLayout.h"

layout(points) in;
layout(points, max_vertices = 1) out;

void main()
{
	vec4 pos = g_positions[gl_PrimitiveIDIn];
    gl_Position = g_camera.projection_view_transform * pos;
    EmitVertex();
    EndPrimitive();
}
