#include <iostream>
#include <thread>
#include "HeapTimer.h"

int main() {
    HeapTimer timer;
    timer.add(1, 1000, []() { std::cout << "Task 1 triggered" << std::endl; });
    timer.add(2, 2000, []() { std::cout << "Task 2 triggered" << std::endl; });
    timer.add(3, 3000, []() { std::cout << "Task 3 triggered" << std::endl; });
    timer.add(4, 900, []() { std::cout << "Task 4 triggered" << std::endl; });

    timer.doWork(1);
    timer.clear();
    while(true){
        timer.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(timer.getNextTick()));
    }
    return 0;
}
