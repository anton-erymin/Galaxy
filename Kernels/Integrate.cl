kernel void Integrate(
    const float time, 
    global float3* position, 
    global float3* velocity, 
    global const float3* acceleration, 
    global const float3* force,
    global float* inverseMass)
{
    size_t i = get_global_id(0);
    
    //float3 acceleration = acceleration[i] + inverseMass[i] * force[i];
    //float3 velocity = velocity[i] + acceleration * time;
    //velocity[i] = velocity;
    //position[i] += velocity * time;
    float3 v = {0.1f, 0, 0};
    position[i] += v * time;
}
