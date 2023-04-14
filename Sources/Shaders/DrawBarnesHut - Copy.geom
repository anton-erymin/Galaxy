#include "SceneData.h"
#include "ShadeLayout.h"

layout(points) in;
layout(line_strip, max_vertices = 20) out;

layout(std140) buffer NodesData
{
    Node g_nodes[];
};

layout(std140) buffer NodesCounter
{
    uint g_nodes_counter;
};

void DrawQuad(mat4 m, vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
    gl_Position = m * vec4(v0, 1.0);
    EmitVertex();
    
    gl_Position = m * vec4(v1, 1.0);
    EmitVertex();
    
    gl_Position = m * vec4(v2, 1.0);
    EmitVertex();
    
    gl_Position = m * vec4(v3, 1.0);
    EmitVertex();
    
    gl_Position = m * vec4(v0, 1.0);
    EmitVertex();
    
    EndPrimitive();
}

void main()
{
    if (gl_PrimitiveIDIn >= g_nodes_counter)
    {
        return;
    }
    
    Node node = g_nodes[gl_PrimitiveIDIn];
    
    mat4 m = g_camera.projection_view_transform;

    float radius = ROOT_RADIUS / float(1 << node.depth);
    
    vec3 v0 = node.position.xyz + vec3(-radius, 0.0, -radius);
    vec3 v1 = node.position.xyz + vec3(-radius, 0.0, radius);
    vec3 v2 = node.position.xyz + vec3(radius, 0.0, radius);
    vec3 v3 = node.position.xyz + vec3(radius, 0.0, -radius);
    
    DrawQuad(m, v0, v1, v2, v3);
    
    radius *= 0.5;
    
    for (int i = 0; i < NODE_CHILD_COUNT; ++i)
    {
        if (node.child[i].x >= NODES_MAX_COUNT)
        {
            // if child is particle draw quad.
            
            vec3 node_offset = vec3(0.0);
            node_offset.x = float((i >> 1) & 1) * radius;
            node_offset.z = float(i & 1) * radius;
             
            // Map from [0; R] to [-R; R].
            node_offset = 2 * node_offset - vec3(radius, 0.0, radius);
            
            vec3 child_node_position = node.position.xyz + node_offset;
            
            v0 = child_node_position + vec3(-radius, 0.0, -radius);
            v1 = child_node_position + vec3(-radius, 0.0, radius);
            v2 = child_node_position + vec3(radius, 0.0, radius);
            v3 = child_node_position + vec3(radius, 0.0, -radius);
            
            DrawQuad(m, v0, v1, v2, v3);
        }
    }
}
