#pragma once

#include "GPUSolverBase.h"

class BarnesHutGPUSolver : public GPUSolverBase
{
public:
    BarnesHutGPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BarnesHutGPUSolver();

private:
    virtual void ComputeAcceleration() override;

    virtual size_t GetPositionsBufferCount() const override { return universe_.GetParticlesCount() + GetNodesMaxCount(); }
    virtual size_t GetNodesMaxCount() const override { return max(int(3 * universe_.GetParticlesCount()), 20); }

    virtual void CreateBuffers() override;
    virtual void CreatePipelines() override;
    virtual void BindLayout(GAL::ComputePipelinePtr pipeline) override;

private:
    // Additional buffers for Tree structure

    GAL::ComputePipelinePtr bounding_box_pipeline_;
    GAL::ComputePipelinePtr build_pipeline_;
    GAL::ComputePipelinePtr summarize_pipeline_;
    GAL::ComputePipelinePtr sort_pipeline_;
    GAL::ComputePipelinePtr compute_acceleration_pipeline_;

    GAL::BufferPtr children_;
    GAL::BufferPtr nodes_index_;
    GAL::BufferPtr radius_;
    GAL::BufferPtr debug_;
};
