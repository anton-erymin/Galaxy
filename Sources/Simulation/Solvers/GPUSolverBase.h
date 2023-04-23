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
	void CreateBuffers();
	void CreatePipelines();
	void FillBuffers();
	void UpdateParamsBuffer();
	void BindLayout(GAL::ComputePipelinePtr pipeline);

	GAL::BufferPtr CreateBuffer(const char* name, size_t size, GAL::BufferType type);

	virtual size_t GetPositionsBufferCount() const { return universe_.GetParticlesCount(); }

	RenderDevice& GetRenderDevice();

private:
	// Position and mass buffers contain both particles and nodes (nodes only if BarnesHut solver used)
	GAL::BufferPtr position_;
	GAL::BufferPtr mass_;
	GAL::BufferPtr velocity_;
	GAL::BufferPtr acceleration_;
	GAL::BufferPtr simulation_parameters_;

	GAL::ComputePipelinePtr integrate_leap_frog_kick_drift_pipeline_;
	GAL::ComputePipelinePtr integrate_leap_frog_kick_pipeline_;
};
