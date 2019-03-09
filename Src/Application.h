#pragma once

#include <cassert>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <thread>

#include "Galaxy.h"
#include "Orbit.h"
#include "Math.h"
#include "UIOverlay.h"
#include "OpenCL.h"

class ImageLoader;
class Universe;
class BruteforceSolver;
class BarnesHutSolver;
class Solver;

class Application
{
public:
    Application();
    ~Application();

    int Run(int argc, char** argv);

    void OnDraw();
    void OnResize(int width, int height);
    void OnIdle();
    void OnKeyboard(unsigned char key, int x, int y);
    void OnKeyboardUp(unsigned char key, int x, int y); 
    void OnMousePressed(int button, int state, int x, int y);
    void OnMouseMove(int x, int y);
    void OnMousePassiveMove(int x, int y);
    void OnMouseWheel(int button, int dir, int x, int y);
    void OnKeyboardSpecialFunc(unsigned char key, int x, int y); 

    ImageLoader& GetImageLoader() { return *imageLoader; }
    Universe& GetUniverse() { return *universe; }
    
    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

    static Application& GetInstance() { assert(instance);  return *instance; }

    struct SimulationParameters
    {
        bool darkMatter = false;
    };

    const SimulationParameters& GetSimulationParamaters() const { return simulationParams; }

private:
    void InitializeUniverse();

    uint32_t width = 0;
    uint32_t height = 0;

    float cSecondsPerTimeUnit = 0;
    float cMillionYearsPerTimeUnit = 0;

    float deltaTime = 0.0f;
    float deltaTimeYears = 0.0f;
    float simulationTime = 0.0f;
    float simulationTimeMillionYears = 0.0f;

    float lastFps = 0.0f;
    int32_t numSteps = 0;
    int32_t totalParticlesCount = 0;

    uint32_t frameCounter = 0;

    bool started = false;
    bool saveToFiles = false;

    UIOverlay ui;

    std::unique_ptr<ImageLoader> imageLoader;
    std::unique_ptr<Universe> universe;
    std::unique_ptr<BruteforceSolver> solverBruteforce;
    std::unique_ptr<BarnesHutSolver> solverBarneshut;

    Solver* solver = nullptr;

    std::thread solverThread;

    struct InputState
    {
        uint32_t buttons = 0;
        float2 prevPos;
        bool brightnessUp = false;
        bool brightnessDown = false;
    };

    InputState inputState;
    std::unordered_map<char, bool*> inputMappings;

    Orbit orbit;

    struct RenderParameters
    {
        bool renderTree = false;
        bool renderPoints = false;
        bool plotFunctions = false;
        float brightness = 1.0f;
        float particlesSizeScale = 1.0f;

    } renderParams;

    SimulationParameters simulationParams;

    GalaxyParameters model;

    cl::OpenCL cl;

    static Application* instance;
};