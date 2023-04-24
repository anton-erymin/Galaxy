#include "CommonData.h"
#include "Utils.h"

const int TREE_CHILDREN_COUNT = 8;
const int NULL_INDEX = -1;
const int LOCK_INDEX = -2;

uint GetBodyCount() { return s_body_count; }
//uint GetNodeMaxCount() { return s_nodes_max_count; }
int GetRootNode() { return int(s_total_count) - 1; }
//int GetActualNodeCount() { return GetRootNode() - int(g_cur_node_idx); }
//int GetNodeUniIndex(int array_index) { return array_index + int(GetBodyCount()); }
int GetNodeArrayIndex(int uni_index) { return uni_index - int(GetBodyCount()); }
bool IsNull(int index) { return index == NULL_INDEX; }
bool IsBody(int index) { return !IsNull(index) && index < int(GetBodyCount()); }
bool IsBodyOrNull(int index) { return index < int(GetBodyCount()); }
bool IsNode(int index) { return index >= int(GetBodyCount()); }
bool IsLocked(int index) { return index == LOCK_INDEX; }

vec4 GetPosition(int i) { return g_position[i]; }
void SetPosition(int i, vec4 pos) { g_position[i] = pos; }
float GetMass(int i) { return g_mass[i]; }
void SetMass(int i, float mass) { g_mass[i] = mass; }
vec3 GetVelocity(int i) { return g_velocity[i].xyz; }
void SetVelocity(int i, vec3 vel) { g_velocity[i].xyz = vel; }
vec3 GetAcceleration(int i) { return g_acceleration[i].xyz; }
void SetAcceleration(int i, vec3 acc) { g_acceleration[i].xyz = acc; }

int GetChildIndex(int uni_index, int child_branch) 
{
	return g_children[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch];
}

void SetChildIndex(int uni_index, int child_branch, int child_index) 
{
	g_children[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch] = child_index;
}

bool TryLockChild(int uni_index, int child_branch, int cur_child_index)
{
	int off = GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch;
	return atomicCompSwap(g_children[off], cur_child_index, LOCK_INDEX) == cur_child_index;
}

int FindChildBranch(vec3 node_center, vec3 body_pos)
{
    int branch = 0;
    if (body_pos.x > node_center.x) branch = 1;
    if (body_pos.y > node_center.y) branch += 2;
    if (body_pos.z > node_center.z) branch += 4;
    return branch;
}

vec3 GetChildCenterPos(vec3 node_center, int child_branch, float radius)
{
    float half_radius = 0.5 * radius;
    vec3 child_offset = 
        vec3((child_branch & 1) * radius, 
            ((child_branch >> 1) & 1) * radius,
            ((child_branch >> 2) & 1) * radius);

    vec3 pos = node_center - vec3(half_radius) + child_offset;
    return pos;
}

int AddNode(vec4 node_center_pos)
{	
    int new_node = atomicAdd(g_cur_node_idx, -1);
	
    if (!IsNode(new_node))
    {
        //NLOG_FATAL("BarnesHutCPUTree: No space left for nodes");
    }

    // Initialize new node
    SetPosition(new_node, node_center_pos);
    SetMass(new_node, -1.0);
    for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
    {
        SetChildIndex(new_node, i, NULL_INDEX);
    }
    return new_node;
}

void ResetTree(BBox bbox)
{
    float max_len = BBox_MaxExtent(bbox);
    vec3 bbox_center = BBox_Center(bbox);

	float root_radius = 0.5 * max_len;
    g_radius = root_radius;
    g_cur_node_idx = GetRootNode();

    // Setup root node
    AddNode(vec4(bbox_center, root_radius));
}
