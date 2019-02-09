#pragma once

#include <cassert>
#include <memory>

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

    ImageLoader& GetImageLoader() { return *imageLoader; }
    Universe& GetUniverse() { return *universe; }

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

    static Application& GetInstance() { assert(instance);  return *instance; }

private:
    uint32_t width;
    uint32_t height;

    std::unique_ptr<ImageLoader> imageLoader;
    std::unique_ptr<Universe> universe;
    std::unique_ptr<BruteforceSolver> solver_bruteforce;
    std::unique_ptr<BarnesHutSolver> solver_barneshut;

    Solver* solver = nullptr;

    float cameraX = 0.0f;
    float cameraY = -20.0f;
    float cameraZ = 40.0f;

    float cSecsInTimeUnit = 0;
    float cMillionYearsInTimeUnit = 0;

    float deltaTime = 0.0f;
    float simulationTime = 0.0f;
    float universeSize;

    bool started = false;
    bool saveToFiles;
    int mode;
    int num1, num2;

    uint64_t lastTime, newTime;
    float accTime, frameTime;

    bool keymap[256];

    struct RenderParameters
    {
        bool render_tree = false;

        enum class ParticleMode
        {
            Point,
            Billboard
        } particleMode = ParticleMode::Billboard;

    } renderParams;

    static Application* instance;
};