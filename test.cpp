#include <unistd.h>

#include <iostream>
#include "Timer.h"

int main() {
    Timer timer;

    timer.set_each_timing_start_callback([]() {
        std::cout << "timing start!" << std::endl;
    });

    timer.set_each_timeout_callback([]() {
        std::cout << "timeout, but still got chance!" << std::endl;
    });
    
    timer.set_final_timeout_callback([]() {
        std::cout << "timeout, you got no chance left!" << std::endl;
        exit(1);
    });
    
    timer.set_loop_times(3);
    timer.set_timeout(3);
    timer.start();

    while(true)
        sleep(1);
      
    return 0;
}
