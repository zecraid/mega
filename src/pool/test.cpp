#include <iostream>
#include <thread>
#include <functional>
#include <memory>
#include "ThreadPool.h"

int main() {
    std::unique_ptr<ThreadPool> pool = std::make_unique<ThreadPool>();
    pool->addTask([](){
        printf("helllo");
    });

    pool->addTask([](){
        printf("qvshiba");
    });
    return 0;
}
