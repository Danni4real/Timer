#include <iostream>
#include "Timer.h"

void loop(Timer *timer) {
    for (int i = 0; i < 10000; i++) {
        std::cerr << i << std::endl;

        timer->set_timeout(1);
        timer->set_callback([]() {
            std::cout << "Timer expired! Executing callback..." << std::endl;
        });
        timer->start();
        timer->stop();
        timer->timed_out();
    }
}

int main() {
    Timer timer;

    std::thread t1(loop, &timer);
    std::thread t2(loop, &timer);
    std::thread t3(loop, &timer);
    
    t1.join();
    t2.join();
    t3.join();

    timer.set_callback([]() {
        std::cout << "Timer expired! Executing callback..." << std::endl;
    });

    std::cout << "test 1:" << std::endl;
    timer.set_timeout(2);
    timer.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    timer.stop();

    std::cout << "\ntest 2:" << std::endl;
    timer.set_timeout(1);
    timer.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    timer.stop();

    std::cout << "\ntest 3:" << std::endl;
    timer.set_timeout(1);
    timer.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    timer.stop();
    
    std::cout << "\ntest 4:" << std::endl;
    timer.set_timeout(3);
    timer.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    timer.restart();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    timer.stop();

    std::cout << "\nDestory timer:" << std::endl;
    return 0;
}
