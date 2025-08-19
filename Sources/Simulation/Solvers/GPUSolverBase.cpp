#include "GPUSolverBase.h"

#include <Engine.h>
#include <Renderer.h>
#include <RendererCore.h>
#include <Private/RenderDevice.h>
#include <GAL.h>
#include <OpenGL/OpenGLGraphics.h>
#include <Data/DeviceData.h>
#include <Debugging/Profiler.h>
#include <Math/Math.h>

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
    integrate_leap_frog_kick_drift_pipeline_->MemoryBarriers(GAL::MemoryBarrierType::SHADER_STORAGE);
}

void GPUSolverBase::IntegrateLeapFrogKick()
{
    PROFILER_BLOCK_GPU("IntegrateLeapFrogKick");

    integrate_leap_frog_kick_pipeline_->Dispatch(CalcNumGroups(universe_.GetParticlesCount(), Device::kGroupSize1D));
    integrate_leap_frog_kick_drift_pipeline_->MemoryBarriers(GAL::MemoryBarrierType::SHADER_STORAGE);
}

void GPUSolverBase::CreateBuffers()
{
    position_ = CreateBuffer("Position", GetPositionsBufferCount() * sizeof(Float4), GAL::BufferType::kStorage);
    mass_ = CreateBuffer("Mass", GetPositionsBufferCount() * sizeof(float), GAL::BufferType::kStorage);
    velocity_ = CreateBuffer("Velocity", universe_.GetParticlesCount() * sizeof(Float4), GAL::BufferType::kStorage);
    acceleration_ = CreateBuffer("Acceleration", universe_.GetParticlesCount() * sizeof(Float4), GAL::BufferType::kStorage);
    simulation_parameters_ = CreateBuffer("SimulationParameters", sizeof(SimulationParameters), GAL::BufferType::kUniform);
}

void GPUSolverBase::CreatePipelines()
{
    integrate_leap_frog_kick_drift_pipeline_ = GetRenderDevice().CreateComputePipeline(SID("IntegrateLeapFrogKickDrift.comp"));
    integrate_leap_frog_kick_pipeline_= GetRenderDevice().CreateComputePipeline(SID("IntegrateLeapFrogKick.comp"));

    BindLayout(integrate_leap_frog_kick_drift_pipeline_);
    BindLayout(integrate_leap_frog_kick_pipeline_);
}

void GPUSolverBase::FillBuffers()
{
    size_t body_count = universe_.GetParticlesCount();

    position_->Write(0, body_count * sizeof(Float4), universe_.positions_.Data());
    mass_->Write(0, body_count * sizeof(float), universe_.masses_.Data());
    velocity_->Write(0, body_count * sizeof(Float4), universe_.velocities_.Data());
}

void GPUSolverBase::UpdateParamsBuffer()
{
    SimulationParameters params = {};
    params.body_count = universe_.GetParticlesCount();
    params.nodes_max_count = GetNodesMaxCount();
    params.total_count = params.body_count + params.nodes_max_count;
    params.timestep = context_.timestep;
    params.gravity_softening_length = context_.gravity_softening_length;
    params.barnes_hut_opening_angle = context_.barnes_hut_opening_angle;

    simulation_parameters_->Write(0, sizeof(SimulationParameters), &params);
}

void GPUSolverBase::BindLayout(GAL::ComputePipelinePtr pipeline)
{
    pipeline->SetBuffer(position_, SID("Position"));
    pipeline->SetBuffer(mass_, SID("Mass"));
    pipeline->SetBuffer(velocity_, SID("Velocity"));
    pipeline->SetBuffer(acceleration_, SID("Acceleration"));
    pipeline->SetBuffer(simulation_parameters_, SID("SimulationParameters"));
}

GAL::BufferPtr GPUSolverBase::CreateBuffer(const char* name, size_t size, GAL::BufferType type)
{
    return GetRenderDevice().CreateBuffer(SID_CSTR(name), type, size, GAL::BufferUsage::DynamicDraw);
}

RenderDevice& GPUSolverBase::GetRenderDevice()
{
    return engine->GetRenderer().GetRendererCore().GetRenderDevice();
}
