#pragma once

#include <Interfaces/IRendererPlugin.h>
#include <Entity.h>

class Universe;
struct SimulationContext;
struct RenderParameters;
class IUpdatable;

class GalaxyRenderer : public IRendererPlugin
{
public:
    GalaxyRenderer(Universe& universe, SimulationContext& sim_context, const RenderParameters& render_params);

    virtual void CreatePipelines(RenderDevice& render_device) override;
    virtual void CreateSizeDependentResources(RenderDevice& render_device, const int2& output_size) override;
    virtual void DestroyResources() override;
    virtual void UpdatePipelines(GAL::ImagePtr& output_image) override;
    virtual void BindSceneDataBuffers() override;
    virtual vector<GAL::GraphicsPipelinePtr> GetPipelines() override;
    virtual void Render() override;

    void SetUpdateHandler(IUpdatable* handler) { aux_updatable_ = handler; }

    void SetPositionBuffer(GAL::BufferPtr buffer);

private:
    void CreateParticlesBuffer();
    void CreateNodesBuffers(size_t nodes_count);
    void UpdateParticlesBuffer();
    void UpdateNodesBuffers();
    void BindNodesBuffers();

private:
    Universe& universe_;
    SimulationContext& sim_context_;
    const RenderParameters& render_params_;

    // For CPU solvers
    GAL::BufferPtr particles_positions_buffer_;
    GAL::BufferPtr nodes_positions_buffer_;

    GAL::GraphicsPipelinePtr particles_render_pipeline_;
    GAL::GraphicsPipelinePtr tree_draw_pipeline_;

    IUpdatable* aux_updatable_ = nullptr;
};
