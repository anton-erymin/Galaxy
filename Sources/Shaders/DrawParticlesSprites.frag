#include "SimulationLayout.h"

in vec4 frag_color;
out vec4 out_color;

ROOT_CONSTANTS
{
    float g_size_scale;
    float g_brightness;
};

void main()
{
    out_color = vec4(g_brightness * frag_color.xyz, 1.0);
}
