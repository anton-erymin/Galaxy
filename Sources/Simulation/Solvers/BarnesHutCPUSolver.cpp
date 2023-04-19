#include "BarnesHutCPUSolver.h"
#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"
#include "BarnesHut/BarnesHutCPUTree.h"
#include "GalaxySimulator/GalaxyTypes.h"

#include <Thread/ThreadPool.h>

BarnesHutCPUSolver::BarnesHutCPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : CPUSolverBase(universe, context, render_params)
{
    tree_ = make_unique<BarnesHutCPUTree>(universe.positions_, universe_.masses_);
}

BarnesHutCPUSolver::~BarnesHutCPUSolver()
{
    Stop();
    tree_.reset();
}

void BarnesHutCPUSolver::TraverseTree(int32 node, float radius)
{
    node = node == NULL_INDEX ? tree_->GetRootIndex() : node;
    radius = radius == 0.0f ? tree_->radius_ : radius;
    float half_radius = 0.5f * radius;

    const float4& node_pos = tree_->GetPosition(node);

    universe_.node_positions_.push_back(float4(node_pos.x, 0.0f, node_pos.z, 1.0f));
    universe_.node_sizes_.push_back(radius);

    if (tree_->IsNode(node))
    {
        for (size_t i = 0; i < TREE_CHILDREN_COUNT; i++)
        {
            int32 child_index = tree_->GetChildIndex(node, i);
            if (tree_->IsNode(child_index))
            {
                TraverseTree(child_index, half_radius);
            }
            else if (tree_->IsBodyOrNull(child_index))
            {
                // For child refering to body add info here
                universe_.node_positions_.push_back(tree_->GetChildCenterPos(node_pos, i, radius));
                universe_.node_sizes_.push_back(half_radius);
            }
        }
    }
}

void BarnesHutCPUSolver::ComputeAcceleration()
{
    // 1. Compute bounding box

    BEGIN_TIME_MEASURE(build_tree_timer, context_.build_tree_time_msecs);

    vector<BoundingBox> boxes(ThreadPool::GetThreadCount());
    auto BoundingBoxKernel = [&](THREAD_POOL_KERNEL_ARGS)
    {
        boxes[block_id].grow(float3(universe_.positions_[global_id]));
    };

    size_t count = universe_.positions_.size();

    PARALLEL_FOR(count, BoundingBoxKernel);

    // Compute final bounding box
    BoundingBox bbox;
    for (size_t i = 0; i < boxes.size(); i++)
    {
        bbox.grow(boxes[i]);
    }

    // 2. Build tree

    tree_->SetBoundingBox(bbox);
    tree_->BuildTree();

    END_TIME_MEASURE(build_tree_timer);

    universe_.node_positions_.clear();
    universe_.node_sizes_.clear();
    // Traverse tree
    if (render_params_.render_tree)
    {
        TraverseTree();
    }

    context_.nodes_count = universe_.node_positions_.size();

    // 3. Compute force

    auto ComputeForceKernel = [&](THREAD_POOL_KERNEL_ARGS)
    {
        assert(global_id < count);

        if (universe_.masses_[global_id] == 0.0f)
        {
            return;
        }

        const float4& pos = universe_.positions_[global_id];

#if 0
        float2 force = tree_->ComputeAcceleration(float2(pos.x, pos.z), context_.gravity_softening_length,
            context_.barnes_hut_opening_angle);
        universe_.forces_[global_id] = float3(force.x, 0.0f, force.y);
#endif // 0

    };

    PARALLEL_FOR(count, ComputeForceKernel);
}
