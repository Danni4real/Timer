//
// Created by dan on 25-3-24.
//

#ifndef TIMER_H
#define TIMER_H

#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

class Timer {
public:
    Timer();
    ~Timer();

    void stop();
    void start();

    void restart();

    bool stopped();

    bool set_timeout(int seconds);

    // times of timing loop
    bool set_loop_times(int times);

    // each_timeout_callback will be called at end of each timing loop (each timeout)
    bool set_each_timeout_callback(const std::function<void()> &callback);

    // final_timeout_callback will be called at end of final timing loop (last timeout)
    bool set_final_timeout_callback(const std::function<void()> &callback);

    // each_timing_start_callback will be called at start of each timing loop
    bool set_each_timing_start_callback(const std::function<void()> &callback);

private:
    void run();

    void stop_private();
    void start_private();

    bool stopped_private();

    bool set_timeout_private(int seconds);
    bool set_loop_times_private(int times);

    bool set_each_timeout_callback_private(const std::function<void()> &callback);
    bool set_final_timeout_callback_private(const std::function<void()> &callback);
    bool set_each_timing_start_callback_private(const std::function<void()> &callback);

    int loop_times_{1};
    int timeout_sec_{0};
    bool running_{false};
    bool cmd_executed_{false}; // start or stop cmd
    bool quit_thread_{false}; // only used by destructor


    std::function<void()> each_timeout_callback_{nullptr};
    std::function<void()> final_timeout_callback_{nullptr};
    std::function<void()> each_timing_start_callback_{nullptr};

    std::thread thread_;
    std::mutex timer_mtx_; // ensure mutual exclusion between caller thread and timer thread
    std::mutex caller_mtx_; // ensure mutual exclusion between multiple caller threads
    std::condition_variable cv_;
};

#endif //TIMER_H
