#pragma once

#include "Application.h"
#include "GalaxyEngine.h"

class GalaxyApplication : public Application
{
public:
    GalaxyApplication() = default;

    virtual ~GalaxyApplication() = default;

    /* Initializes application before window start. */
    void Startup(int argc, char** argv, uint32_t width, uint32_t height,
        void* window_handle) override;

    /* Is called in main loop if no other window messages exist. */
    void Update() override;

    /* If we want to continue. */
    bool IsRun() override;

    /* Cleanups application. */
    void Cleanup() override;

    /* When resize event occured. */
    void Resize(uint32_t width, uint32_t height) override;

    /* Keyboard and mouse input events. */
    void Input(const InputEvent& event) override;

private:
    std::unique_ptr<GalaxyEngine> engine_;
};
