#include "utils/ThreadPool.hpp"

#include <memory>
#include <borealis/core/logger.hpp>

constexpr std::chrono::milliseconds max_idle_time{60000}; // 60 seconds

#ifdef __SWITCH__
size_t ThreadPool::max_thread_num = 4;
#else
size_t ThreadPool::max_thread_num = (std::min)(int(std::thread::hardware_concurrency()), 5);
#endif

ThreadPool::ThreadPool() {
    brls::Logger::info("ThreadPool initialized with max threads: {}", max_thread_num);
    size_t num = max_thread_num;
    this->start(num);
}

ThreadPool::~ThreadPool() { brls::Logger::info("ThreadPool destroyed"); }

void ThreadPool::start(size_t num) {
    brls::Logger::info("Starting ThreadPool with {} threads", num);

    while(this->threads.size() < num) {
        Thread thread = std::make_shared<std::thread>(task_loop, this);
        std::lock_guard<std::mutex> lock(this->threadMutex);
        this->threads.push_back(thread);
    }
}

void* ThreadPool::task_loop(void* ptr) {
    ThreadPool* pool = reinterpret_cast<ThreadPool*>(ptr);
    std::shared_ptr<HttpClient> client = std::make_shared<HttpClient>();

    while(!pool->isStop.load()) {
        Task task;

        {
            std::unique_lock<std::mutex> locker(pool->taskMutex);
            pool->taskCondition.wait_for(locker, std::chrono::milliseconds(max_idle_time),
                [&pool] { return !pool->tasks.empty() || pool->isStop.load(); });

            if (pool->tasks.empty()) {
                continue;
            }

            task = std::move(pool->tasks.front());
            pool->tasks.pop_front();
        }

        if (task) {
            try {
                task(client);
            } catch (const std::exception& e) {
                brls::Logger::error("Task execution failed: {}", e.what());
            }
        }
    }
    brls::Logger::debug("ThreadPool exit : {}", fmt::ptr(pool));
    return nullptr;
}

void ThreadPool::stop() {
    this->isStop.store(true);
    this->taskCondition.notify_all();

    for (auto &th : this->threads) {
        th->join();
    }
    threads.clear();
}