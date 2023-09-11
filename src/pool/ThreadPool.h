#ifndef THREADPOOL_H
#define THREADPOOL_H

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

    // 尽量用make_shared代替new，如果通过new再传递给shared_ptr，内存是不连续的，会造成内存碎片化
    explicit ThreadPool(int threadCount = 8){
        assert(threadCount > 0);
        for(int i = 0; i < threadCount; i++) {
            std::thread([this]() {
                std::unique_lock<std::mutex> locker(mtx_);
                while(true) {
                    if(!tasks.empty()) {
                        auto task = std::move(tasks.front());    // 左值变右值,资产转移
                        tasks.pop();
                        locker.unlock();    // 因为已经把任务取出来了，所以可以提前解锁了
                        task();
                        locker.lock();      // 马上又要取任务了，上锁
                    } else if(isClosed) {
                        break;
                    } else {
                        cond_.wait(locker);    // 等待,如果任务来了就notify的
                    }

                }
            }).detach();
        }
    }

    ~ThreadPool() {
        std::unique_lock<std::mutex> locker(mtx_);
        isClosed = true;
        cond_.notify_all();  // 唤醒所有的线程
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
    std::queue<std::function<void()>> tasks; // 任务队列，函数类型为void()
};

#endif