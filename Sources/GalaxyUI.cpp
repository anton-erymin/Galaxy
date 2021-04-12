#include "GalaxyUI.h"
#include "GalaxyEngine.h"

namespace UI
{

void GalaxyUI::Build()
{
    if (ImGui::CollapsingHeader("Galaxy"))
    {
        if (ImGui::SliderFloat("Timestep", &engine_.deltaTime, -0.1f, 0.1f, "%.3f", 100.0f))
        {
            engine_.UpdateDeltaTime();
        }
    }
}

}
