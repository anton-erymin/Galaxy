#pragma once

class Universe;
class ISolver;
class IRendererPlugin;
class MainWindow;

class GalaxySimulator
{
public:
    GalaxySimulator();
    ~GalaxySimulator();

private:
    void CreateUniverse();
    void CreateSolver();
    void CreateRenderer();

private:
    //SimulationParameters simulation_params_;
    //Timings timings_;

    float cSecondsPerTimeUnit = 0;
    float cMillionYearsPerTimeUnit = 0;


    int32_t totalParticlesCount = 0;

    uint32_t frameCounter = 0;

    bool started = false;
    bool saveToFiles = false;

    unique_ptr<Universe> universe_;
    unique_ptr<ISolver> solver_;
    unique_ptr<IRendererPlugin> renderer_;
    unique_ptr<MainWindow> main_window_;

    //RenderParameters renderParams;
    //SimulationParameters simulationParams;

#if 0
    
    GAL::ComputePipelinePtr particles_update_pipeline_;
    GAL::ComputePipelinePtr particles_clear_forces_pipeline_;
    GAL::ComputePipelinePtr particles_barnes_hut_pipeline_;
    GAL::ComputePipelinePtr particles_solve_pipeline_;
#endif // 0

    //
    //Entity nodes_buffer_ = kInvalidEntity;
    //GAL::BufferPtr nodes_counter;
    bool write_flag_ = false;

    bool is_simulated_ = false;

    //Entity controller_ = kInvalidEntity;
};
