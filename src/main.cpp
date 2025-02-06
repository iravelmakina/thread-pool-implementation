#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "ThreadPool.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution dis(6, 12);  // 6–12 seconds
std::atomic running(true);

std::string get_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

void taskGenerator(ThreadPool& pool, const std::atomic<bool>& running) {
    size_t t = 1;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));  // generate tasks every second
        pool.submit([t] {
            printf("Task %lu executed by thread %s\n", t, get_thread_id().c_str());
            std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));  // random task duration
        });
        ++t;
    }
}

int main() {
    ThreadPool pool(4);
    std::thread generator(taskGenerator, std::ref(pool), std::ref(running));

    // run the thread pool for 2 minutes with pause/resume cycles
    for (size_t i = 0; i < 2; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(80));
        std::cout << "Pausing thread pool..." << std::endl;
        pool.pause();

        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "Resuming thread pool..." << std::endl;
        pool.resume();
    }

    std::this_thread::sleep_for(std::chrono::seconds(30));  // additional run time
    std::cout << "Initiating graceful shutdown..." << std::endl;
    pool.shutdown();

    // stop task generator
    running = false;
    if (generator.joinable()) {
        generator.join();
    }
    return 0;
}
