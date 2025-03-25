# C++ Timer

Usage:

Timer timer;

timer.set_callback([] {
    std::cout << "Timer expired! Executing callback..." << std::endl;
});

timer.set_timeout(2);

timer.start();

std::this_thread::sleep_for(std::chrono::seconds(1));

timer.stop();

if (timer.timed_out())
    std::cout << "timed out!" << std::endl;
else
    std::cout << "stopped before timed out!" << std::endl;

