#pragma once

#include "ISolver.h"
#include "Thread/ThreadPool.h"

class Thread;

class CPUSolverBase : public ISolver
{
public:
	CPUSolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
	~CPUSolverBase();

	virtual void Start() override;

protected:
	void Stop();

private:
	void SolverRun();
	void Solve(float time) override;
	void IntegrationKernel(THREAD_POOL_KERNEL_ARGS);
	void LeapFrogKickDriftIntegrationKernel(THREAD_POOL_KERNEL_ARGS);
	void LeapFrogKickIntegrationKernel(THREAD_POOL_KERNEL_ARGS);

	// This must not be pure virtual because it is called from the separate thread
	// and destructor of subclass can have been called by the moment
	virtual void ComputeAcceleration() { }

	void Dump(const char* prefix);

protected:
	// This needs to be declared before thread
	vector<mutex> force_mutexes_;
	unique_ptr<Thread> thread_;
	volatile atomic_bool active_flag_;
};
