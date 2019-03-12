#pragma once

#include <memory>
#include <mutex>

#include "Solver.h"
#include "OpenCL.h"

class BarnesHutGPUSolver : public Solver
{
public:
    BarnesHutGPUSolver(Universe& universe);

    void Solve(float time) override;
    void SolveForces() override;

    const BarnesHutTree& GetBarnesHutTree() const { return *barnesHutTree; }
    std::mutex& GetTreeMutex() { return mu; }

    void Inititalize(float time) override;
    void Prepare() override;

private:
    void BuildTree();

    std::unique_ptr<BarnesHutTree> barnesHutTree;
    std::mutex mu;

    cl::OpenCL cl;

    cl::ProgramPtr programIntegrate;
    cl::KernelPtr kernelIntegrate;

    cl::BufferPtr position;
    cl::BufferPtr velocity;
    cl::BufferPtr acceleration;
    cl::BufferPtr force;
    cl::BufferPtr inverseMass;
};