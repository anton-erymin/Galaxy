#pragma once

#include <atomic>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <iostream>

#undef min
#undef max

/**
    The class for dispatching work across the CPU cores.
*/
class ThreadPool
{
public:
    ThreadPool();

    template<typename KERNEL>
    void Dispatch(KERNEL const& kernel, std::uint32_t count, std::uint32_t block_size = 16) const;

    static std::uint32_t GetThreadCount();

    static bool Create(std::uint32_t thread_count);
    static void Destroy();

protected:
    /**
        The type for a kernel to be executed.
    */
    class KernelBase
    {
    public:
        KernelBase();
        virtual ~KernelBase();

        virtual void Run() = 0;
    };

    /**
        The type for a kernel to be executed.
    */
    template<typename KERNEL>
    class Kernel : public KernelBase
    {
    public:
        Kernel(KERNEL const& kernel);

        void Run() override;

    protected:
        // The kernel to be executed.
        KERNEL const& kernel_;
    };

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
    // The kernel scheduled for execution.
    static std::unique_ptr<KernelBase> kernel_;
    // The available CPU threads.
    static std::vector<std::thread> threads_;
    // The number of available threads.
    static std::uint32_t thread_count_;
};

/**
    Constructor.

    \param kernel The kernel to be executed.
*/
template<typename KERNEL>
ThreadPool::Kernel<KERNEL>::Kernel(KERNEL const& kernel)
    : kernel_(kernel)
{
}

/**
    Runs the kernel.
*/
template<typename KERNEL>
void ThreadPool::Kernel<KERNEL>::Run()
{
    for (;;)
    {
        auto const block_index = block_index_++;

        if (block_index >= block_count_)
        {
            break;  // everything has been processed
        }

        auto const begin = block_index * block_size_;

        auto const end = std::min((block_index + 1) * block_size_, count_ - 1);

        //std::cout << "Process block " << block_index << " from " << begin << " to " << end << std::endl;

        for (auto index = begin; index < end; ++index)
        {
            kernel_(index); // run the kernel
        }
    }
}

/**
    Dispatches the kernel.

    \param kernel The kernel to be executed.
    \param count The number of kernel invocations.
    \param block_size The size of a block.
*/
template<typename KERNEL>
void ThreadPool::Dispatch(KERNEL const& kernel, std::uint32_t count, std::uint32_t block_size) const
{
    // Set dispatch properties
    count_ = count;
    block_size_ = std::max(block_size, 1u);
    block_count_ = (count_ + block_size_ - 1u) / block_size_;

    // Special case - only 1 block? no need to go wide
    if (block_count_ <= 1u)
    {
        for (auto i = 0u; i < count; ++i)
        {
            kernel(i);
        }
    }
    else
    {
        // Install the kernel
        block_index_ = 0u;
        kernel_ = std::make_unique<Kernel<KERNEL> >(kernel);

        // And dispatch
        thread_count_ = 0;  // consume all threads from the pool
        signal_.notify_all();

        // Wait until all threads have returned to the pool
        for (;;)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (thread_count_ == threads_.size())
                break;  // all threads have completed
            sync_.wait(lock);
        }

        // Release kernel
        kernel_.reset();
    }
}