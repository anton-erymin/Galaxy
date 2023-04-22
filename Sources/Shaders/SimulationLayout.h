layout(std430) buffer Position { vec4 g_position[]; };
layout(std430) buffer Velocity { vec4 g_velocity[]; };
layout(std430) buffer Acceleration { vec4 g_acceleration[]; };
layout(std430) buffer Mass { float g_mass[]; };
layout(std430) buffer Children { int g_children[];};
