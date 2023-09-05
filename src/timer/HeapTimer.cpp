#include "HeapTimer.h"

void HeapTimer::swapNode_(size_t i, size_t j) {
    assert(i >= 0 && i <heap_.size());
    assert(j >= 0 && j <heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void HeapTimer::siftUp_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t parent_index = (i - 1) / 2;
    while(parent_index >= 0){
        if(heap_[parent_index] > heap_[i]){
            swapNode_(i, parent_index);
            i = parent_index;
            parent_index = (i - 1) / 2;
        }else{
            break;
        }
    }
}

// true：下滑次数至少为1次
bool HeapTimer::siftDown_(size_t i, size_t n) {
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size());    // n:共几个结点
    size_t index = i;
    size_t child = 2 * child + 1;
    while(child < n) {
        if(child+1 < n && heap_[child+1] < heap_[child]) {
            child++;
        }
        if(heap_[child] < heap_[index]) {
            swapNode_(index, child);
            index = child;
            child = 2 * child + 1;
        } else
            break;
    }
    return index > i;
}

void HeapTimer::del_(size_t index) {
    assert(index >= 0 && index < heap_.size());
    // 将要删除的结点换到队尾，然后调整堆
    size_t tmp = index;
    size_t n = heap_.size() - 1;
    assert(tmp <= n);
    // 如果就在队尾，就不用移动了
    if(index < heap_.size() - 1) {
        swapNode_(tmp, heap_.size() - 1);
        if(!siftDown_(tmp, n)) {
            siftUp_(tmp);
        }
    }
    printf("pop %d\n",heap_.back().id);
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

// 调整指定id的结点
void HeapTimer::adjust(int id, int newExpires) {
    assert(!heap_.empty() && ref_.count(id));
    heap_[ref_[id]].expires = Clock::now() + std::chrono::milliseconds(newExpires);
    siftDown_(ref_[id], heap_.size());
}

void HeapTimer::add(int id, int timeOut, const std::function<void()> &callback) {
    assert(id >= 0);
    // 如果有，则调整
    if(ref_.count(id)){
        int tmp = ref_[id];
        heap_[tmp].expires = Clock::now() + std::chrono::milliseconds(timeOut);
        heap_[tmp].callback = callback;
        if(!siftDown_(tmp,heap_.size())){
            siftUp_(tmp);
        }
    }else{
        size_t n = heap_.size();
        ref_[id] = n;
        heap_.push_back({id,Clock::now() + std::chrono::milliseconds(timeOut), callback});
        siftUp_(n);
        for(int i = 0;i < heap_.size();i++){
            printf("%d ",heap_[i].id);
        }
        printf("\n");
    }
}

// 立即执行id所指向的定时器回调函数，并删除定时器
void HeapTimer::doWork(int id) {
    if(heap_.empty() || ref_.count(id) == 0){
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.callback(); // 触发回调函数
    del_(i);
}

// 检查并触发定时器堆中的任何已过期任务
void HeapTimer::tick(){
    // 清除超时结点
    if(heap_.empty()){
        return;
    }
    while(!heap_.empty()){
        TimerNode node = heap_.front();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(node.expires - Clock::now()).count() > 0) {
            break;
        }
        node.callback();
        pop();
    }
}

void HeapTimer::pop(){
    assert(!heap_.empty());
    del_(0);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

// 返回定时器堆中下一个将要过期的任务的剩余时间
int HeapTimer::getNextTick() {
    tick();
    size_t res = -1;
    if(!heap_.empty()){
        res = std::chrono::duration_cast<std::chrono::milliseconds>(heap_.front().expires - Clock::now()).count();
        if(res < 0){
            res = 0;
        }
    }
    return res;
}
