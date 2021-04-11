#pragma once

#include <atomic>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

#undef min
#undef max

/**
    The class for dispatching work across the CPU cores.
*/
class ThreadPool
{
public:
    using Kernel = std::function<void(uint32_t)>;

    ThreadPool();

    void Dispatch(const Kernel& kernel, std::uint32_t count, std::uint32_t block_size = 16) const;

    static std::uint32_t GetThreadCount();

    static bool Create(std::uint32_t thread_count);
    static void Destroy();

private:

    static void Worker();

    // Whether to terminate the threads.
    static bool terminate_;
    // The mutex for synchronization.
    static std::mutex mutex_;
    // The condition variable for synchronizing.
    static std::condition_variable sync_;
    // The condition variable for signalling.
    static std::condition_variable signal_;
    // The number of kernel invocations.
    static std::uint32_t count_;
    // The size of a block.
    static std::uint32_t block_size_;
    // The number of available blocks.
    static std::uint32_t block_count_;
    // The index of the currently executed block.
    static std::atomic<std::uint32_t> block_index_;
    // The available CPU threads.
    static std::vector<std::thread> threads_;
    // The number of available threads.
    static std::uint32_t thread_count_;
    // Current kernel
    static Kernel kernel_;
};
