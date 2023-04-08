#define NODE_CHILD_COUNT 4

struct Particle
{
    vec4 position; //.w - mass.
    vec4 velocity; // .w - inverse mass
    vec4 acceleration;
    vec4 force;
};

struct BarnesHutNode
{
    vec4 position;
    
    float mass;
    int depth;
    int pad0;
    int pad1;
    
    // if child is -1 it's empty
    // if child is -2 it's locked
    // if child < NODES_COUNT this is node
    // if child >= NODES_COUNT this is particle at (child - NODES_COUNT)
    // Only .x is valid.
    ivec4 child[NODE_CHILD_COUNT];
};

struct ParticlesUpdateRootConstants
{
    float time;
    uint pad[3];
};
