#include "GalaxyRenderer.h"
#include "Universe.h"
#include "Particle.h"
#include "GalaxySimulator/GalaxyTypes.h"
#include "GalaxySimulator/GalaxyModule.h"

#include <Engine.h>
#include <Renderer.h>
#include <RendererCore.h>
#include <Private/RenderDevice.h>
#include <GAL.h>
#include <OpenGL/OpenGLGraphics.h>
#include <Data/DeviceData.h>
#include <Misc/Paths.h>
#include <Debugging/Profiler.h>
#include <ResourceManager.h>
#include <Interfaces/IImageSystem.h>

#pragma pack(push, 1)

struct DrawParticlesSpritesRootConstants
{
    uint32 g_offset;
    float g_size_scale;
    float g_brightness;
};

struct ParticleData
{
    Float4 color;
    float magnitude;
    float size;
    float pad0;
    float pad1;
};

#pragma pack(pop)

GalaxyRenderer::GalaxyRenderer(Universe& universe, SimulationContext& sim_context, 
    const RenderParameters& render_params)
    : universe_(universe)
    , sim_context_(sim_context)
    , render_params_(render_params)
{
    ShaderTools::ShaderManager& sm = engine->GetRenderer().GetRendererCore().GetRenderDevice().GetShaderManager();

    // Add additional shaders directory
    sm.AddShadersPath(Paths::BaseDir() + "/../../../Sources/Shaders"); // Galaxy shaders dir

    particle_images_.EmplaceBack(ResourceManager::GetResource("star.png", ResourceLoadFlags::POST_LOAD_INITIALIZE));
    particle_images_.EmplaceBack(ResourceManager::GetResource("dust1.png", ResourceLoadFlags::POST_LOAD_INITIALIZE));
    particle_images_.EmplaceBack(ResourceManager::GetResource("dust2.png", ResourceLoadFlags::POST_LOAD_INITIALIZE));
    particle_images_.EmplaceBack(ResourceManager::GetResource("dust3.png", ResourceLoadFlags::POST_LOAD_INITIALIZE));

    for (ResourcePtr& image : particle_images_)
    {
        image->As<ImageResource>()->PostLoadInitialize(true);
    }

    // Create batches
    uint32 cur_texture_idx = 0;
    uint32 start = 0;
    size_t i = 0;
    for (; i < universe_.GetParticlesCount(); i++)
    {
        const Particle* particle = universe_.GetParticles()[i];
        if (particle->texture_idx != cur_texture_idx)
        {
            render_batches_.EmplaceBack(RenderBatch{ start, uint32(i) - start, cur_texture_idx });
            cur_texture_idx = particle->texture_idx;
            start = i;
        }
    }

    if (uint32(i) - start)
    {
        render_batches_.EmplaceBack(RenderBatch{ start, uint32(i) - start });
    }
}

void GalaxyRenderer::CreatePipelines(RenderDevice& render_device)
{
    particles_render_sprite_pipeline_= render_device.CreateGraphicsPipeline(SID("DrawParticlesSprites.geom"), SID("DrawParticlesSprites.vert"), SID("DrawParticlesSprites.frag"));
    particles_render_points_pipeline_ = render_device.CreateGraphicsPipeline(SID("DrawParticlesPoints.geom"), SID("ShadeSingleColor.vert"), SID("ShadeSingleColor.frag"));
    tree_draw_pipeline_ = render_device.CreateGraphicsPipeline(SID("DrawBarnesHut.geom"), SID("ShadeSingleColor.vert"), SID("ShadeSingleColor.frag"));
}

void GalaxyRenderer::CreateSizeDependentResources(RenderDevice& render_device, const Int2& output_size)
{
}

void GalaxyRenderer::DestroyResources()
{
}

