//
// Created by L iu on 2023/8/12.
//

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

class ThreadPool {
public:
    explicit ThreadPool(unsigned int size = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<class F,class... Args>
    auto add(F &&f,Args &&...args) -> std::future<typename std::invoke_result<F, Args...>::type>;

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_variable_;
    std::atomic<bool> stop_{false};
};

template <class F, class... Args>
auto ThreadPool::add(F &&f, Args &&...args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        // don't allow enqueueing after stopping the pool
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks_.emplace([task]() { (*task)(); });
    }
    condition_variable_.notify_one();
    return res;
}


#endif //__THREADPOOL_H__
