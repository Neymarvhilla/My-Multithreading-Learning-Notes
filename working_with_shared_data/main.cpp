#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>

#include <queue>


using namespace std::literals;

/*
 *
 * Critical section
 * - A region of code
 * - Must only be executed by one thread at a time
 * - Usually when accessing a shared resource
 *          - Shared data, network connection, hardware device
 * - The thread "enters" the critical section
 *          - It starts executing the code in the critical section
 * - The thread "leaves" the critical section
 *          - It has executed all the code in the critical section
 *
 *
 * - A critical section exists when there is concurrent access to a shared mutable state, and at least one thread mutates it
 * - Even if only one thread is writing, if other threads are reading concurrently, and there is no synchronization, this is data race and therefore a critical section.
 *          */
/*
 * - I think I understand,
 * what defines a critical section is access to a shared mutable state and writing to it,
 * now one thread could do both and another could only read.
 * They don't both have to do both to enter a critical section.
 * the thread that is only reading is entering a critical section because another thread is writing and this could happen concurrently.
 * so the the mutex locks in only one thread at a time
 * */

/*
 * Locking Protocol
 * - Only one thread can enter the critical section
 *      - All the other threads are "locked out"
 *      - Only this thread can execute the code in the critical section
 *
 * - The thread leaves the critical section
 *      - One of the other threads can now enter it
 *      */

/*
 * Mutex
 * - MUTual EXclusion object
 * - We can use a mutex to implement locking
 * - A mutex has two states
 *      - "locked"
 *      - "unlocked"
 *      */

/*
 * Mutual Exclusion
 * - Exclusion
 *      - The mutex is used to exclude threads from the critical section
 * - Mutual
 *      - The threads agree to respect the mutex
 * - Locking
 *      - if the mutex is unlocked, a thread can enter the critical section
 *      - if the mutex is locked, no thread can enter until it becomes unlocked
 *      - A thread locks the mutex when it enters the critical section
 *      - A thread unlocks the mutex when it leaves the critical section
 *
 * - The mutex essentially acts as a gate-keeper to the critical section
 *      */

/*
 * Thread Synchronization with Mutex
 * - Some threads A, B, C, ...... wish to enter a critical section
 *          - Thread A locks the mutex
 *          - Thread A enters the critical section
 *          - Thread B, C, ... wait until they can lock the mutex
 *          - Thread A leaves the critical section
 *          - Thread A unlocks the mutex
 *          - One of threads B,C, ..... can now lock the mutex and enter the critical section
 *          */

/*
 * Thread Synchronization with Mutex
 * - The threads are synchronized
 *      - They cannot interleave when they execute in the critical section
 *      - There is no data race
 *
 * - Unlocking a mutex "publishes" any changes
 *      - Thread A modifies shared data
 *      - The new value is now available to other threads
 *      - it will be seen by the next thread which accesses the shared data
 *      */

/*
 * Acquire Release Semantics
 * - A thread locks a mutex
 *      it acquires exclusive access to the critical section
 * - The thread unlocks the mutex
 *      - it releases exclusive access to the critical section
 *      - it also releases the result of any modifications
 *      - the next thread that locks the mutex will acquire these results
 * - These acquire release semantics impose ordering on the threads
 *      - There is no data race
 *      - The shared data is always in a consistent state
 * */

/*
 * std::mutex Class
 * - The C++ Standard Library provides an std::mutex class
 *      - Defined in <mutex>
 * - A mutex object must be visible in all task functions which uses it
 * - It must also be defined outside the task functions
 *      - Global/static variable with global task function
 *      - Class data member with task function
 *      - Variable captured by reference with lambda expressions
 *      */

/*
 * std::mutex interface
 * - Three main member functions:
 * - lock()
 *      - Tries to lock the mutex
 *      - if not successful, waits until it locks the mutex
 * - try_lock()
 *      - Tries to lock the mutex
 *      - Returns immediately if not successful
 * - unlock()
 *      - Releases the lock on the mutex
 *      */

/*
 * Rewrite using std::mutex
 *
 *      // Global mutex object
 *      std::mutex task_mutex;
 *      void task(const std::string &str)
 *      {
 *          for (int i {0}; i < 5; ++i) {
 *              // Lock the mutex before the critical section
 *              task_mutex.lock();
 *
 *              // The thread has exclusive access to the critical section
 *              std::cout << str[0] << str[1] << str[2] << std::endl;
 *
 *              // unlock the mutex after the critical section
 *              task_mutex.unlock();
 *          }
 *      }
 *      */

