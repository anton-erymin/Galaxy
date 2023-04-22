#include "GalaxyRenderer.h"
#include "Universe.h"
#include "Particle.h"
#include "GalaxySimulator/GalaxyTypes.h"

#include <Engine.h>
#include <Renderer.h>
#include <RendererCore.h>
#include <Private/RenderDevice.h>
#include <GAL.h>
#include <OpenGL/GraphicsOpenGL.h>
#include <Interfaces/IDeviceBufferSystem.h>
#include <Data/DeviceData.h>
#include <Misc/Paths.h>

namespace Device
{
#pragma pack(push, 1)
#include "../Shaders/ParticleseData.h"
#pragma pack(pop)
}

GalaxyRenderer::GalaxyRenderer(Universe& universe, SimulationContext& sim_context, 
    const RenderParameters& render_params)
    : universe_(universe)
    , sim_context_(sim_context)
    , render_params_(render_params)
{
    // Add additional shaders directory
    string shaders_path = Paths::BaseDir() + "/../../../Sources/Shaders";
    engine->GetRenderer().GetRendererCore().GetRenderDevice().GetShaderManager().
        AddShadersPath(shaders_path);

    CreateParticlesBuffer();
}

void GalaxyRenderer::CreatePipelines(RenderDevice& render_device)
{
    particles_render_pipeline_ = render_device.CreateGraphicsPipeline("DrawParticles.geom",  "ShadeSingleColor.vert", "ShadeSingleColor.frag");
    tree_draw_pipeline_ = render_device.CreateGraphicsPipeline("DrawBarnesHut.geom", "ShadeSingleColor.vert", "ShadeSingleColor.frag");

    test_pipeline_ = render_device.CreateComputePipeline("Test.comp");
}

void GalaxyRenderer::CreateSizeDependentResources(RenderDevice& render_device, const int2& output_size)
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
        state.SetRootConstantsSize(sizeof(Device::ShadeSingleColorRootConstants));
        particles_render_pipeline_->SetState(state);
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
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().
        DeviceBufferSystem();

    particles_render_pipeline_->SetBuffer(particles_positions_buffer_, "Position");
}

vector<GAL::GraphicsPipelinePtr> GalaxyRenderer::GetPipelines()
{
    return vector({ particles_render_pipeline_, tree_draw_pipeline_ });
}

void GalaxyRenderer::Render()
{
    if (render_params_.colors_inverted)
    {
        GAL_OpenGL::SetClearColor(float4(1.0f, 1.0f, 1.0f, 1.0f));
        GAL_OpenGL::ClearColor();
    }

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

    test_pipeline_->Dispatch(int3(1, 1, 1));
    GAL_OpenGL::MemoryBarriers(GAL::MemoryBarrierType::SHADER_STORAGE);

    // Draw particles
    if (render_params_.render_particles && render_params_.render_as_points)
    {
        if (!render_params_.colors_inverted)
        {
            GAL_OpenGL::EnableBlendOneOne();
        }

        GAL_OpenGL::SetPointSize(render_params_.particle_size_scale);

        Device::ShadeSingleColorRootConstants root_constants = {};
        float4 particle_color = render_params_.colors_inverted ? Math::kBlackColor : Math::kWhiteColor;
        root_constants.color = particle_color * render_params_.brightness;
        root_constants.transform = Matrix();
        particles_render_pipeline_->SetRootConstants(&root_constants);

        particles_render_pipeline_->BeginGraphics();
        particles_render_pipeline_->Draw(0, universe_.GetParticlesCount(), GAL::PrimitiveType::Points);
        particles_render_pipeline_->EndGraphics();

        if (!render_params_.colors_inverted)
        {
            GAL_OpenGL::DisableBlend();
        }
    }

    // Draw tree
    if (sim_context_.IsBarnesHut() && render_params_.render_tree)
    {
        size_t nodes_count = universe_.node_positions_.size();
        tree_draw_pipeline_->BeginGraphics();
        tree_draw_pipeline_->Draw(0, nodes_count, GAL::PrimitiveType::Points);
        tree_draw_pipeline_->EndGraphics();
    }
}

void GalaxyRenderer::CreateParticlesBuffer()
{
    size_t count = universe_.GetParticlesCount();

    RenderDevice& device = engine->GetRenderer().GetRendererCore().GetRenderDevice();

    particles_positions_buffer_ = device.CreateBuffer("ParticlesPositionsBuffer", GAL::BufferType::kStorage, count * sizeof(float4), GAL::BufferUsage::DynamicDraw);
}

void GalaxyRenderer::CreateNodesBuffers(size_t nodes_count)
{
    RenderDevice& device = engine->GetRenderer().GetRendererCore().GetRenderDevice();

    nodes_positions_buffer_ = device.CreateBuffer("NodePositionsBuffer", GAL::BufferType::kStorage, nodes_count * sizeof(float4), GAL::BufferUsage::DynamicDraw);
    nodes_sizes_buffer_ = device.CreateBuffer("NodeSizesBuffer", GAL::BufferType::kStorage, nodes_count * sizeof(float), GAL::BufferUsage::DynamicDraw);
}

void GalaxyRenderer::UpdateParticlesBuffer()
{
    size_t count = universe_.GetParticlesCount();
    particles_positions_buffer_->Write(0, count * sizeof(float4), universe_.positions_.data());
}

void GalaxyRenderer::UpdateNodesBuffers()
{
    size_t nodes_count = universe_.node_positions_.size();

    if (nodes_count == 0)
    {
        return;
    }

    size_t required_size = nodes_count * sizeof(float4);
    if (!nodes_positions_buffer_ || nodes_positions_buffer_->GetSize() < required_size)
    {
        CreateNodesBuffers(nodes_count);
        BindNodesBuffers();
    }

    nodes_positions_buffer_->Write(0, nodes_count * sizeof(float4), universe_.node_positions_.data());
    nodes_sizes_buffer_->Write(0, nodes_count * sizeof(float), universe_.node_sizes_.data());
}

void GalaxyRenderer::BindNodesBuffers()
{
    tree_draw_pipeline_->SetBuffer(nodes_positions_buffer_, "NodePositions");
    tree_draw_pipeline_->SetBuffer(nodes_sizes_buffer_, "NodeRadius");
}
