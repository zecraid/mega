// vsconde mode keraid_server
#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <condition_variable>  // NOLINT
#include <functional>
#include <future>  // NOLINT
#include <memory>
#include <mutex>  // NOLINT
#include <queue>
#include <thread>  // NOLINT
#include <utility>
#include <vector>
class ThreadPool{
public:
    explicit ThreadPool(unsigned int size = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<typename T>
    void addTask(T&& task){
        std::unique_lock<std::mutex> locker(mtx_);
        if(stop_){
            throw std::runtime_error("添加到一个已停止的线程池");
        }
        tasks_.emplace(std::forward<T>(task));
        cond_.notify_one();
    }

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cond_;
    std::atomic<bool> stop_{false};
};

#endif //!__THREADPOOL_H__