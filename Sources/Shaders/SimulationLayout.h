#include "Utils.h"

// Contains both particles and nodes
layout(std430) buffer Position { vec4 g_position[]; };
layout(std430) buffer Mass { float g_mass[]; };

// Contains only particles
layout(std430) buffer Velocity { vec4 g_velocity[]; };
layout(std430) buffer Acceleration { vec4 g_acceleration[]; };

// Contains child indices only for nodes
layout(std430) buffer Children { int g_children[]; };

// Current free node index
layout(std430) buffer NodesIndex { int g_cur_node_idx; };

// Final root bounding box radius
layout(std430) buffer RootRadius{ float g_radius; };

layout(std140) uniform SimulationParameters
{
	uint g_body_count;
	uint g_nodes_max_count;
	uint g_total_count; // Bodies + Nodes available to allocate
	float g_timestep;
	float g_gravity_softening_length;
    float g_barnes_hut_opening_angle;
	uint pad0;
	uint pad1;
};
