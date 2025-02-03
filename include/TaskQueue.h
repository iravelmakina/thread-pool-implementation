#pragma once
#include <queue>
#include "TaskQueue.h"


class TaskQueue {
public:
    void enqueue(const std::function<void()>&);

    std::function<void()> dequeue(const std::atomic<bool>& stopFlag, const std::atomic<bool>& immediateStopFlag);

    bool isEmpty() const;

    void notifyAll();

private:
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable cv;
};



