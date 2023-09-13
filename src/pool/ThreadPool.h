#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <assert.h>

class ThreadPool {
public:
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;

    explicit ThreadPool(int threadCount = 8){
        assert(threadCount > 0);
        for(int i = 0; i < threadCount; i++) {
            std::thread([this]() {
                std::unique_lock<std::mutex> locker(mtx_);
                while(true) {
                    if(!tasks.empty()) {
                        auto task = std::move(tasks.front());
                        tasks.pop();
                        locker.unlock();    // 任务取出执行，解锁
                        task();
                        locker.lock();      // 任务执行完成，加锁，准备取出下一个任务
                    } else if(isClosed) {
                        break;
                    } else {
                        cond_.wait(locker);
                    }

                }
            }).detach();
        }
    }

    ~ThreadPool() {
        std::unique_lock<std::mutex> locker(mtx_);
        isClosed = true;
        cond_.notify_all();
    }

    template<typename T>
    void AddTask(T&& task) {
        std::unique_lock<std::mutex> locker(mtx_);
        tasks.emplace(std::forward<T>(task));
        cond_.notify_one();
    }

private:
    std::mutex mtx_;
    std::condition_variable cond_;
    bool isClosed;
    std::queue<std::function<void()>> tasks;
};

#endif //!__THREADPOOL_H__