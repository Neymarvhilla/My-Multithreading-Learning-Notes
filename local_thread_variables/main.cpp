#include <iostream>
#include <thread>
#include <mutex>
#include <random>

#include <string>
/*
 * LAZY INITIALIZATION
 * - Common pattern in functional programming
 * - A variable is only initialized when it is first used
 * - This is useful when the variable is expensive to construct
 *      - e.g. it sets up a network connection
 * - Can be used in multi-threaded code
 *      - But we need to avoid data races
 *      */

/*
 * Thread-safe Lazy Initialization
 *
 * - Every thread that calls process() locks the mutex
 *      - Locking the mutex blocks every other thread that calls process()
 * - The lock is only needed while ptest is being initialized
 *      - Once ptest has been initialized, locking the mutex is unnecessary
 *      - Causes a loss of performance
 *      */

/*
 * Double-checked locking Algorithm
 * - More efficient version of thread-safe lazy initialization
 * - if ptest is not initialized
 *      - Lock the mutex
 *      - if ptest is not initialized, initialize it
 *      - Unlock the mutex
 *      - Use ptest
 *  - Otherwise
 *      - Use ptest
 *  - ptest is checked twice (why?)
 *  */

/*
 * Double-checked Locking
 *
 *      void process() {
 *          if(!ptest) { // First check of ptest
 *              std::lock_guard<std::mutex> lck_guard(mut);
 *
 *              if(!ptest) // Second check of ptest
 *                  ptest = new Test; // initialize ptest
 *          }
 *          // use ptest
 *          // ....
 *      }
 *
 *      - Statement (1) checks ptest
 *      - Statement (2) locks the mutex
 *      - Another thread could interleave between these operations
 *              - Race condition
 *              */


/*
 * - Thread A checks ptest, which is null
 * - Thread B checks ptest, which is null
 * - Thread B locks the mutex
 * - Thread B initializes ptest
 * - Thread B unlocks the mutex
 * - Thread A locks the mutex
 * - Thread A initializes ptest
 * */

/*
 * Is That Not Enough
 * - There is still a race condition
 *      ptest = new Test;
 * - The initialization of ptest involves several operations
 *      - Allocate enough memory to store a Test object
 *      - Construct a Test object in the memory
 *      - Store the address in ptest
 *
 * - C++ allows these to be performed in a different order, e.g
 *      - Allocate enough memory to store a Test object
 *      - Store the address in ptest
 *      - Construct a Test object in the memory
 *      */
/*
 * Important
 * So new Test is a combined operation:

 - It finds heap space.

 - It constructs the object in-place within that space.

 - It returns the pointer to the constructed object.

This is also why delete must do two things:

 - Call the object's destructor.

 - Free the allocated heap memory.
 */
// lazy initialization (single threaded)


/*
 * Undefined behaviour
 * - Thread A checks ptest and locks the mutex
 * - Thread A allocates the memory and assigns to ptest
 *      ptest = new sizeof(test);
 * - However, it has not yet called the constructor
 * - Thread B checks ptest and it is not null
 * - Thread B does not lock the mutex
 * - Thread B jumps out of the if statement
 * - Thread B calls a member function of an uninitialized object
 *      - Undefined behaviour
 *      */

/*
 * std::call_once
 * - One way to solve this is to use std::call_once()
 *      - A given function is only called once
 *      - It is done in one thread
 *      - The thread cannot be interrupted until the function call completes
 * - We use it with global instance of std::once_flag
 * - We pass the instance and the function to std::call_once()
 * */

/*
 *
 *      // The flag stores synchronization data
 *      std::once_flag ptest_flag;
 *      void process() {
 *          // Pass a callable object which performs the initialization
 *          std::call_once(ptest_flag, [] () {
 *              ptest = new Test;
 *          });
 *          // Use ptest
 *      }
 *
 *      - Threadsafe
 *      - Less overhead than a mutex
 *      */


/*
 * - C++17 defines the order of initialization
 *      - Allocate enough memory to store a Test object
 *      - Construct a Test object in the memory
 *      - Store the address in ptest
 *              ptest = new Test;
 * - Double-checked locking no longer causes a data race
 * */

/*
 * Conclusion
 * - Four ways to do thread-safe lazy initialization
 *      - Naive use of a mutex
 *      - Use std::call_once()
 *      - Double-checked locking with a C++17 compiler or later
 *      - Meyers singleton with static local variable
 *
 * - Recommendations
 *      - Use Meyers singleton, if ptest is not required outside the function
 *      - Otherwise, use std::call_once()
 *      */

/*
 * */

/* */

class Test {
    //...
public:
    void func() {/*..... */}
};

Test *ptest = nullptr; // Variable to be lazily initialized

// our mutex

std::once_flag ptest_flag;

void process() {
    // Pass a callable object which performs the initialization
    std::call_once(ptest_flag, []() {
        ptest = new Test;
    });
    ptest->func();
}
std::mutex mut;
//void process() {
//
//    std::unique_lock<std::mutex> uniq_lck(mut);
//    if(!ptest) {
//        ptest = new Test;
//    }
//    uniq_lck.unlock();
//    ptest->func();
//}

// Thread local random number engine
thread_local std::mt19937 mt;

void func() {
    std::uniform_real_distribution<double> dist(0, 1); // Doubles in the range 0 - 10

    for (int i = 0; i < 10; ++i) {
        std::cout << dist(mt) << ", ";
    }
}


class Singleton {
public:
    static Singleton &instance() {
        std::call_once(init, [] () {
            instance_ = new Singleton();
        });
        return *instance_;
    }

private:
    Singleton() = default; // constructor
    static Singleton *instance_;
    static std::once_flag init;
    Singleton (const Singleton &source) = delete;
    Singleton &operator=(const Singleton &source) = delete;
    Singleton (Singleton &&source) = delete;
    Singleton &operator=(Singleton &&soure) = delete;
};


static int &number1() {
    static int num1 {23};
    return num1;
}

int &number2() {
    static int num2 {24};
    return num2;
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::thread thr1(func);
    thr1.join();

    std::cout << "\nThread 2's random values: \n";
    std::thread thr2(func);
    thr2.join();
    std::cout << "\n";


    double *my_double = new double(12.8);


    double mm  {*my_double * 21.5};
    std::cout << mm << std::endl;
    delete my_double;
    return 0;
}
