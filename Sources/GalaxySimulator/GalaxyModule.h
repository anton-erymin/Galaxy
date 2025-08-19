#pragma once

#include <Misc/IGameModule.h>
#include <ResourceManager.h>
#include <EngineMinimal.h>

class GalaxySimulator;

#ifndef DONT_DECLARE_ENGINE_PTR
extern IEngine* iengine;
#endif

class GAMEPLAY_API GameModule final : public IGameModule
{
public:
    GameModule();
    virtual ~GameModule();

    virtual void Initialize(IEngine& eng) override;
    virtual void PostInitialize() override;
    virtual void Shutdown() override;

private:
    UniquePtr<GalaxySimulator> simulator_;
};
