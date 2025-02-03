#include <iostream>

#include "threadPool.h"


ThreadPool::ThreadPool(const size_t numThreads, TaskQueue &taskQueue): taskQueue(taskQueue) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ThreadPool::worker, this);
    }
    std::cout << "Thread pool is created!" << std::endl;
    if (!taskQueue.isEmpty()) {
        taskQueue.notifyAll();
    }
}


void ThreadPool::shutdown() {
    stopFlag = true;
    taskQueue.notifyAll();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}


void ThreadPool::stopNow() {
    immediateStopFlag = true;
    taskQueue.notifyAll();
    for (std::thread& worker : workers) {
        worker.join();
    }
}


void ThreadPool::worker() const {
    while(true) {
        if (immediateStopFlag) {
            break;
        }

        std::function<void()> task = taskQueue.dequeue(stopFlag, immediateStopFlag);
        if (!task) {
            if (stopFlag || immediateStopFlag) {
                break;
            }
            continue;
        }
        task();
    }
}


ThreadPool::~ThreadPool() {
    shutdown();
    std::cout << "Job is done!" << std::endl;
}
