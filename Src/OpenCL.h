#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include <cl/cl.h>

namespace cl
{

class Program;
class Kernel;
class Buffer;
class Event;

using ProgramPtr = std::shared_ptr<Program>;
using KernelPtr = std::shared_ptr<Kernel>;
using BufferPtr = std::shared_ptr<Buffer>;
using EventPtr = std::shared_ptr<Event>;

enum class MemoryType
{
    ReadOnly = 0,
    WriteOnly = 1,
    ReadWrite = 2
};

void ClCheckStatus(cl_int status, const char* message = nullptr);

class OpenCL
{
public:
    OpenCL();
    ~OpenCL();

    ProgramPtr CreateProgram(const std::string& source);
    EventPtr CreateUserEvent();
    BufferPtr CreateBuffer(size_t size, MemoryType memoryType = MemoryType::ReadWrite, void* hostPtr = nullptr);

    void EnqueueKernel(
        KernelPtr kernel, 
        uint32_t numDimensions, 
        size_t globalX, 
        size_t globalY, 
        size_t globalZ, 
        size_t groupSizeX, 
        size_t groupSizeY, 
        size_t groupSizeZ, 
        const std::vector<EventPtr>& waitList = {},
        bool createEvent = false, 
        EventPtr& event = EventPtr());

    void EnqueueReadBuffer(
        BufferPtr buffer, 
        size_t offset, 
        size_t size, 
        void* data, 
        bool blocking = true,
        const std::vector<EventPtr>& waitList = {},
        bool createEvent = false, 
        EventPtr& event = EventPtr());

    void EnqueueWriteBuffer(
        BufferPtr buffer, 
        size_t offset, 
        size_t size, 
        const void* data, 
        bool blocking = true,
        const std::vector<EventPtr>& waitList = {},
        bool createEvent = false, 
        EventPtr& event = EventPtr());
    
    void EnqueueBarrier(
        const std::vector<EventPtr>& waitList = {},
        bool createEvent = false, 
        EventPtr& event = EventPtr());

    cl_device_id GetDevice() const { return devices[0]; }
    cl_context GetContext() const { return context; }
    cl_command_queue GetCommandQueue() const { return queue; }

    void FlushCommandQueue() const;
    void WaitIdle() const;

private:
    cl_platform_id platform = nullptr;
    std::vector<cl_device_id> devices;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
};

class Resource
{
public:
    Resource(OpenCL& cl) : cl(cl) { }

    inline OpenCL& GetOpenCL() { return cl; }

private:
    OpenCL& cl;
};

class Kernel : public Resource
{
public:
    Kernel(OpenCL& cl, Program& program, const char* name);
    ~Kernel();

    cl_kernel GetKernel() const { return kernel; }
    const std::string& GetName() const { return name; }
    Program& GetProgram() { return program; }

    template <typename T>
    void SetArg(T value, uint32_t arg = 0);
    void SetArg(Buffer* buffer, uint32_t arg = 0);

private:
    cl_kernel kernel = nullptr;
    std::string name;
    Program& program;
};

class Program : public Resource
{
public:
    Program(OpenCL& cl, const std::string& source);
    ~Program();

    KernelPtr GetKernel(const char* name);

    cl_program GetProgram() const { return program; }

private:
    cl_program program = nullptr;
    std::unordered_map<std::string, KernelPtr> kernels;
};

class MemoryObject : public Resource
{
public:
    MemoryObject(OpenCL& cl) : Resource(cl) { }
    virtual ~MemoryObject();

    cl_mem GetMemory() const { return object; }

protected:
    cl_mem object = nullptr;
};

class Buffer : public MemoryObject
{
public:
    Buffer(OpenCL& cl, size_t size, MemoryType memoryType = MemoryType::ReadWrite, void* hostPtr = nullptr);

    size_t GetSize() const { return size; }
    MemoryType GetMemoryType() const { return memoryType; }

private:
    size_t size;
    MemoryType memoryType;
    void* hostPtr = nullptr;
};

class Event : public Resource
{
public:
    Event(OpenCL& cl);
    Event(OpenCL& cl, cl_event event);
    ~Event();

    void WaitFor() const;
    void SetUserEventStatus(cl_int status);

    cl_event GetEvent() const { return event; }

private:
    cl_event event = nullptr;
    bool isUserEvent = true;
};

template<typename T>
void Kernel::SetArg(T value, uint32_t arg)
{
    T val = value;
    ClCheckStatus(clSetKernelArg(kernel, arg, sizeof(T), &val), "Failed to set kernel arg");
}

}
