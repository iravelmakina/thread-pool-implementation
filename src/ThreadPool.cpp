#include <iostream>

#include "ThreadPool.h"


ThreadPool::ThreadPool(const size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        _workers.emplace_back(&ThreadPool::worker, this);
    }
    std::cout << "Thread pool is created!" << std::endl;
}


void ThreadPool::submit(const std::function<void()>& task) {
    if (_stopFlag || _immediateStopFlag) {
        std::cout << "Task discarded (thread pool is shutting down)." << std::endl;
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
}


void ThreadPool::shutdown() {
    std::cout << "Completing current tasks if there are any...!" << std::endl;
    _stopFlag = true;
    _cv.notify_all();

    for (std::thread& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
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
    _workers.clear();
    std::cout << "Thread pool stopped immediately." << std::endl;
}


bool ThreadPool::runAllowed() const {
    return !_taskQueue.empty() && !_pauseFlag;
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


ThreadPool::~ThreadPool() {
    shutdown();
    std::cout << "Job is done!" << std::endl;
}
