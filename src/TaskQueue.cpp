#include "TaskQueue.h"


void TaskQueue::enqueue(const std::function<void()>& task) {
    std::unique_lock lock(mutex);
    tasks.push(task);
    cv.notify_one();
}


std::function<void()> TaskQueue::dequeue(const std::atomic<bool>& stopFlag, const std::atomic<bool>& immediateStopFlag) {
    std::unique_lock lock(mutex);
    cv.wait(lock, [this, &stopFlag, &immediateStopFlag] {return !isEmpty() || stopFlag || immediateStopFlag;});
    if ((stopFlag || immediateStopFlag) && tasks.empty()) {
        return nullptr;
    }
    std::function<void()> task = tasks.front();
    tasks.pop();
    return task;
}


bool TaskQueue::isEmpty() const {
    return tasks.empty();
}


void TaskQueue::notifyAll() {
    cv.notify_all();
}
