#include "OpenCL.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <iostream>

namespace cl
{

static inline void ClCheckStatus(cl_int status, const char* message = nullptr)
{
    if (status < 0)
    {
        throw std::runtime_error("OpenCL error: " + (message ? std::string(message) : "") + ", status = " + std::to_string(status));
    }
}

static void __stdcall ContextErrorCallback(const char *errinfo, const void *privateInfo, size_t cb, void *userData)
{
    std::cerr << "OpenCL context error: " + std::string(errinfo) << std::endl;
}

OpenCL::OpenCL()
{
    cl_uint numPlatforms = 0;
    ClCheckStatus(clGetPlatformIDs(1, &platform, &numPlatforms), "Failed to get platforms");

    if (numPlatforms == 0)
    {
        throw std::runtime_error("No OpenCL platform found");
    }

    devices.resize(10);
    cl_uint numDevices = 0;
    ClCheckStatus(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, static_cast<cl_uint>(devices.size()), devices.data(), &numDevices), "Failed to get devices");

    if (numDevices == 0)
    {
        throw std::runtime_error("No suitable OpenCL device found");
    }

    cl_int status = -1;
    context = clCreateContext(nullptr, 1, devices.data(), ContextErrorCallback, nullptr, &status);
    ClCheckStatus(status, "Failed to create context");

    queue = clCreateCommandQueueWithProperties(context, devices[0], nullptr, &status);
    ClCheckStatus(status, "Failed to create command queue");
}

void OpenCL::FlushCommandQueue() const
{
    ClCheckStatus(clFlush(queue), "Failed to flush");
}

void OpenCL::WaitIdle() const
{
    ClCheckStatus(clFinish(queue), "Failed to finish");
}

}
