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
    node = node == NULL_INDEX ? tree_->GetRootNode() : node;
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
    BEGIN_TIME_MEASURE(build_tree_timer, context_.build_tree_time_msecs);

    // 1. Build tree

    tree_->BuildTree();

    universe_.node_positions_.clear();
    universe_.node_sizes_.clear();
    // Traverse tree
    if (render_params_.render_tree)
    {
        TraverseTree();
    }

    context_.nodes_count = universe_.node_positions_.size();

    tree_->SummarizeTree();

    END_TIME_MEASURE(build_tree_timer);

    // 2. Compute force

    size_t count = universe_.positions_.size();

    auto ComputeForceKernel = [&](THREAD_POOL_KERNEL_ARGS)
    {
        assert(global_id < count);

        if (universe_.masses_[global_id] == 0.0f)
        {
            return;
        }

        universe_.forces_[global_id] = tree_->ComputeAcceleration(
            global_id, context_.gravity_softening_length, context_.barnes_hut_opening_angle);
    };

    BEGIN_TIME_MEASURE(solving_timer, context_.compute_force_time_msecs);
    PARALLEL_FOR(count, ComputeForceKernel);
    END_TIME_MEASURE(solving_timer);
}