void GalaxyRenderer::UpdatePipelines(GAL::ImagePtr& output_image)
{
    {
        GAL::PipelineState state = {};
        state.SetColorAttachment(0, output_image);
        state.SetRootConstantsSize(sizeof(DrawParticlesSpritesRootConstants));
        state.blending.is_enabled = true;
        state.depth_test_enabled = false;
        particles_render_sprite_pipeline_->SetState(state);

        particles_render_sprite_pipeline_->SetImage(SID("g_image"), 0);
    }

    {
        GAL::PipelineState state = {};
        state.SetColorAttachment(0, output_image);
        state.SetRootConstantsSize(sizeof(Device::ShadeSingleColorRootConstants));
        state.blending.is_enabled = true;
        particles_render_points_pipeline_->SetState(state);

        Device::ShadeSingleColorRootConstants root_constants = {};
        root_constants.color = Math::kWhiteColor;
        root_constants.transform = Matrix();
        particles_render_points_pipeline_->SetRootConstants(&root_constants);
    }

    {
        GAL::PipelineState state = {};
        state.SetColorAttachment(0, output_image);
        state.SetRootConstantsSize(sizeof(Device::ShadeSingleColorRootConstants));
        tree_draw_pipeline_->SetState(state);

        Device::ShadeSingleColorRootConstants root_constants = {};
        root_constants.color = Math::kGreenColor;
        root_constants.transform = Matrix();
        tree_draw_pipeline_->SetRootConstants(&root_constants);
    }
}

void GalaxyRenderer::BindSceneDataBuffers()
{
    if (!particles_positions_buffer_)
    {
        CreateParticlesBuffer();
    }

    if (particles_render_points_pipeline_)
    {
        particles_render_points_pipeline_->SetBuffer(particles_positions_buffer_, SID("Position"));
        particles_render_sprite_pipeline_->SetBuffer(particles_positions_buffer_, SID("Position"));

        particles_render_sprite_pipeline_->SetBuffer(particles_data_buffer_, SID("ParticlesData"));
    }
}

Array<GAL::GraphicsPipelinePtr> GalaxyRenderer::GetPipelines()
{
    return Array({ particles_render_sprite_pipeline_, particles_render_points_pipeline_, tree_draw_pipeline_ });
}

void GalaxyRenderer::Render()
{
    PROFILER_BLOCK_GPU("Galaxy Render");

    if (aux_updatable_)
    {
        // GPU solver is called here before render part
        aux_updatable_->Update();
    }

    if (render_params_.colors_inverted)
    {
        particles_render_points_pipeline_->SetClearColor(Math::kWhiteColor);
    }
    else
    {
        particles_render_points_pipeline_->SetClearColor(Math::kBlackColor);
    }

    particles_render_points_pipeline_->ClearColorAttachments();

    if (sim_context_.IsCPUAlgorithm() && sim_context_.positions_update_completed_flag)
    {
        UpdateParticlesBuffer();

        if (sim_context_.algorithm == SimulationAlgorithm::BARNESHUT_CPU)
        {
            UpdateNodesBuffers();
        }

        sim_context_.positions_update_completed_flag = false;

        // After update positions buffer signal to solver
        // so that it will be able to update postions during integration phase
        sim_context_.solver_cv.notify_one();
    }

    // Draw particles
    if (render_params_.render_particles)
    {
        PROFILER_BLOCK_GPU("Render Particles");

        if (render_params_.render_as_points)
        {
            particles_render_points_pipeline_->SetPointSize(render_params_.particle_size_scale);

            Device::ShadeSingleColorRootConstants root_constants = {};
            Float4 particle_color = render_params_.colors_inverted ? Math::kBlackColor : Math::kWhiteColor;
            root_constants.color = particle_color;
            root_constants.transform = Matrix();
            particles_render_points_pipeline_->SetRootConstants(&root_constants);

            particles_render_points_pipeline_->BeginGraphics();
            particles_render_points_pipeline_->Draw(0, universe_.GetParticlesCount(), GAL::PrimitiveType::Points);
            particles_render_points_pipeline_->EndGraphics();
        }
        else
        {
            particles_render_sprite_pipeline_->BeginGraphics();
            for (const RenderBatch& batch : render_batches_)
            {
                DrawParticlesSpritesRootConstants root_constants = { batch.offset, render_params_.particle_size_scale, render_params_.brightness };
                particles_render_sprite_pipeline_->SetRootConstants(&root_constants);

                iengine->ImageSystem()->GetDeviceImage(particle_images_[batch.texture_idx]->GetEntity())->Bind(0);

                particles_render_sprite_pipeline_->Draw(0, batch.count, GAL::PrimitiveType::Points);
            }
            particles_render_sprite_pipeline_->EndGraphics();
        }
    }

    // Draw tree
    if (sim_context_.IsBarnesHut() && render_params_.render_tree)
    {
        PROFILER_BLOCK_GPU("Draw Tree");

        size_t nodes_count = universe_.node_positions_.Size();
        tree_draw_pipeline_->BeginGraphics();
        tree_draw_pipeline_->Draw(0, nodes_count, GAL::PrimitiveType::Points);
        tree_draw_pipeline_->EndGraphics();
    }
}

