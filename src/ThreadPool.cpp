#include <iostream>

#include "ThreadPool.h"


ThreadPool::ThreadPool(const size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        _workers.emplace_back(&ThreadPool::worker, this);
    }
    _executor = std::thread(&ThreadPool::startExecutionCycle, this);
    std::cout << "Thread pool is created!" << std::endl;
}


void ThreadPool::submit(const std::function<void()>& task) {
    if (_stopFlag || _immediateStopFlag) {
        std::cout << "Task discarded (thread pool is shutting down)." << std::endl;
        return;
    }
    if (_executionPhaseFlag) {
        std::cout << "Task discarded (there are still tasks in the queue to perform)." << std::endl;
        return;
    }
    std::lock_guard lock(_mutex);
    _taskQueue.push(task);
}


void ThreadPool::pause() {
    if (!_pauseFlag) {
        _pauseFlag = true;
        _cv.notify_all();
        std::cout << "Thread pool paused." << std::endl;
    }
}


void ThreadPool::resume() {
    if (_pauseFlag) {
        _pauseFlag = false;
        _cv.notify_all();
        std::cout << "Thread pool resumed." << std::endl;
    }
} // cool


void ThreadPool::shutdown() {
    std::cout << "Completing current tasks if there are any...!" << std::endl;
    _stopFlag = true;
    _cv.notify_all();

    for (std::thread& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (_executor.joinable()) {
        _executor.join();
    }
    _workers.clear();
    std::cout << "Thread pool shut down." << std::endl;
}


void ThreadPool::stopNow() {
    std::cout << "Stopping abruptly!" << std::endl;
    _immediateStopFlag = true;
    _cv.notify_all();

    for (std::thread& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (_executor.joinable()) {
        _executor.join();
    }

    _workers.clear();
    std::cout << "Thread pool stopped immediately." << std::endl;
}


bool ThreadPool::runAllowed() const {
    return !_taskQueue.empty() && _executionPhaseFlag && !_pauseFlag;
}


void ThreadPool::worker() {
    while(!_stopFlag && !_immediateStopFlag) {
        std::unique_lock lock(_mutex);
        _cv.wait(lock, [this] { return runAllowed() || _stopFlag || _immediateStopFlag; });
        if (_immediateStopFlag) {
            break;
        }

        if (runAllowed()) {
            std::function<void()> task = _taskQueue.front();
            _taskQueue.pop();
            lock.unlock();
            task();
        }
    }
}


void ThreadPool::startExecutionCycle() {
    while (!_stopFlag && !_immediateStopFlag && !_pauseFlag) {
        std::cout << "Buffering tasks for 45 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(45));

        std::unique_lock lock(_mutex);
        _cv.wait(lock, [this] { return !_pauseFlag || _stopFlag || _immediateStopFlag ; });
        lock.unlock();
        if (_stopFlag || _immediateStopFlag) break;

        std::cout << "Executing buffered tasks..." << std::endl;
        _executionPhaseFlag = true;
        _cv.notify_all();


        while (true) {
            std::unique_lock lock(_mutex);
            if (_taskQueue.empty()) break;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        _executionPhaseFlag = false;
    }
}


ThreadPool::~ThreadPool() {
    shutdown();
    std::cout << "Job is done!" << std::endl;
}
