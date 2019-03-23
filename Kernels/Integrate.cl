kernel void Integrate(
    const float time, 
    global float3* position, 
    global float3* velocity, 
    global const float3* acceleration, 
    global const float3* force,
    global const float* inverseMass)
{
    size_t i = get_global_id(0);
    
    float3 a = acceleration[i] + inverseMass[i] * force[i];
    float3 v = velocity[i] + a * time;
    velocity[i] = v;
    position[i] += v * time * 1;
}