/*
 * Output
 * - The output is no longer scrambled up
 * - The accesses to the critical section are synchronized
 *      - This prevents the threads from interfering with each other
 *      */

/*
 * std::mutex::try_lock()
 * - try_lock() returns immediately
 *      - Returns true if it locked the mutex
 *      - Returns false if it could  not lock the mutex
 *      */

/*
 * Usually called in a loop
 *          // keep trying to get the lock
 *          while (!the_mutex.try_lock()) {
 *
 *              // could not lock the mutex
 *              // Try again later
 *              std::this_thread::sleep_for(100ms);
 *          }
 *          // Finally locked the mutex
 *          // Can now execute in the critical section
 *          */

/*
 * Internal Synchronization
 * - Multiple threads accessing the same memory location
 *      - With modification
 *      - Must be synchronized to prevent a data race
 *
 *
 * - C++ STL containers need to be externally synchronized
 *      - e.g. by locking a mutex before calling a member function
 *
 * - Our own types can provide internal synchronization
 *      - An std::mutex as a data member
 *      - The member functions lock the mutex before accessing the class's data
 *      - They unlock the mutex after accessing the class's data
 *      */

/*
 * Wrapper for std::vector
 * - std::vector acts as a memory location
 *      - We may need to lock a mutex before calling its member functions
 *
 * - Alternatively, we could write an internally synchronized wrapper for it
 * - A class which
 *      - Has an std::vector data member
 *      - Has an std::mutex data member
 *      - Member functions which lock the mutex before accessing the std::vector
 *      - Then unlock the mutex after accessing it
 *
 * - An internally synchronized class
 * */

/* */

// Global mutex object
std::mutex task_mutex;
void task(const std::string &str) {
    for (int i {0}; i < 5; ++i) {
        // Lock the mutex before the critical section
        task_mutex.lock();

        // Start of critical section
        std::cout << str[0] << str[1] << str[2] <<std::endl;
        // End of critical section

        // Unlock the mutex after the critical section
        task_mutex.unlock();
    }
}
std::mutex the_mutex;
void task1() {
    std::cout << "Task1 trying to lock the mutex" << std::endl;
    the_mutex.lock();
    std::cout << "Task1 has locked the mutex" << std::endl;
    std::this_thread::sleep_for(500ms);
    the_mutex.unlock();
}

void task2() {
    std::this_thread::sleep_for(100ms);
    std::cout << "Task2 trying to lock the mutex" << std::endl;
    while (!the_mutex.try_lock()) {
        std::cout << "Task2 could not lock the mutex" << std::endl;
        std::this_thread::sleep_for(100ms);
    }
    std::cout << "Task2 has locked the mutex" << std::endl;
    the_mutex.unlock();
}


// Thread safe queue
class ThreadSafeQueue {
private:
    std::queue<int> q;
    std::mutex m;
public:
    void push(int val) {
        std::lock_guard<std::mutex> lock(m);
        q.push(val);
    }

    bool try_pop(int &val) {
        if (q.empty()) {
            return false;
        }
        std::lock_guard<std::mutex> lock(m);
        val = q.front();
        q.pop(); return true;
    }
};

class ThreadSafeVector{
private:
    std::mutex m;
    std::vector<int> vec;

public:

    void push_back(const int &val) {
        std::lock_guard<std::mutex> lock(m);
        // start of the critical section
        vec.push_back(val);
    }

    void print()  {
        std::lock_guard<std::mutex> lock(m);
        // start of critical section
        for (int i{0}; i < vec.size(); ++i) {
            std::cout << vec[i] << ", " ;
        }

    }
};

void func(ThreadSafeVector &vec) {
    for (int i{0}; i < 5; ++i) {
        vec.push_back(i);
        std::this_thread::sleep_for(50ms);
        vec.print();

    }
}

int main() {
    //std::cout << "Hello, World!" << std::endl;

//    std::thread thread1(task, "abc");
//    std::thread thread2(task, "def");
//    std::thread thread3(task, "xyz");
//
//    thread1.join(); thread2.join(); thread3.join();


//    std::thread path1(task1); std::thread path2(task2);
//    path1.join(); path2.join();

    ThreadSafeVector my_vec;

    std::thread vp1(func, std::ref(my_vec));
    std::thread vp2(func, std::ref(my_vec));
    std::thread vp3(func, std::ref(my_vec));

    vp1.join(); vp2.join(); vp3.join();
    return 0;
}
