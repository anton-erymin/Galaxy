#include "OpenCL.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

namespace cl
{

static thread_local std::vector<cl_event> events;
static thread_local cl_event clEvent = nullptr;

inline void ClCheckStatus(cl_int status, const char* message)
{
    if (status < CL_SUCCESS)
    {
        std::string str = "OpenCL error: " + (message ? std::string(message) : "") + ", status = " + std::to_string(status);
        std::cout << str<< std::endl;
        throw std::runtime_error(str);
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

    devices.resize(10, nullptr);
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

OpenCL::~OpenCL()
{
    ClCheckStatus(clReleaseCommandQueue(queue), "Failed to release queue");
    ClCheckStatus(clReleaseContext(context), "Failed to release context");
    ClCheckStatus(clReleaseDevice(devices[0]), "Failed to release device");
}

static void PrepareEvents(const std::vector<EventPtr>& waitList)
{
    events.resize(waitList.size());
    for (size_t i = 0; i < waitList.size(); ++i)
    {
        events[i] = waitList[i]->GetEvent();
    }
}

void OpenCL::EnqueueKernel(
    KernelPtr kernel, 
    uint32_t numDimensions, 
    size_t globalX, 
    size_t globalY, 
    size_t globalZ, 
    size_t groupSizeX, 
    size_t groupSizeY, 
    size_t groupSizeZ, 
    const std::vector<EventPtr>& waitList, 
    bool createEvent, 
    EventPtr& event)
{
    assert(kernel);
    assert(queue);
    assert(numDimensions >= 1 && numDimensions <= 3);

    PrepareEvents(waitList);

    const size_t global[3] = { globalX, globalY, globalZ };
    const size_t group[3] = { groupSizeX, groupSizeY, groupSizeZ };
    ClCheckStatus(clEnqueueNDRangeKernel(
        queue, kernel->GetKernel(), numDimensions, nullptr, global, group, static_cast<cl_uint>(waitList.size()), events.data(), createEvent ? &clEvent : nullptr), 
    "Failed to enqueue kernel");

    if (createEvent)
    {
        event = std::make_shared<Event>(*this, clEvent);
    }
}

void OpenCL::EnqueueReadBuffer(
    BufferPtr buffer, 
    size_t offset, 
    size_t size, 
    void* data, 
    bool blocking, 
    const std::vector<EventPtr>& waitList, 
    bool createEvent, 
    EventPtr& event)
{
    assert(buffer);
    assert(offset + size <= buffer->GetSize());
    assert(data);

    PrepareEvents(waitList);

    ClCheckStatus(clEnqueueReadBuffer(
        queue, buffer->GetMemory(), blocking, offset, size, data, static_cast<cl_uint>(waitList.size()), events.data(), createEvent ? &clEvent : nullptr), 
        "Failed to enqueue write buffer command");

    if (createEvent)
    {
        event = std::make_shared<Event>(*this, clEvent);
    }
}

void OpenCL::EnqueueWriteBuffer(
    BufferPtr buffer, 
    size_t offset, 
    size_t size, 
    const void* data, 
    bool blocking, 
    const std::vector<EventPtr>& waitList, 
    bool createEvent, 
    EventPtr& event)
{
    assert(buffer);
    assert(offset + size <= buffer->GetSize());
    assert(data);

    PrepareEvents(waitList);

    ClCheckStatus(clEnqueueWriteBuffer(
        queue, buffer->GetMemory(), blocking, offset, size, data, static_cast<cl_uint>(waitList.size()), events.data(), createEvent ? &clEvent : nullptr), 
    "Failed to enqueue write buffer command");

    if (createEvent)
    {
        event = std::make_shared<Event>(*this, clEvent);
    }
}

void OpenCL::EnqueueBarrier(const std::vector<EventPtr>& waitList, bool createEvent, EventPtr& event)
{
    PrepareEvents(waitList);

    ClCheckStatus(clEnqueueBarrierWithWaitList(
        queue, static_cast<cl_uint>(waitList.size()), events.data(), createEvent ? &clEvent : nullptr), 
        "Failed to enqueue barrier");

    if (createEvent)
    {
        event = std::make_shared<Event>(*this, clEvent);
    }
}

void OpenCL::FlushCommandQueue() const
{
    assert(queue);
    ClCheckStatus(clFlush(queue), "Failed to flush");
}

void OpenCL::WaitIdle() const
{
    assert(queue);
    ClCheckStatus(clFinish(queue), "Failed to finish");
}

ProgramPtr OpenCL::CreateProgram(const std::string& source)
{
    return std::make_shared<Program>(*this, source);
}

EventPtr OpenCL::CreateUserEvent()
{
    return std::make_shared<Event>(*this);
}

BufferPtr OpenCL::CreateBuffer(size_t size, MemoryType memoryType, void* hostPtr)
{
    return std::make_shared<Buffer>(*this, size, memoryType, hostPtr);
}

MemoryObject::~MemoryObject()
{
    assert(object);
    clReleaseMemObject(object);
}

Event::Event(OpenCL& cl)
    : Resource(cl)
{
    cl_int status = -1;
    event = clCreateUserEvent(cl.GetContext(), &status);
    ClCheckStatus(status, "Failed to create user event");
}

Event::Event(OpenCL& cl, cl_event event)
    : Resource(cl)
    , event(event)
    , isUserEvent(false)
{
}

Event::~Event()
{
    clReleaseEvent(event);
}

void Event::WaitFor() const
{
    ClCheckStatus(clWaitForEvents(1, &event), "Failed to wait on event");
}

void Event::SetUserEventStatus(cl_int status)
{
    assert(isUserEvent);
    ClCheckStatus(clSetUserEventStatus(event, status), "Failed to set user event status");
}

Program::Program(OpenCL& cl, const std::string& source)
    : Resource(cl)
{
    const char* string = source.c_str();
    cl_int status = -1;
    program = clCreateProgramWithSource(cl.GetContext(), 1, &string, nullptr, &status);
    ClCheckStatus(status, "Failed to create program");
    status = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);

    if (status != CL_SUCCESS)
    {
        char buildLog[1024];
        clGetProgramBuildInfo(program, GetOpenCL().GetDevice(), CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, nullptr);
        std::cout << buildLog << std::endl;
    }

    ClCheckStatus(status, "Failed to build program");
}

Program::~Program()
{
    kernels.clear();
    clReleaseProgram(program);
}

KernelPtr Program::GetKernel(const char* name)
{
    auto it = kernels.find(name);
    if (it != kernels.end())
    {
        return it->second;
    }

    KernelPtr kernel = std::make_shared<Kernel>(GetOpenCL(), *this, name);
    kernels[name] = kernel;
    return kernel;
}

Kernel::Kernel(OpenCL& cl, Program& program, const char* name)
    : Resource(cl)
    , name(name)
    , program(program)
{
    cl_int status = -1;
    kernel = clCreateKernel(program.GetProgram(), name, &status);
    ClCheckStatus(status, "Failed to create kernel");
}

Kernel::~Kernel()
{
    clReleaseKernel(kernel);
}

void Kernel::SetArg(Buffer* buffer, uint32_t arg)
{
    assert(buffer);
    assert(kernel);

    cl_mem memory = buffer->GetMemory();
    assert(memory);
    ClCheckStatus(clSetKernelArg(kernel, arg, sizeof(cl_mem), &memory), "Failed to set kernel arg");
}

static cl_mem_flags MemoryTypeToMemFlags(MemoryType type)
{
    switch (type)
    {
    case cl::MemoryType::ReadOnly:
        return CL_MEM_READ_ONLY;
    case cl::MemoryType::WriteOnly:
        return CL_MEM_WRITE_ONLY;
    case cl::MemoryType::ReadWrite:
        return CL_MEM_READ_WRITE;
    default:
        assert(!"Unknown");
        break;
    }

    return CL_MEM_READ_ONLY;
}

Buffer::Buffer(OpenCL& cl, size_t size, MemoryType memoryType, void* hostPtr)
    : MemoryObject(cl)
    , size(size)
    , memoryType(memoryType)
    , hostPtr(hostPtr)
{
    cl_int status = -1;
    object = clCreateBuffer(cl.GetContext(), MemoryTypeToMemFlags(memoryType), size, hostPtr, &status);
    ClCheckStatus(status, "Failed to create buffer");
}

}
