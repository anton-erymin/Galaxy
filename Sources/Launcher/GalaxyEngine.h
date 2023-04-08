#pragma once

#include <Engine.h>

//class GalaxySimulator;

class GalaxyEngine final : public Engine
{
public:
    GalaxyEngine();
    virtual ~GalaxyEngine();

    virtual void OnPostInitialize() override;

private:
    //unique_ptr<GalaxySimulator> simulator_;
};
