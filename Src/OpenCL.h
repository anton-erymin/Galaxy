#pragma once

#include <vector>

#include <cl/cl.h>

namespace cl
{

class OpenCL
{
public:
    OpenCL();

    cl_device_id GetDevice() const { return devices[0]; }
    cl_context GetContext() const { return context; }
    cl_command_queue GetCommandQueue() const { return queue; }

    void FlushCommandQueue() const;
    void WaitIdle() const;

private:
    cl_platform_id platform;
    std::vector<cl_device_id> devices;
    cl_context context;
    cl_command_queue queue;
};

}
