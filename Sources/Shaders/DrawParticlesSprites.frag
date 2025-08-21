#include "SimulationLayout.h"

in vec4 frag_color;
in vec2 frag_uv;

out vec4 out_color;

uniform sampler2D g_image;

ROOT_CONSTANTS
{
    uint g_offset;
    float g_size_scale;
    float g_brightness;
};

void main()
{
    vec4 color = texture(g_image, frag_uv);
    out_color = vec4(g_brightness * frag_color.xyz * color.xyz, 1.0);
}
