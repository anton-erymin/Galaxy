#include "BarnesHutCPUSolver.h"
#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"
#include "BarnesHut/BarnesHutCPUTree.h"
#include "GalaxySimulator/GalaxyTypes.h"

#include <Thread/ThreadPool.h>

BarnesHutCPUSolver::BarnesHutCPUSolver(Universe& universe, SimulationContext& context)
    : CPUSolverBase(universe, context)
{
    tree_ = make_unique<BarnesHutCPUTree>(float2(), 0);
}

BarnesHutCPUSolver::~BarnesHutCPUSolver()
{
}

void BarnesHutCPUSolver::TraverseTree(const BarnesHutCPUTree& node)
{
    float2 node_pos = 0.5f * (node.point + node.oppositePoint);

    universe_.node_positions_.push_back(float4(node_pos.x, 0.0f, node_pos.y, 1.0f));
    universe_.node_sizes_.push_back(node.length);

    if (!node.IsLeaf())
    {
        for (size_t i = 0; i < 4; i++)
        {
            TraverseTree(*node.children[i]);
        }
    }
}

void BarnesHutCPUSolver::Solve(float time)
{
    size_t count = universe_.positions_.size();
    size_t thread_count = ThreadPool::GetThreadCount();
    size_t block_size = (count + thread_count - 1) / thread_count;

    // 1. Compute bounding box

    vector<BoundingBox> boxes(thread_count);
    auto BoundingBoxKernel = [&](size_t i)
    {
        size_t begin = i * block_size;
        size_t end = min((i + 1) * block_size, count - 1);

        for (size_t index = begin; index <= end; ++index)
        {
            boxes[i].grow(float3(universe_.positions_[index]));
        }
    };

    PARALLEL_FOR(thread_count, BoundingBoxKernel);

    // Compute final bounding box
    BoundingBox bbox;
    for (size_t i = 0; i < thread_count; i++)
    {
        bbox.grow(boxes[i]);
    }
    
    tree_->SetBoundingBox(bbox);

    // 2. Build tree
    tree_->Reset();
    for (size_t i = 0; i < count; i++)
    {
        const float4& pos = universe_.positions_[i];
        tree_->Insert(float2(pos.x, pos.z), universe_.masses_[i]);
    }

    // Traverse tree
    universe_.node_positions_.clear();
    universe_.node_sizes_.clear();
    TraverseTree(*tree_);

    context_.nodes_count = universe_.node_positions_.size();

    // 3. Compute force

    auto ComputeForceKernel = [&](size_t i)
    {
        assert(i < count);

        for (size_t j = i + 1; j < count; j++)
        {
            float3 l = float3(universe_.positions_[j]) - float3(universe_.positions_[i]);
            // TODO: Softended distance right?
            float dist = SoftenedDistance(l.length_sq(), cSoftFactor);
            float dist_cubic = dist * dist * dist;

            float3 force1 = GravityAcceleration(l, universe_.masses_[j], dist_cubic);
            float3 force2 = GravityAcceleration(-l, universe_.masses_[i], dist_cubic);

            {
                lock_guard<mutex> lock(force_mutexes_[i]);
                universe_.forces_[i] += force1;
            }

            {
                lock_guard<mutex> lock(force_mutexes_[j]);
                universe_.forces_[j] += force2;
            }
        }
    };

    auto IntegrationKernel = [&](size_t i)
    {
        assert(i < count);
        if (universe_.masses_[i] > 0.0f)
        {
            float3 pos = float3(universe_.positions_[i]);
            IntegrateMotionEquation(time, pos, universe_.velocities_[i],
                universe_.forces_[i], universe_.inverse_masses_[i]);
            universe_.positions_[i] = pos;
        }
        // Clear force accumulator
        universe_.forces_[i] = float3();
    };

    PARALLEL_FOR(count, ComputeForceKernel);

    // After computing force before integration phase wait for signal from renderer
    // that it has finished copying new positions into device buffer
    unique_lock<mutex> lock(context_.solver_mu);
    context_.solver_cv.wait(lock, [this]()
        {
            return context_.positions_update_completed_flag == false || !active_flag_;
        });

    // 4. Integrate

    // Now we can start update positions
    PARALLEL_FOR(count, IntegrationKernel);

    // After updating positions turn on flag signaling to renderer that it needs to update device buffer
    context_.positions_update_completed_flag = true;
}
