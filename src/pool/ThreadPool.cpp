#include "ThreadPool.h"
ThreadPool::ThreadPool(unsigned int size) {
    assert(size > 0);
    for (unsigned int i = 0; i < size; ++i) { // 创建指定数量的线程并将它们添加到线程池中
        threads_.emplace_back(std::thread([this]() {
            std::unique_lock<std::mutex> locker(mtx_);
            while (true) {
                if(!tasks_.empty()){
                    std::function<void()> task = std::move(tasks_.front());
                    tasks_.pop();
                    locker.unlock();
                    task();
                    locker.lock();
                } else if(stop_){
                    break;
                } else {
                    cond_.wait(locker);
                }
            }
        }));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mtx_);
        stop_ = true;
    }
    cond_.notify_all();
    for (std::thread &th : threads_) {
        if (th.joinable()) {
            th.join();
        }
    }
}

