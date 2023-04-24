#include "BruteforceCPUSolver.h"
#include "BarnesHutGPUSolver.h"

#include <Private/RenderDevice.h>
#include <Math/Math.h>
#include <Data/DeviceData.h>
#include <OpenGL/GraphicsOpenGL.h>
#include <Debugging/Profiler.h>

constexpr uint32 TREE_CHILDREN_COUNT = 8;

BarnesHutGPUSolver::BarnesHutGPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : GPUSolverBase(universe, context, render_params)
{
}

BarnesHutGPUSolver::~BarnesHutGPUSolver()
{
}

void BarnesHutGPUSolver::ComputeAcceleration()
{
    PROFILER_BLOCK_GPU("Barnes-Hut");

#if 0
    {
        PROFILER_BLOCK_GPU("Bounding Box");

        bounding_box_pipeline_->Dispatch(int3(1, 1, 1));
        GAL_OpenGL::MemoryBarriers(GAL::MemoryBarrierType::SHADER_STORAGE);
        GAL_OpenGL::Finish();
    }
#endif // 0


    {
        PROFILER_BLOCK_GPU("Build Tree");

        //build_pipeline_->Dispatch(CalcNumGroups(universe_.GetParticlesCount(), 8));
        //build_pipeline_->Dispatch(CalcNumGroups(universe_.GetParticlesCount(), Device::kGroupSize1D));
        build_pipeline_->Dispatch(int3(1, 1, 1));
        GAL_OpenGL::MemoryBarriers(GAL::MemoryBarrierType::SHADER_STORAGE);
    }
}

void BarnesHutGPUSolver::CreateBuffers()
{
    GPUSolverBase::CreateBuffers();

    children_ = CreateBuffer("Children", GetNodesMaxCount() * TREE_CHILDREN_COUNT * sizeof(int32), GAL::BufferType::kStorage);
    nodes_index_ = CreateBuffer("NodesIndex", sizeof(int32), GAL::BufferType::kStorage);
    radius_ = CreateBuffer("Radius", sizeof(float), GAL::BufferType::kStorage);
    debug_ = CreateBuffer("Debug", 20 * sizeof(int), GAL::BufferType::kStorage);
}

void BarnesHutGPUSolver::CreatePipelines()
{
    GPUSolverBase::CreatePipelines();

    bounding_box_pipeline_ = GetRenderDevice().CreateComputePipeline("BoundingBox.comp");
    build_pipeline_ = GetRenderDevice().CreateComputePipeline("BuildTree.comp");
    //summarize_pipeline_ = GetRenderDevice().CreateComputePipeline("Summarize.comp");
    //sort_pipeline_ = GetRenderDevice().CreateComputePipeline("Sort.comp");
    //compute_acceleration_pipeline_ = GetRenderDevice().CreateComputePipeline("TreeComputeAcceleration.comp");

    BindLayout(bounding_box_pipeline_);
    BindLayout(build_pipeline_);
    //BindLayout(summarize_pipeline_);
    //BindLayout(sort_pipeline_);
    //BindLayout(compute_acceleration_pipeline_);    
}

void BarnesHutGPUSolver::BindLayout(GAL::ComputePipelinePtr pipeline)
{
    GPUSolverBase::BindLayout(pipeline);

    pipeline->SetBuffer(children_, "Children");
    pipeline->SetBuffer(nodes_index_, "NodesIndex");
    pipeline->SetBuffer(radius_, "RootRadius");
    pipeline->SetBuffer(debug_, "DebugBuffer");
}
