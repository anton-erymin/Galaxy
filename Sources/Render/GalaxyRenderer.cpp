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
#include <Interfaces/IOnScreenDebugStringManager.h>

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

    GAL_OpenGL::SetPointSize(3.0f);
}

void GalaxyRenderer::CreatePipelines(RenderDevice& render_device)
{
    particles_render_pipeline_ = render_device.CreateGraphicsPipeline(
        "DrawParticles.geom", 
        "ShadeSingleColor.vert", 
        "ShadeSingleColor.frag");
#if 0
    {
        *g_tree_draw_pipeline = GetRenderer().GetRenderDevice().CreateGraphicsPipeline(
            "DrawBarnesHut.geom", "ShadeSingleColor.vert", "ShadeSingleColor.frag",
            state, g_defines);
    }
#endif // 0
}

void GalaxyRenderer::CreateSizeDependentResources(RenderDevice& render_device, const int2& output_size)
{
}

void GalaxyRenderer::DestroyResources()
{
}

void GalaxyRenderer::UpdatePipelines(GAL::ImagePtr& output_image)
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

void GalaxyRenderer::BindSceneDataBuffers()
{
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().
        DeviceBufferSystem();

    GAL::BufferPtr positions_buffer = buffer_system->GetDeviceBuffer(particles_positions_);

    particles_render_pipeline_->SetBuffer(positions_buffer, "ParticlesPositions");
}

vector<GAL::GraphicsPipelinePtr> GalaxyRenderer::GetPipelines()
{
    return vector({ particles_render_pipeline_ });
}

void GalaxyRenderer::Render()
{
    if (buffer_update_requested_flag_)
    {
        UpdateParticlesBuffer();
        buffer_update_requested_flag_ = false;

        // After update positions buffer signal to solver
        // so that it will be able to update postions during integration phase
        if (solver_cv_)
        {
            solver_cv_->notify_one();
        }
    }

    GAL_OpenGL::EnableBlendOneOne();

    particles_render_pipeline_->BeginGraphics();
    particles_render_pipeline_->Draw(0, universe_.GetParticlesCount(), GAL::PrimitiveType::Points);
    particles_render_pipeline_->EndGraphics();

    GAL_OpenGL::DisableBlend();
}

void GalaxyRenderer::CreateParticlesBuffer()
{
    size_t count = universe_.GetParticlesCount();
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().
        DeviceBufferSystem();

    buffer_system->CreateDeviceBuffer("ParticlesPositionsBuffer", particles_positions_,
        count * sizeof(float4), GAL::BufferType::kStorage, GAL::BufferUsage::DynamicDraw);
}

void GalaxyRenderer::UpdateParticlesBuffer()
{
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().
        DeviceBufferSystem();

    size_t count = universe_.GetParticlesCount();

    //vector<Device::Particle> device_particles(count);

#if 0
    for (size_t i = 0u; i < count; ++i)
    {
        device_particles[i].position = universe_.positions_[i];
        device_particles[i].position.w = universe_.masses_[i];
        device_particles[i].velocity.w = universe_.inverse_masses_[i];
        device_particles[i].velocity = universe_.velocities_[i];
        //device_particles[i].acceleration = universe_.accelerations_[i];
        device_particles[i].force = universe_.forces_[i];
    }
#endif // 0

    GAL::BufferPtr buffer = buffer_system->GetDeviceBuffer(particles_positions_);
    buffer->Write(0, count * sizeof(float4), universe_.positions_.data());
}
