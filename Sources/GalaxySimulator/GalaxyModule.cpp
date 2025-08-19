#include "GalaxyModule.h"
#include "GalaxySimulator.h"   
#include "Misc/Paths.h"
#include "File/Filesystem.h"

IEngine* iengine = nullptr;

GameModule::GameModule()
{
}

GameModule::~GameModule()
{
}

void GameModule::Initialize(IEngine& eng)
{
    iengine = &eng;
    Paths::SetShadersDir(Filesystem::MakeAbsolute(Paths::BaseDir() + "/../../../Libs/Nucleus/Sources/Nucleus/Engine/Shaders"));
}

void GameModule::PostInitialize()
{
    simulator_ = MakeUnique<GalaxySimulator>();
}

void GameModule::Shutdown()
{
}
