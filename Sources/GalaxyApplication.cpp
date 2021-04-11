#include "GalaxyApplication.h"

void GalaxyApplication::Startup(int argc, char **argv, 
    uint32_t width, uint32_t height, void* window_handle)
{
    const char* shaders_path = "../../Libs/Nucleus/Sources/Nucleus/Shaders";

    engine_ = std::make_unique<GalaxyEngine>(width, height, window_handle, shaders_path);
}

void GalaxyApplication::Update()
{
    engine_->Tick();
}

bool GalaxyApplication::IsRun()
{
    return engine_->IsRun();
}

void GalaxyApplication::Cleanup()
{
    engine_.reset();
}

void GalaxyApplication::Resize(uint32_t width, uint32_t height)
{
    engine_->Resize(width, height);
}

void GalaxyApplication::Input(const InputEvent& event)
{
    engine_->Input(event);
}
