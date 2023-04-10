#include "GalaxyRenderer.h"
#include "Universe.h"
#include "Particle.h"

#include <Engine.h>
#include <Renderer.h>
#include <RendererCore.h>
#include <Private/RenderDevice.h>
#include <GAL.h>
#include "OpenGL/GraphicsOpenGL.h"
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

GalaxyRenderer::GalaxyRenderer(Universe& universe)
    : universe_(universe)
{
    string shaders_path = Paths::BaseDir() + "/../../../Sources/Shaders";
    engine->GetRenderer().GetRendererCore().GetRenderDevice().GetShaderManager().AddShadersPath(shaders_path);

    CreateParticlesBuffer();
    FillParticlesBuffer();

    GAL_OpenGL::SetPointSize(5.0f);
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
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();

    particles_render_pipeline_->SetBuffer(buffer_system->GetDeviceBuffer(particles_buffer_), "ParticlesData");
}

vector<GAL::GraphicsPipelinePtr> GalaxyRenderer::GetPipelines()
{
    return vector({ particles_render_pipeline_ });
}

void GalaxyRenderer::Render()
{
    FillParticlesBuffer();

    GAL_OpenGL::EnableBlendOneOne();

    particles_render_pipeline_->BeginGraphics();
    particles_render_pipeline_->Draw(0, universe_.GetParticlesCount(), GAL::PrimitiveType::Points);
    particles_render_pipeline_->EndGraphics();

    GAL_OpenGL::DisableBlend();
}

void GalaxyRenderer::CreateParticlesBuffer()
{
    size_t count = universe_.GetParticlesCount();
    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();

    buffer_system->CreateDeviceBuffer("ParticlesBuffer", particles_buffer_,
        count * sizeof(Device::Particle), GAL::BufferType::kStorage, GAL::BufferUsage::DynamicDraw);
}

void GalaxyRenderer::FillParticlesBuffer()
{
    size_t count = universe_.GetParticlesCount();

    vector<Device::Particle> device_particles(count);

    for (size_t i = 0u; i < count; ++i)
    {
        device_particles[i].position = universe_.positions_[i];
        device_particles[i].position.w = universe_.masses_[i];
        device_particles[i].velocity.w = universe_.inverse_masses_[i];
        device_particles[i].velocity = universe_.velocities_[i];
        //device_particles[i].acceleration = universe_.accelerations_[i];
        device_particles[i].force = universe_.forces_[i];
    }

    Systems::IDeviceBufferSystem* buffer_system = engine->GetRenderer().GetRendererCore().DeviceBufferSystem();
    GAL::BufferPtr buffer = buffer_system->GetDeviceBuffer(particles_buffer_);
    buffer->Write(0, count * sizeof(Device::Particle), device_particles.data());
}
