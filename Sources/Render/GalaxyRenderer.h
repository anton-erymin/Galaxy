#pragma once

#include <Interfaces/IRendererPlugin.h>
#include <Entity.h>

class Universe;

class GalaxyRenderer : public IRendererPlugin
{
public:
    GalaxyRenderer(Universe& universe);

    virtual void CreatePipelines(RenderDevice& render_device) override;
    virtual void CreateSizeDependentResources(RenderDevice& render_device, const int2& output_size) override;
    virtual void DestroyResources() override;
    virtual void UpdatePipelines(GAL::ImagePtr& output_image) override;
    virtual void BindSceneDataBuffers() override;
    virtual vector<GAL::GraphicsPipelinePtr> GetPipelines() override;
    virtual void Render() override;

private:
    void CreateParticlesBuffer();
    void FillParticlesBuffer();

private:
    Universe& universe_;

    Entity particles_positions_ = kInvalidEntity;

    GAL::GraphicsPipelinePtr particles_render_pipeline_;
    GAL::GraphicsPipelinePtr tree_draw_pipeline_;
};
