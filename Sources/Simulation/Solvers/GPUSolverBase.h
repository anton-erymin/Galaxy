#pragma once

#include "SolverBase.h"
#include "Universe.h"

#include <GALFwd.h>

class RenderDevice;

class GPUSolverBase : public SolverBase
{
public:
	GPUSolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
	~GPUSolverBase();

	virtual void Initialize() override;
	virtual void Start() override { }

	GAL::BufferPtr GetPositionBuffer() const { return position_; }

private:
	virtual void IntegrateLeapFrogKickDrift() override;
	virtual void IntegrateLeapFrogKick() override;

protected:
	virtual void CreateBuffers();
	virtual void CreatePipelines();
	virtual void BindLayout(GAL::ComputePipelinePtr pipeline);

	void FillBuffers();
	void UpdateParamsBuffer();

	GAL::BufferPtr CreateBuffer(const char* name, size_t size, GAL::BufferType type);

	virtual size_t GetPositionsBufferCount() const { return universe_.GetParticlesCount(); }
	virtual size_t GetNodesMaxCount() const { return 0; }

	RenderDevice& GetRenderDevice();

private:
	GAL::ComputePipelinePtr integrate_leap_frog_kick_drift_pipeline_;
	GAL::ComputePipelinePtr integrate_leap_frog_kick_pipeline_;

	// Position and mass buffers contain both particles and nodes (nodes only if BarnesHut solver used)
	GAL::BufferPtr position_;
	GAL::BufferPtr mass_;
	GAL::BufferPtr velocity_;
	GAL::BufferPtr acceleration_;
	GAL::BufferPtr simulation_parameters_;
};
