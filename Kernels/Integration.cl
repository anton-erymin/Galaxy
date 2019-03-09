//[[cl::required_work_group_size(1, 2, 3)]] 
__kernel void Integrate(__global float* a, __global float* b)
{
    a[get_global_id(0)] += b[get_global_id(0)];
}
