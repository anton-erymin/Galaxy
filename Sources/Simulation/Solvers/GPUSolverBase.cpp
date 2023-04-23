#include "GPUSolverBase.h"

#include <Engine.h>
#include <Renderer.h>
#include <RendererCore.h>
#include <Private/RenderDevice.h>
#include <GAL.h>
#include <OpenGL/GraphicsOpenGL.h>
#include <Data/DeviceData.h>
#include <Debugging/Profiler.h>

struct SimulationParameters
{
    uint body_count;
    uint nodes_max_count;
    uint total_count; // Bodies + Nodes available to allocate
    float timestep;
    float gravity_softening_length;
    float barnes_hut_opening_angle;
    uint pad0;
    uint pad1;
};

int3 CalcNumGroups(int size, uint group_size)
{
    return int3((size + group_size - 1) / group_size, 1, 1);
}

int3 CalcNumGroups(int2 size, uint group_size)
{
    return int3((size.x + group_size - 1) / group_size, (size.y + group_size - 1) / group_size, 1);
}

int3 CalcNumGroups(int3 size, uint group_size)
{
    return int3((size.x + group_size - 1) / group_size, (size.y + group_size - 1) / group_size,
        (size.z + group_size - 1) / group_size);
}

GPUSolverBase::GPUSolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : SolverBase(universe, context, render_params)
{
}

GPUSolverBase::~GPUSolverBase()
{
}

void GPUSolverBase::Initialize()
{
    CreateBuffers();
    FillBuffers();
    UpdateParamsBuffer();
    CreatePipelines();

    SolverBase::Initialize();
}

void GPUSolverBase::IntegrateLeapFrogKickDrift()
{
    PROFILER_BLOCK_GPU("IntegrateLeapFrogKickDrift");

    integrate_leap_frog_kick_drift_pipeline_->Dispatch(CalcNumGroups(universe_.GetParticlesCount(), Device::kGroupSize1D));
    GAL_OpenGL::MemoryBarriers(GAL::MemoryBarrierType::SHADER_STORAGE);
}

void GPUSolverBase::IntegrateLeapFrogKick()
{
    PROFILER_BLOCK_GPU("IntegrateLeapFrogKick");

    integrate_leap_frog_kick_pipeline_->Dispatch(CalcNumGroups(universe_.GetParticlesCount(), Device::kGroupSize1D));
    GAL_OpenGL::MemoryBarriers(GAL::MemoryBarrierType::SHADER_STORAGE);
}

void GPUSolverBase::CreateBuffers()
{
    position_ = CreateBuffer("Position", GetPositionsBufferCount() * sizeof(float4), GAL::BufferType::kStorage);
    mass_ = CreateBuffer("Mass", GetPositionsBufferCount() * sizeof(float), GAL::BufferType::kStorage);
    velocity_ = CreateBuffer("Velocity", universe_.GetParticlesCount() * sizeof(float4), GAL::BufferType::kStorage);
    acceleration_ = CreateBuffer("Acceleration", universe_.GetParticlesCount() * sizeof(float4), GAL::BufferType::kStorage);
    simulation_parameters_ = CreateBuffer("SimulationParameters", sizeof(SimulationParameters), GAL::BufferType::kUniform);
}

void GPUSolverBase::CreatePipelines()
{
    integrate_leap_frog_kick_drift_pipeline_ = GetRenderDevice().CreateComputePipeline("IntegrateLeapFrogKickDrift.comp");
    integrate_leap_frog_kick_pipeline_= GetRenderDevice().CreateComputePipeline("IntegrateLeapFrogKick.comp");

    BindLayout(integrate_leap_frog_kick_drift_pipeline_);
    BindLayout(integrate_leap_frog_kick_pipeline_);
}

void GPUSolverBase::FillBuffers()
{
    size_t body_count = universe_.GetParticlesCount();

    position_->Write(0, body_count * sizeof(float4), universe_.positions_.data());
    mass_->Write(0, body_count * sizeof(float), universe_.masses_.data());
    velocity_->Write(0, body_count * sizeof(float4), universe_.velocities_.data());
}

void GPUSolverBase::UpdateParamsBuffer()
{
    SimulationParameters params = {};
    params.body_count = universe_.GetParticlesCount();
    params.nodes_max_count = 0;
    params.total_count = universe_.GetParticlesCount();
    params.timestep = context_.timestep;
    params.gravity_softening_length = context_.gravity_softening_length;
    params.barnes_hut_opening_angle = context_.barnes_hut_opening_angle;

    simulation_parameters_->Write(0, sizeof(SimulationParameters), &params);
}

void GPUSolverBase::BindLayout(GAL::ComputePipelinePtr pipeline)
{
    pipeline->SetBuffer(position_, "Position");
    pipeline->SetBuffer(mass_, "Mass");
    pipeline->SetBuffer(velocity_, "Velocity");
    pipeline->SetBuffer(acceleration_, "Acceleration");
    pipeline->SetBuffer(simulation_parameters_, "SimulationParameters");
}

GAL::BufferPtr GPUSolverBase::CreateBuffer(const char* name, size_t size, GAL::BufferType type)
{
    return GetRenderDevice().CreateBuffer(name, type, size, GAL::BufferUsage::DynamicDraw);
}

RenderDevice& GPUSolverBase::GetRenderDevice()
{
    return engine->GetRenderer().GetRendererCore().GetRenderDevice();
}
