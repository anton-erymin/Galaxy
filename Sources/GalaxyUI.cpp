#include "GalaxyUI.h"
#include "GalaxyEngine.h"

namespace UI
{

void GalaxyUI::Build()
{
#if 0
    if (ImGui::CollapsingHeader("Galaxy"))
    {
        const float kCoeff = 10000.0f;

        float dt = engine_.deltaTime * kCoeff;

        if (ImGui::SliderFloat("Timestep", &dt, -1.0f, 1.0f, "%.3f", 1.0f))
        {
            engine_.UpdateDeltaTime(dt / kCoeff);
        }
    }
#endif // 0

}

}
