#include "Utils.h"

struct ParticleData
{
    vec4 color;
    float magnitude;
    float size;
    float pad0;
    float pad1;
};

// Contains both particles and nodes
layout(std430) coherent buffer Position { vec4 g_position[]; }; // .w - radius for nodes
layout(std430) coherent buffer Mass { float g_mass[]; };

// Contains only particles
layout(std430) buffer Velocity { vec4 g_velocity[]; };
layout(std430) buffer Acceleration { vec4 g_acceleration[]; };
layout(std430) buffer ParticlesData { ParticleData g_particle_data[]; };

// Contains child indices only for nodes
layout(std430) coherent buffer Children { int g_children[]; };

// Current free node index
layout(std430) coherent buffer NodesIndex { int g_cur_node_idx; };

// Final root bounding box radius
layout(std430) coherent buffer RootRadius { float g_radius; };

// Current free node index
layout(std430) buffer DebugBuffer { int g_debug[]; };

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
