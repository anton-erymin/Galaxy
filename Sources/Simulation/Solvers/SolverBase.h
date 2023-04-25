#pragma once

#include "GalaxySimulator/GalaxyTypes.h"

#include <Misc/FPSCounter.h>

class Universe;
struct SimulationContext;
struct RenderParameters;
class BodyTracker;

class SolverBase : public IUpdatable
{
public:
    SolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
        : universe_(universe)
        , context_(context)
        , render_params_(render_params)
    {
    }

    virtual ~SolverBase() = default;

    virtual void Initialize();
    virtual void Update() override;

    virtual void Start() { }
    
protected:
    void Solve();

private:

//protected:

    // These must not be pure virtual because it may be called from the separate thread (for CPU solvers)
	// and destructor of subclass can have been called by the moment
	virtual void ComputeAcceleration() { }
    virtual void IntegrateLeapFrogKickDrift() { }
    virtual void IntegrateLeapFrogKick() { }

    void Dump(const char* prefix);

protected:
    Universe& universe_;
    SimulationContext& context_;
    const RenderParameters& render_params_;

    FPSCounter fps_counter_;
    unique_ptr<BodyTracker> tracker_;
};

class BodyTracker
{
public:
    BodyTracker(Universe& universe)
        : universe_(universe)
    {
        StorePositions();
    }

    void AddTracks(initializer_list<size_t>& list)
    {
        track_indices_.insert(track_indices_.end(), list.begin(), list.end());
    }

    void Track();
    void StorePositions();

private:
    const Universe& universe_;
    vector<size_t> track_indices_;
    vector<float3> prev_pos_;
};
