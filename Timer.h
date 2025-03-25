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

    bool set_timeout(int seconds);
    bool set_callback(const std::function<void()> &callback);

    void restart();
    void start();
    void stop();

    bool timed_out();

private:
    int timeout_sec{0};
    bool running_{false};
    bool timed_out_{false};
    bool cmd_executed{false}; // start or stop cmd
    bool quit_thread_{false}; // only used by destructor
    std::function<void()> callback_{nullptr};

    std::thread thread_;
    std::mutex timer_mtx_; // ensure mutual exclusion between caller thread and timer thread
    std::mutex caller_mtx_; // ensure mutual exclusion between multiple caller threads
    std::condition_variable cv_;

    bool set_timeout_private(int seconds);
    bool set_callback_private(const std::function<void()> &callback);

    void start_private();
    void stop_private();

    bool timed_out_private();

    void run();
};

#endif //TIMER_H
