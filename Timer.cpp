//
// Created by dan on 25-3-24.
//
#include <iostream>

#include "Timer.h"

Timer::Timer() {
    thread_ = std::thread(&Timer::run, this);
}

Timer::~Timer() {
    stop(); {
        std::lock_guard lock(timer_mtx_);
        quit_thread_ = true;
        cv_.notify_all();
    }

    if (thread_.joinable())
        thread_.join();
}

bool Timer::set_timeout(int seconds) {
    std::lock_guard lock(caller_mtx_);
    return set_timeout_private(seconds);
}

bool Timer::set_callback(const std::function<void()> &callback) {
    std::lock_guard lock(caller_mtx_);
    return set_callback_private(callback);
}

void Timer::restart() {
    std::lock_guard lock(caller_mtx_);
    stop_private();
    start_private();
}

void Timer::start() {
    std::lock_guard lock(caller_mtx_);
    start_private();
}

void Timer::stop() {
    std::lock_guard lock(caller_mtx_);
    stop_private();
}

bool Timer::timed_out() {
    std::lock_guard lock(caller_mtx_);
    return timed_out_private();
}

bool Timer::set_timeout_private(int seconds) {
    std::lock_guard lock(timer_mtx_);

    if (running_) {
        std::cerr << "Error: Can not set timeout when timer is running!" << std::endl;
        return false;
    }

    timeout_sec = seconds;
    return true;
}

bool Timer::set_callback_private(const std::function<void()> &callback) {
    std::lock_guard lock(timer_mtx_);

    if (running_) {
        std::cerr << "Error: Can not set callback when timer is running!" << std::endl;
        return false;
    }

    callback_ = callback;
    return true;
}

void Timer::start_private() {
    std::unique_lock lock(timer_mtx_);

    if (timeout_sec <= 0 || !callback_) {
        std::cerr << "Error: Invalid timeout or callback function." << std::endl;
        return;
    }

    if (running_) {
        std::cerr << "Warn: Timer already started!" << std::endl;
        return;
    }

    running_ = true;
    timed_out_ = false;
    cmd_executed = false;

    cv_.notify_all();
    cv_.wait(lock, [this] { return cmd_executed; });
}

void Timer::stop_private() {
    std::unique_lock lock(timer_mtx_);

    if (!running_) {
        std::cerr << "Warn: Timer already stopped!" << std::endl;
        return;
    }

    running_ = false;
    cmd_executed = false;

    cv_.notify_all();
    cv_.wait(lock, [this] { return cmd_executed; });
}

bool Timer::timed_out_private() {
    std::lock_guard lock(timer_mtx_);
    return timed_out_;
}

void Timer::run() {
    std::unique_lock lock(timer_mtx_);

wait_start_or_quit: // use goto instead of loop to keep locked util wait()
    cv_.wait(lock, [this] { return running_ || quit_thread_; });
    if (quit_thread_) return;

    cmd_executed = true;
    cv_.notify_all();

    auto timeout = std::chrono::seconds(timeout_sec);
    if (cv_.wait_for(lock, timeout, [this] { return !running_; })) {
        cmd_executed = true;
        cv_.notify_all();
    } else {
        running_ = false;
        timed_out_ = true;
        if (callback_) callback_();
    }

    goto wait_start_or_quit;
}
