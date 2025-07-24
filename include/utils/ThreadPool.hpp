#pragma once

#include "http/HttpClient.hpp"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <borealis/core/singleton.hpp>
#include <list>

class ThreadPool : public brls::Singleton<ThreadPool> {
public:
    using Task = std::function<void(std::shared_ptr<HttpClient> client)>;

    void submit(Task fn) {
        {
            std::lock_guard<std::mutex> lock(taskMutex);
            tasks.emplace_back(fn);
        }
        this->taskCondition.notify_one();
    }

    void start(size_t num);

    size_t size() const {return threads.size();}

    void stop();

    static size_t max_thread_num;

    explicit ThreadPool();
    virtual ~ThreadPool();

private:
    static void* task_loop(void* arg);
    typedef std::shared_ptr<std::thread> Thread;
    std::list<Thread> threads;
    std::mutex threadMutex;
    std::list<Task> tasks;
    std::mutex taskMutex;
    std::condition_variable taskCondition;
    std::atomic_bool isStop;
};

