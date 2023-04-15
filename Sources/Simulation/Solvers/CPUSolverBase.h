#pragma once

#include "ISolver.h"

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
	// This needs to be declared before thread
	vector<mutex> force_mutexes_;
	unique_ptr<Thread> thread_;
	volatile atomic_bool active_flag_;
};
