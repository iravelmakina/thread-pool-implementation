#include <iostream>

#include "ThreadPool.h"


ThreadPool::ThreadPool(const size_t numThreads) {
    std::cout << "Thread pool is created!" << std::endl;
    for (size_t i = 0; i < numThreads; ++i) {
        _workers.emplace_back(&ThreadPool::worker, this);
    }
}


void ThreadPool::submit(const std::function<void()>& task) {
    if (_stopFlag || _immediateStopFlag || _pauseFlag) {
        std::cout << "Task discarded (thread pool is shutting down or paused)." << std::endl;
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
    _pauseFlag = true;
    _cv.notify_all();
    std::cout << "Thread pool paused." << std::endl;

}


void ThreadPool::resume() {
    _pauseFlag = false;
    _cv.notify_all();
    std::cout << "Thread pool resumed." << std::endl;
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

    std::cout << "Thread pool stopped immediately." << std::endl;
}



void ThreadPool::worker() {
    while(true) {
        if (_immediateStopFlag || _stopFlag) {
            break;
        }
        if (!_executionPhaseFlag && !_pauseFlag) {
            std::cout << "Buffering tasks for 45 seconds..." << std::endl;
            auto start = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - start < std::chrono::seconds(8)) {
                if (_pauseFlag || _stopFlag || _immediateStopFlag) {
                    break;
                }
            }
            _executionPhaseFlag = true;
            _cv.notify_all();
        }

        std::unique_lock lock(_mutex);
        _cv.wait(lock, [this] {  return !_taskQueue.empty() ||_immediateStopFlag || _stopFlag || _pauseFlag; });

        while (!_taskQueue.empty() && !_immediateStopFlag && !_pauseFlag) {
            std::function<void()> task = _taskQueue.front();
            _taskQueue.pop();
            _cv.notify_all();
            lock.unlock();
            task();
            lock.lock();
        }
        if (_taskQueue.empty()) {
            _executionPhaseFlag = false;
        }
    }
}


ThreadPool::~ThreadPool() {
    if (!_stopFlag && !_immediateStopFlag) {
        shutdown();
    }
}