void GalaxyRenderer::SetPositionBuffer(GAL::BufferPtr buffer)
{
    particles_positions_buffer_ = buffer;
    BindSceneDataBuffers();
}

void GalaxyRenderer::CreateParticlesBuffer()
{
    size_t count = universe_.GetParticlesCount();

    RenderDevice& device = engine->GetRenderer().GetRendererCore().GetRenderDevice();

    particles_positions_buffer_ = device.CreateBuffer(SID("ParticlesPositionsBuffer"), GAL::BufferType::kStorage, count * sizeof(Float4), GAL::BufferUsage::DynamicDraw);
    particles_data_buffer_ = device.CreateBuffer(SID("ParticlesDataBuffer"), GAL::BufferType::kStorage, count * sizeof(ParticleData), GAL::BufferUsage::StaticDraw);
}

void GalaxyRenderer::CreateNodesBuffers(size_t nodes_count)
{
    RenderDevice& device = engine->GetRenderer().GetRendererCore().GetRenderDevice();

    nodes_positions_buffer_ = device.CreateBuffer(SID("NodePositionsBuffer"), GAL::BufferType::kStorage, nodes_count * sizeof(Float4), GAL::BufferUsage::DynamicDraw);
}

void GalaxyRenderer::UpdateParticlesBuffer()
{
    if (!particles_positions_buffer_)
    {
        CreateParticlesBuffer();
    }

    size_t count = universe_.GetParticlesCount();
    particles_positions_buffer_->Write(0, count * sizeof(Float4), universe_.positions_.Data());

    static bool s_particles_data_uploaded = false;
    if (!s_particles_data_uploaded)
    {
        // Fill particles particle_data only once
        Array<ParticleData> particle_data(count);
        for (size_t i = 0; i < count; i++)
        {
            particle_data[i].color = universe_.GetParticles()[i]->color;
            particle_data[i].magnitude = universe_.GetParticles()[i]->magnitude;
            particle_data[i].size = universe_.GetParticles()[i]->size;
        }
        particles_data_buffer_->Write(0, count * sizeof(ParticleData), particle_data.Data());

        s_particles_data_uploaded = true;
    }
}

void GalaxyRenderer::UpdateNodesBuffers()
{
    size_t nodes_count = universe_.node_positions_.Size();

    if (nodes_count == 0)
    {
        return;
    }

    size_t required_size = nodes_count * sizeof(Float4);
    if (!nodes_positions_buffer_ || nodes_positions_buffer_->GetSize() < required_size)
    {
        CreateNodesBuffers(nodes_count);
        BindNodesBuffers();
    }

    nodes_positions_buffer_->Write(0, nodes_count * sizeof(Float4), universe_.node_positions_.Data());
}

void GalaxyRenderer::BindNodesBuffers()
{
    tree_draw_pipeline_->SetBuffer(nodes_positions_buffer_, SID("NodePositions"));
}
