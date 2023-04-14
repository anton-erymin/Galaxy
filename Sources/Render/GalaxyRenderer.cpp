#include "GalaxyRenderer.h"
#include "Universe.h"
#include "Particle.h"

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

GalaxyRenderer::GalaxyRenderer(Universe& universe, condition_variable* solver_cv)
    : universe_(universe)
    , solver_cv_(solver_cv)
{
    // Add additional shaders directory
    string shaders_path = Paths::BaseDir() + "/../../../Sources/Shaders";
    engine->GetRenderer().GetRendererCore().GetRenderDevice().GetShaderManager().
        AddShadersPath(shaders_path);

    CreateParticlesBuffer();
    CreateNodesBuffers(1);

    GAL_OpenGL::SetPointSize(10.0f);
}

void GalaxyRenderer::CreatePipelines(RenderDevice& render_device)
{
    particles_render_pipeline_ = render_device.CreateGraphicsPipeline("DrawParticles.geom",  "ShadeSingleColor.vert", "ShadeSingleColor.frag");
    tree_draw_pipeline_ = render_device.CreateGraphicsPipeline("DrawBarnesHut.geom", "ShadeSingleColor.vert", "ShadeSingleColor.frag");
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

        Device::ShadeSingleColorRootConstants root_constants = {};
        root_constants.color = float4(1.0f);
        root_constants.transform = Matrix();
        particles_render_pipeline_->SetRootConstants(&root_constants);
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

    GAL::BufferPtr particles_positions_buffer = buffer_system->GetDeviceBuffer(particles_positions_buffer_);
    
    particles_render_pipeline_->SetBuffer(particles_positions_buffer, "ParticlesPositions");

    BindNodesBuffers();
}

vector<GAL::GraphicsPipelinePtr> GalaxyRenderer::GetPipelines()
{
    return vector({ particles_render_pipeline_, tree_draw_pipeline_ });
}

void GalaxyRenderer::Render()
{
    if (buffer_update_requested_flag_)
    {
        UpdateParticlesBuffer();
        UpdateNodesBuffers();

        buffer_update_requested_flag_ = false;

        // After update positions buffer signal to solver
        // so that it will be able to update postions during integration phase
        if (solver_cv_)
        {
            solver_cv_->notify_one();
        }
    }

    // Draw particles
    {
        GAL_OpenGL::EnableBlendOneOne();

        particles_render_pipeline_->BeginGraphics();
        particles_render_pipeline_->Draw(0, universe_.GetParticlesCount(), GAL::PrimitiveType::Points);
        particles_render_pipeline_->EndGraphics();

        GAL_OpenGL::DisableBlend();
    }

    // Draw tree
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

    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();

    buffer_system->CreateDeviceBuffer("ParticlesPositionsBuffer", particles_positions_buffer_,
        count * sizeof(float4), GAL::BufferType::kStorage, GAL::BufferUsage::DynamicDraw);
}

void GalaxyRenderer::CreateNodesBuffers(size_t nodes_count)
{
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();

    buffer_system->CreateDeviceBuffer("NodePositionsBuffer", nodes_positions_,
        nodes_count * sizeof(float4), GAL::BufferType::kStorage, GAL::BufferUsage::DynamicDraw);

    buffer_system->CreateDeviceBuffer("NodeSizesBuffer", nodes_sizes_,
        nodes_count * sizeof(float), GAL::BufferType::kStorage, GAL::BufferUsage::DynamicDraw);
}

void GalaxyRenderer::UpdateParticlesBuffer()
{
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();

    size_t count = universe_.GetParticlesCount();

    GAL::BufferPtr buffer = buffer_system->GetDeviceBuffer(particles_positions_buffer_);
    buffer->Write(0, count * sizeof(float4), universe_.positions_.data());
}

void GalaxyRenderer::UpdateNodesBuffers()
{
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();

    size_t nodes_count = universe_.node_positions_.size();

    GAL::BufferPtr node_positions_buffer = buffer_system->GetDeviceBuffer(nodes_positions_);
    GAL::BufferPtr node_sizes_buffer = buffer_system->GetDeviceBuffer(nodes_sizes_);

    size_t required_size = nodes_count * sizeof(float4);
    if (node_positions_buffer->GetSize() < required_size)
    {
        CreateNodesBuffers(nodes_count);

        node_positions_buffer = buffer_system->GetDeviceBuffer(nodes_positions_);
        node_sizes_buffer = buffer_system->GetDeviceBuffer(nodes_sizes_);

        BindNodesBuffers();
    }

    node_positions_buffer->Write(0, nodes_count * sizeof(float4), universe_.node_positions_.data());
    node_sizes_buffer->Write(0, nodes_count * sizeof(float), universe_.node_sizes_.data());
}

void GalaxyRenderer::BindNodesBuffers()
{
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();

    GAL::BufferPtr node_positions_buffer = buffer_system->GetDeviceBuffer(nodes_positions_);
    GAL::BufferPtr node_sizes_buffer = buffer_system->GetDeviceBuffer(nodes_sizes_);

    tree_draw_pipeline_->SetBuffer(node_positions_buffer, "NodePositions");
    tree_draw_pipeline_->SetBuffer(node_sizes_buffer, "NodeSizes");
}
