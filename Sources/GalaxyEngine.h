#pragma once

#include "Engine.h"

class GalaxyEngine : public Engine
{
public:
    GalaxyEngine(std::uint32_t width, std::uint32_t height, void* window_handle,
        const char* shaders_path);

    virtual ~GalaxyEngine() = default;

private:
};
