#pragma once

#include "Components.h"
#include "Engine.h"

class OrbitController : public InputHandler
{
public:
    OrbitController();

private:
    bool Input_Impl(Engine& game, const InputEvent& event, ObjectControllContext& context);
    void Activated_Impl(Engine& game, ObjectControllContext& context);
    void Deactivated_Impl(Engine& game, ObjectControllContext& context);
};
