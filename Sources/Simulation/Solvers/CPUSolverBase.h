#pragma once

#include "ISolver.h"
#include "Thread/ThreadPool.h"

class Thread;

class CPUSolverBase : public ISolver
{
public:
	CPUSolverBase(Universe& universe, SimulationContext& context);
	~CPUSolverBase();

	virtual void Start() override;

private:
	void SolverRun();

protected:
	void IntegrationKernel(THREAD_POOL_KERNEL_ARGS);

protected:
	// This needs to be declared before thread
	vector<mutex> force_mutexes_;
	unique_ptr<Thread> thread_;
	volatile atomic_bool active_flag_;
};
