#include "Galaxy.h"
#include "GalaxySimulator.h"   

GameModule::GameModule()
{
}

GameModule::~GameModule()
{
}

void GameModule::Initialize(IEngine& eng)
{
}

void GameModule::PostInitialize()
{
    simulator_ = MakeUnique<GalaxySimulator>();
}

void GameModule::Shutdown()
{
}
