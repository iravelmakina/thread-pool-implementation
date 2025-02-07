# Thread Pool Implementation

## Overview
This project implements a **Thread Pool** in C++ using `std::thread`, `std::mutex`, and `std::condition_variable`.  
It supports **task execution, pausing, resuming, buffering tasks, and immeadiate/graceful shutdown**.

![threadPoolStart](https://github.com/user-attachments/assets/f7235188-f122-4e21-a3e9-137fd1678fc8)

## Features
- Efficient **Thread Pool** that manages a queue of tasks.
- **Worker threads** execute tasks in parallel.
- **Buffering mechanism** to accumulate tasks before execution.
- **Pause & Resume functionality** to control task execution.
- **Immediate shutdown** ensures current active tasks are abandoned before termination.
- **Graceful shutdown** ensures all tasks complete before termination.
- **Performance tracking** for execution time and queue length statistics.

## Installation and Compilation

### **1. Clone the Repository**
```
git clone https://github.com/iravelmakina/thread-pool-implementation.git
cd thread-pool-implementation
```

### **2. Compile the Program**
#### **Using g++ (Linux/macOS)**
The program requires **C++11 or higher**.
```
g++ -std=c++11 -pthread main.cpp ThreadPool.cpp -o threadpool
```

#### **MSVC (Windows)**
On Windows, compile using Microsoft Visual Studio's cl.exe:
```
cl /std:c++11 /EHsc main.cpp ThreadPool.cpp
```

#### **Using CMake**
```
mkdir -p build && cd build
cmake -DCMAKE_CXX_STANDARD=11 ..
make
```

By default, the project compiles with **C++11 or higher**. You can specify a newer C++ standard if your compiler supports it.

### **3. Run the Program**
```
./threadpool
```

## Usage
The program creates a **thread pool with worker threads** that execute tasks in parallel and a **manager thread that handles buffering process**.  
It also includes **pause/resume cycles** and an **immediate or graceful shutdown process**.

By default:
- Buffering of tasks performs for **45 seconds**.
- Tasks are executed with **random durations (6-12 seconds)**.
- The **pool pauses every 60 seconds** and **resumes after 5 seconds**.
- After **4 pause/resume cycles**, the pool **shuts down gracefully**.
- Performance metrics are **printed at the end**.

## Code Structure
```
thread-pool-implementation/
│── src/
│   ├── main.cpp             # Entry point of the program
│   ├── ThreadPool.cpp       # Implementation of the thread pool
│── include/
│   ├── ThreadPool.h         # ThreadPool class definition
│── CMakeLists.txt           # CMake configuration file
│── README.md                # Documentation
```

# Documentation

## API Reference
### **ThreadPool Methods**
```
| Method | Description |
|--------|-------------|
| `ThreadPool(size_t numThreads)` | Initializes the thread pool with `numThreads` workers. |
| `void submit(const std::function<void()>& task)` | Adds a new task to the queue. |
| `void pause()` | Pauses task execution. |
| `void resume()` | Resumes execution after pausing. |
| `void stopNow()` | Immediately stops all worker threads. |
| `void shutdown()` | Gracefully shuts down the pool, waiting for tasks to finish. |
```

## Example Usage
```cpp
#include "ThreadPool.h"

int main() {
    ThreadPool pool(4);  // Create a thread pool with 4 workers

    // Submit a simple task
    pool.submit([] {
        std::cout << "Task executed by thread " << std::this_thread::get_id() << std::endl;
    });

    pool.shutdown();  // Gracefully shut down the pool
    return 0;
}
```

## Performance Metrics

At the end of execution, the program prints **performance statistics**, including:

- **Total threads created**
- **Total tasks executed**
- **Average waiting time (ms)**
- **Average task execution time (ms)**
- **Average queue length**

![threadPoolEnd](https://github.com/user-attachments/assets/9312c385-f331-417b-ac4e-aa2343cac7bc)

## License

This project is open-source under the **MIT License**.
