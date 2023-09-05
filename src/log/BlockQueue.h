#ifndef __BLOCKQUEUE_H
#define __BLOCKQUEUE_H
#include <deque>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>

template<typename T>
class BlockQueue{
public:
    explicit BlockQueue(size_t maxsize = 1000);
    ~BlockQueue();
    bool empty(); // 队列是否空
    bool full(); // 队列是否满
    void push_back(const T& item); // 尾插
    void push_front(const T& item); // 头插
    bool pop(T& item);  // 弹出的任务放入item
    bool pop(T& item, int timeout);  // 等待时间
    void clear(); // 清空队列
    T front(); // 头元素
    T back(); // 尾元素
    size_t capacity(); // 返回队列容量
    size_t size(); // 返回队列大小
    void flush(); // 唤醒消费者
    void close(); // 清空队列，通知全部生产者和消费者
private:
    std::deque<T> deq_;                      // 底层数据结构
    std::mutex mtx_;                         // 锁
    bool isClose_;                      // 关闭标志
    size_t capacity_;                   // 容量
    std::condition_variable condConsumer_;   // 消费者条件变量
    std::condition_variable condProducer_;   // 生产者条件变量
};

template<typename T>
BlockQueue<T>::BlockQueue(size_t maxsize) :capacity_(maxsize){
    assert(maxsize > 0);
    isClose_ = false;
}

template<typename T>
BlockQueue<T>::~BlockQueue() {
    close();
}

template<typename T>
void BlockQueue<T>::close() {
    clear();
    isClose_ = true;
    condConsumer_.notify_all();
    condProducer_.notify_all();
}

template<typename T>
void BlockQueue<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

template<typename T>
bool BlockQueue<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

template<typename T>
bool BlockQueue<T>::full() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<typename T>
void BlockQueue<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_){ // 队列满了，需要等待
        condProducer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}

template<typename T>
void BlockQueue<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_){ // 队列满了，需要等待
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

template<typename T>
bool BlockQueue<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.empty()){ // 队列空了，等待
        condConsumer_.wait(locker);
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();         // 唤醒生产者
    return true;
}

template<typename T>
bool BlockQueue<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()){
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout))
           == std::cv_status::timeout){
            return false;
        }
        if(isClose_){
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<typename T>
T BlockQueue<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template<typename T>
T BlockQueue<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template<typename T>
size_t BlockQueue<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template<typename T>
size_t BlockQueue<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template<typename T>
void BlockQueue<T>::flush() {
    condConsumer_.notify_one();
}


#endif //!__BLOCKQUEUE_H
