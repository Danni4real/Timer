//
// Created by dan on 25-3-24.
//
#include <iostream>

#include "Timer.h"
#include "ThreadLog.h"

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

bool Timer::set_loop_times(int times) {
    std::lock_guard lock(caller_mtx_);
    LOG_CALL(times);

    return set_loop_times_private(times);
}

bool Timer::set_timeout(int seconds) {
    std::lock_guard lock(caller_mtx_);
    LOG_CALL(seconds);

    return set_timeout_private(seconds);
}

bool Timer::set_each_timing_start_callback(const std::function<void()> &callback) {
    std::lock_guard lock(caller_mtx_);
    // LOG_CALL_0();

    return set_each_timing_start_callback_private(callback);
}

bool Timer::set_each_timeout_callback(const std::function<void()> &callback) {
    std::lock_guard lock(caller_mtx_);
    LOG_CALL_0();

    return set_each_timeout_callback_private(callback);
}

bool Timer::set_final_timeout_callback(const std::function<void()> &callback) {
    std::lock_guard lock(caller_mtx_);
    LOG_CALL_0();

    return set_final_timeout_callback_private(callback);
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

bool Timer::stopped() {
    std::lock_guard lock(caller_mtx_);
    return stopped_private();
}

bool Timer::set_timeout_private(int seconds) {
    std::lock_guard lock(timer_mtx_);
    LOG_CALL(seconds);

    if (running_) {
        LOG_ERROR("Error: Can not set timeout when timer is running!");
        return false;
    }

    timeout_sec_ = seconds;
    return true;
}

bool Timer::set_loop_times_private(int times) {
    std::lock_guard lock(timer_mtx_);
    LOG_CALL(times);

    if (running_) {
        LOG_ERROR("Error: Can not set loop times when timer is running!");
        return false;
    }

    loop_times_ = times;
    return true;
}

bool Timer::set_each_timing_start_callback_private(const std::function<void()> &callback) {
    std::lock_guard lock(timer_mtx_);
    // LOG_CALL_0();

    if (running_) {
        LOG_ERROR("Error: Can not set callback when timer is running!");
        return false;
    }

    each_timing_start_callback_ = callback;
    return true;
}

bool Timer::set_each_timeout_callback_private(const std::function<void()> &callback) {
    std::lock_guard lock(timer_mtx_);
    LOG_CALL_0();

    if (running_) {
        LOG_ERROR("Error: Can not set callback when timer is running!");
        return false;
    }

    each_timeout_callback_ = callback;
    return true;
}

bool Timer::set_final_timeout_callback_private(const std::function<void()> &callback) {
    std::lock_guard lock(timer_mtx_);
    LOG_CALL_0();

    if (running_) {
        LOG_ERROR("Error: Can not set callback when timer is running!");
        return false;
    }

    final_timeout_callback_ = callback;
    return true;
}

void Timer::start_private() {
    std::unique_lock lock(timer_mtx_);

    if (timeout_sec_ <= 0) {
        LOG_ERROR("Invalid timeout seconds: %d", timeout_sec_);
        return;
    }

    if (running_) return;

    running_ = true;
    cmd_executed_ = false;

    cv_.notify_all();
    cv_.wait(lock, [this] { return cmd_executed_; });
}

void Timer::stop_private() {
    std::unique_lock lock(timer_mtx_);

    if (!running_) return;

    running_ = false;
    cmd_executed_ = false;

    cv_.notify_all();
    cv_.wait(lock, [this] { return cmd_executed_; });
}

bool Timer::stopped_private() {
    std::lock_guard lock(timer_mtx_);
    return !running_;
}

void Timer::run() {
    std::unique_lock lock(timer_mtx_);

wait_start_or_quit: // use goto instead of loop to keep locked util wait()
    cv_.wait(lock, [this] { return running_ || quit_thread_; });
    if (quit_thread_) return;

    cmd_executed_ = true;
    cv_.notify_all();

    auto timeout = std::chrono::seconds(timeout_sec_);

    for (int i = 0; i < loop_times_; i++) {
        if (each_timing_start_callback_)
            each_timing_start_callback_();

        if (cv_.wait_for(lock, timeout, [this] { return !running_; })) {
            cmd_executed_ = true;
            cv_.notify_all();
            goto wait_start_or_quit;
        }

        if (each_timeout_callback_)
            each_timeout_callback_();
    }

    running_ = false;

    if (final_timeout_callback_)
        final_timeout_callback_();

    goto wait_start_or_quit;
}
