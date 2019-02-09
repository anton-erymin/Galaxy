#include "Threading.h"

#include <cassert>

bool                                    ThreadPool::terminate_;
std::mutex                              ThreadPool::mutex_;
std::condition_variable                 ThreadPool::sync_;
std::condition_variable                 ThreadPool::signal_;
std::uint32_t                           ThreadPool::count_;
std::uint32_t                           ThreadPool::block_size_;
std::uint32_t                           ThreadPool::block_count_;
std::atomic<std::uint32_t>              ThreadPool::block_index_;
std::unique_ptr<ThreadPool::KernelBase> ThreadPool::kernel_;
std::vector<std::thread>                ThreadPool::threads_;
std::uint32_t                           ThreadPool::thread_count_;

/**
    Constructor.
*/
ThreadPool::KernelBase::KernelBase()
{
}

/**
    Destructor.
*/
ThreadPool::KernelBase::~KernelBase()
{
}

/**
    Runs the kernel.
*/
void ThreadPool::KernelBase::Run()
{
}

/**
    Constructor.
*/
ThreadPool::ThreadPool()
{
}

/**
    Gets the number of CPU threads.

    \return The number of CPU threads.
*/
std::uint32_t ThreadPool::GetThreadCount()
{
    return static_cast<std::uint32_t>(threads_.size());
}

/**
    Creates the thread pool.

    \param thread_count The number of threads in the pool.
    \return true if the thread pool was created successfully.
*/
bool ThreadPool::Create(std::uint32_t thread_count)
{
    terminate_ = false;

    thread_count_ = 0;

    thread_count = (std::max(thread_count, 1u) + 1u) & ~1u;

    threads_.reserve(thread_count);

    // Spawn requested number of threads
    for (auto i = 0u; i < threads_.capacity(); ++i)
    {
        threads_.push_back(std::thread(Worker));
    }

    // Wait for all threads to have started
    for (;;)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (thread_count_ == threads_.size())
            break;  // all threads have started
        sync_.wait(lock);
    }

    return true;
}

/**
    Destroys the thread pool.
*/
void ThreadPool::Destroy()
{
    terminate_ = true;

    // Wait for all threads to have completed
    if (threads_.size() > 0)
    {
        signal_.notify_all();

        for (auto i = 0u; i < threads_.size(); ++i)
        {
            threads_[i].join();
        }
    }

    threads_.resize(0);

    thread_count_ = 0;
}

/**
    The worker thread.
*/
void ThreadPool::Worker()
{
    for (;;)
    {
        // Put the thread to sleep until some blocks need processing
        {
            std::unique_lock<std::mutex> lock(mutex_);
            assert(thread_count_ < threads_.capacity());
            if (++thread_count_ == threads_.capacity())
                sync_.notify_one(); // all threads are back to the pool
            signal_.wait(lock);
        }

        // Were we woken up to kill ourselves?
        if (terminate_)
        {
            break;
        }

        // Process all the available blocks
        {
            assert(kernel_);
            kernel_->Run();
        }
    }
}

