#pragma once

#include "SolverBase.h"
#include "Thread/ThreadPool.h"

class Thread;

class CPUSolverBase : public SolverBase
{
public:
	CPUSolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
	~CPUSolverBase();

	virtual void Start() override;

protected:
	void Stop();

private:	
	void SolverRun();

	virtual void IntegrateLeapFrogKickDrift() override;
	virtual void IntegrateLeapFrogKick() override;

	void LeapFrogKickDriftIntegrationKernel(THREAD_POOL_KERNEL_ARGS);
	void LeapFrogKickIntegrationKernel(THREAD_POOL_KERNEL_ARGS);

protected:
	// This needs to be declared before thread
	Array<UniquePtr<Mutex>> force_mutexes_;
	UniquePtr<Thread> thread_;
	volatile AtomicBool active_flag_;
};
