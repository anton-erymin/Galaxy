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
std::vector<std::thread>                ThreadPool::threads_;
std::uint32_t                           ThreadPool::thread_count_;
ThreadPool::Kernel                      ThreadPool::kernel_;

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

    block_index_ = 0;
    block_count_ = 0;

    thread_count_ = 0;

    thread_count = (std::max(thread_count, 1u) + 1u) & ~1u;

    threads_.reserve(thread_count);

    // Spawn requested number of threads
    for (auto i = 0u; i < threads_.capacity(); ++i)
    {
        threads_.push_back(std::thread(Worker));
    }

    // Wait for all threads to have started
    while (true)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (thread_count_ == threads_.size())
        {
            break;  // all threads have started
        }
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
    while (true)
    {
        // Put the thread to sleep until some blocks need processing
        //while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            assert(thread_count_ < threads_.capacity());

            if (thread_count_ < threads_.capacity())
            {
                ++thread_count_;
                if (thread_count_ == threads_.capacity())
                {
                    sync_.notify_one(); // all threads are back to the pool
                }
            }

            //if (block_index_ < block_count_ || terminate_)
            //{
            //    break; // Have some work or terminate
            //}

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

            while (true)
            {
                auto const block_index = block_index_++;

                if (block_index >= block_count_)
                {
                    break;  // everything has been processed
                }

                auto const begin = block_index * block_size_;
                auto const end = std::min((block_index + 1) * block_size_, count_ - 1);

                for (auto index = begin; index <= end; ++index)
                {
                    kernel_(index); // run the kernel
                }
            }
        }
    }
}

void ThreadPool::Dispatch(const Kernel& kernel, std::uint32_t count, std::uint32_t block_size) const
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
        kernel_ = kernel;

        // And dispatch
        thread_count_ = 0;  // consume all threads from the pool
        signal_.notify_all();

        // Wait until all threads have returned to the pool
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (thread_count_ == threads_.size())
            {
                break;  // all threads have completed
            }
            sync_.wait(lock);
        }

        // Release kernel
        kernel_ = nullptr;
    }
}