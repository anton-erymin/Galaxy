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
    tree_ = make_unique<BarnesHutCPUTree>();
}

BarnesHutCPUSolver::~BarnesHutCPUSolver()
{
}

void BarnesHutCPUSolver::TraverseTree(const BarnesHutCPUTree& node)
{
    float2 node_pos = 0.5f * (node.point_ + node.opposite_point_);

    universe_.node_positions_.push_back(float4(node_pos.x, 0.0f, node_pos.y, 1.0f));
    universe_.node_sizes_.push_back(node.length_);

    if (!node.is_leaf_)
    {
        for (size_t i = 0; i < 4; i++)
        {
            TraverseTree(*node.children_[i]);
        }
    }
}

void BarnesHutCPUSolver::Solve(float time)
{
    size_t count = universe_.positions_.size();

    BEGIN_TIME_MEASURE(total_timer, context_.total_step_time_msecs);
    BEGIN_TIME_MEASURE(build_tree_timer, context_.build_tree_time_msecs);
    BEGIN_TIME_MEASURE(solver_timer, context_.solver_time_msecs);

    // 1. Compute bounding box

    vector<BoundingBox> boxes(ThreadPool::GetThreadCount());
    auto BoundingBoxKernel = [&](THREAD_POOL_KERNEL_ARGS)
    {
        boxes[block_id].grow(float3(universe_.positions_[global_id]));
    };

    PARALLEL_FOR(count, BoundingBoxKernel);

    // Compute final bounding box
    BoundingBox bbox;
    for (size_t i = 0; i < boxes.size(); i++)
    {
        bbox.grow(boxes[i]);
    }
    
    // 2. Build tree

    tree_->Reset();
    tree_->SetBoundingBox(bbox);

    auto BuildTreeKernel = [&](THREAD_POOL_KERNEL_ARGS)
    {
        const float4& pos = universe_.positions_[global_id];
        tree_->Insert(float2(pos.x, pos.z), universe_.masses_[global_id]);
    };

    PARALLEL_FOR(count, BuildTreeKernel);
    //for (size_t i = 0; i < count; i++) { BuildTreeKernel(i, 0, 0, 0); }

    END_TIME_MEASURE(build_tree_timer);

    universe_.node_positions_.clear();
    universe_.node_sizes_.clear();
    // Traverse tree
    if (render_params_.render_tree)
    {
        TraverseTree(*tree_);
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

        if (global_id == 0)
        {
            return;
        }

        const float4& pos = universe_.positions_[global_id];

        float2 force = tree_->ComputeAcceleration(float2(pos.x, pos.z), context_.gravity_softening_length, context_.barnes_hut_opening_angle);
        universe_.forces_[global_id] = float3(force.x, 0.0f, force.y);
    };

    PARALLEL_FOR(count, ComputeForceKernel);

    END_TIME_MEASURE(solver_timer);

    // After computing force before integration phase wait for signal from renderer
    // that it has finished copying new positions into device buffer
    unique_lock<mutex> lock(context_.solver_mu);
    context_.solver_cv.wait(lock, [this]() { return context_.positions_update_completed_flag == false || !active_flag_; });

    // 4. Integrate    
    // Now we can start update positions

    PARALLEL_FOR_METHOD_BIND(count, BarnesHutCPUSolver::IntegrationKernel);

    END_TIME_MEASURE(total_timer);

    // After updating positions turn on flag signaling to renderer that it needs to update device buffer
    context_.positions_update_completed_flag = true;
}
