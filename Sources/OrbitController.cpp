#include "OrbitController.h"

using namespace std::placeholders;

OrbitController::OrbitController()
    : InputHandler(
        std::bind(&OrbitController::Input_Impl, this, _1, _2, _3),
        std::bind(&OrbitController::Activated_Impl, this, _1, _2),
        std::bind(&OrbitController::Deactivated_Impl, this, _1, _2))
{
}

bool OrbitController::Input_Impl(Engine& engine, const InputEvent& event,
    ObjectControllContext& context)
{
	if (context.is_changed)
	{
        //Update(engine, context);
	}

    return context.is_changed;
}

void OrbitController::Activated_Impl(Engine& engine, ObjectControllContext& context)
{
}

void OrbitController::Deactivated_Impl(Engine& game, ObjectControllContext& context)
{
}
