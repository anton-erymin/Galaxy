#include "GalaxyEngine.h"
//#include "GalaxySimulator.h"   

GalaxyEngine::GalaxyEngine()
    : Engine()
{
}

GalaxyEngine::~GalaxyEngine()
{
    NLOG("~GalaxyEngine");
}

void GalaxyEngine::OnPostInitialize()
{
    //simulator_ = make_unique<GalaxySimulator>();   
}
