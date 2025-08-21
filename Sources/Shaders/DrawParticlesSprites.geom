#include "ShadeLayout.h"
#include "SimulationLayout.h"

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec4 frag_color;
out vec2 frag_uv;

ROOT_CONSTANTS
{
    uint g_offset;
    float g_size_scale;
    float g_brightness;
};

void main()
{
	vec4 pos = g_position[g_offset + gl_PrimitiveIDIn];
	ParticleData data = g_particle_data[g_offset + gl_PrimitiveIDIn];
    
    frag_color.xyz = data.color.xyz;
    
    vec3 v1 = vec3(g_camera.view_transform[0].x, g_camera.view_transform[1].x, g_camera.view_transform[2].x);
    vec3 v2 = vec3(g_camera.view_transform[0].y, g_camera.view_transform[1].y, g_camera.view_transform[2].y);
        
    float s = 0.5 * data.size * g_size_scale;
    vec4 p0 = vec4(pos.xyz - v1 * s + v2 * s, 1.0);
    vec4 p1 = vec4(pos.xyz + v1 * s + v2 * s, 1.0);
    vec4 p2 = vec4(pos.xyz - v1 * s - v2 * s, 1.0);
    vec4 p3 = vec4(pos.xyz + v1 * s - v2 * s, 1.0);
                    
    gl_Position = g_camera.projection_view_transform * p0;
    frag_uv = vec2(0.0, 1.0);
    EmitVertex();
    
    gl_Position = g_camera.projection_view_transform * p2;
    frag_uv = vec2(0.0, 0.0);
    EmitVertex();
    
    gl_Position = g_camera.projection_view_transform * p1;
    frag_uv = vec2(1.0, 1.0);
    EmitVertex();
    
    gl_Position = g_camera.projection_view_transform * p3;
    frag_uv = vec2(1.0, 0.0);
    EmitVertex();
    
    EndPrimitive();
}
