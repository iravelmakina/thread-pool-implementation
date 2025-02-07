#include <iostream>
#include <thread>
#include <random>
#include <atomic>

#include "ThreadPool.h"


std::string get_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

void taskGenerator(ThreadPool& pool, const std::atomic<bool>& running, std::mt19937& gen, std::uniform_int_distribution<>& dis) {
    size_t t = 1;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(3));  // generate tasks every 3 seconds
        pool.submit([t, &gen, &dis] {
            printf("Task %lu executed by thread %s\n", t, get_thread_id().c_str());
            std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));  // random task duration
        });
        ++t;
    }
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(6, 12);

    ThreadPool pool(4);
    std::atomic<bool> running(true);

    std::thread generator(taskGenerator, std::ref(pool), std::ref(running), std::ref(gen), std::ref(dis));

    // run the thread pool for 4,33 minutes with pause/resume cycles
    for (size_t i = 0; i < 4; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        std::cout << "Pausing thread pool..." << std::endl;
        pool.pause();

        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Resuming thread pool..." << std::endl;
        pool.resume();
    }

    std::this_thread::sleep_for(std::chrono::seconds(20));  // additional run time
    std::cout << "Initiating graceful shutdown..." << std::endl;
    pool.shutdown();

    // stop task generator
    running = false;
    if (generator.joinable()) {
        generator.join();
    }
    return 0;
}