#include "SceneData.h"
#include "ShadeLayout.h"

layout(points) in;
layout(line_strip, max_vertices = 20) out;

layout(std430) buffer NodePositions
{
    vec4 g_node_positions[];
};

layout(std430) buffer NodeSizes
{
    float g_node_radius[];
};

void DrawQuad(mat4 proj_view, vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
    gl_Position = proj_view * vec4(v0, 1.0);
    EmitVertex();
    
    gl_Position = proj_view * vec4(v1, 1.0);
    EmitVertex();
    
    gl_Position = proj_view * vec4(v2, 1.0);
    EmitVertex();
    
    gl_Position = proj_view * vec4(v3, 1.0);
    EmitVertex();
    
    gl_Position = proj_view * vec4(v0, 1.0);
    EmitVertex();
    
    EndPrimitive();
}

void DrawLine(mat4 proj_view, vec3 v0, vec3 v1)
{
    gl_Position = proj_view * vec4(v0, 1.0);
    EmitVertex();
    
    gl_Position = proj_view * vec4(v1, 1.0);
    EmitVertex();
	
	EndPrimitive();
}

void main()
{
    vec3 node_pos = g_node_positions[gl_PrimitiveIDIn].xyz;
    float radius = g_node_radius[gl_PrimitiveIDIn];
    
    mat4 proj_view = g_camera.projection_view_transform;

	// Top
    vec3 v0 = node_pos + vec3(-radius, -radius, -radius);
    vec3 v1 = node_pos + vec3(-radius, -radius, radius);
    vec3 v2 = node_pos + vec3(radius, -radius, radius);
    vec3 v3 = node_pos + vec3(radius, -radius, -radius);

	// Bottom
	vec3 v4 = node_pos + vec3(-radius, radius, -radius);
    vec3 v5 = node_pos + vec3(-radius, radius, radius);
    vec3 v6 = node_pos + vec3(radius, radius, radius);
    vec3 v7 = node_pos + vec3(radius, radius, -radius);
	
    DrawQuad(proj_view, v0, v1, v2, v3);
	DrawQuad(proj_view, v4, v5, v6, v7);
	DrawLine(proj_view, v0, v4);
	DrawLine(proj_view, v1, v5);
	DrawLine(proj_view, v2, v6);
	DrawLine(proj_view, v3, v7);
}
