#pragma once

#include <Interfaces/IRendererPlugin.h>
#include <Entity.h>

class Universe;

class GalaxyRenderer : public IRendererPlugin
{
public:
    GalaxyRenderer(Universe& universe, condition_variable* solver_cv = nullptr);

    virtual void CreatePipelines(RenderDevice& render_device) override;
    virtual void CreateSizeDependentResources(RenderDevice& render_device, const int2& output_size) override;
    virtual void DestroyResources() override;
    virtual void UpdatePipelines(GAL::ImagePtr& output_image) override;
    virtual void BindSceneDataBuffers() override;
    virtual vector<GAL::GraphicsPipelinePtr> GetPipelines() override;
    virtual void Render() override;

    atomic_bool& GetBufferUpdateRequestedFlag() { return buffer_update_requested_flag_; }

private:
    void CreateParticlesBuffer();
    void CreateNodesBuffers(size_t nodes_count);
    void UpdateParticlesBuffer();
    void UpdateNodesBuffers();
    void BindNodesBuffers();

private:
    Universe& universe_;

    Entity particles_positions_buffer_;
    Entity nodes_positions_;
    Entity nodes_sizes_;

    GAL::GraphicsPipelinePtr particles_render_pipeline_;
    GAL::GraphicsPipelinePtr tree_draw_pipeline_;

    atomic_bool buffer_update_requested_flag_ = true;
    condition_variable* solver_cv_ = nullptr;
};
