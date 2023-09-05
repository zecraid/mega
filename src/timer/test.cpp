#include <iostream>
#include <thread>
#include "HeapTimer.h"

int main() {
    HeapTimer timer;
    timer.add(1, 1000, []() { std::cout << "Task 1 triggered" << std::endl; });
    timer.add(2, 2000, []() { std::cout << "Task 2 triggered" << std::endl; });
    timer.add(3, 3000, []() { std::cout << "Task 3 triggered" << std::endl; });
    timer.add(4, 900, []() { std::cout << "Task 4 triggered" << std::endl; });
    timer.add(5, 1500, []() { std::cout << "Task 5 triggered" << std::endl; });
    timer.add(6, 2500, []() { std::cout << "Task 6 triggered" << std::endl; });
    timer.add(7, 4000, []() { std::cout << "Task 7 triggered" << std::endl; });
    timer.add(8, 600, []() { std::cout << "Task 8 triggered" << std::endl; });
    while(true){
        timer.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(timer.getNextTick()));
    }
    return 0;
}
