#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>
#include <condition_variable>
using namespace std::literals;
/*
 * Before we begin, some notes from the legendary Bjarne Stroustrup
 *
 * - Resource Management
 *
 * - A resource is anything you must acquire and later release
 *      - Explicitly or Implicitly
 *      - Examples : memory strings, locks, file handles, sockets, thread handles, shaders
 *      - Not just memory
 * - Avoid resource leaks
 *      - Avoid manual release
 * - Every object has a handle
 *      - Responsible for access and release
 * - Every resource handle is rooted in a scope
 *      - And handles can be moved from scope to scope
 *      - Minimize resource retention
 *      */

/*
 * - Control of lifetime
 *
 * - Necessary for simple and efficient resource management
 *      - Construction
 *          - Before first use establish the invariant(if any)
 *          - Constructor
 *      - Destruction
 *          - After last use release every resource (if any)
 *          - destructor
 *      - Copy
 *          - Copy: a=b impies a==b (regular types)
 *          - Copy constructor: X(const X&)
 *          - Copy assignment: X::operator=(const X&)
 *      - Move
 *          - Move resources between scopes
 *          - Move constructor: X(X &&)
 *          - Move assignment: X::operator=(X &&)
 *          */

/*
 *
 * - Object-Oriented Programming
 *
 * - "C++ is not an OOP language; it's a language that supports OO among other programming styles" - Bjarne Stroustrup from the early 1980s onwards
 *          - Inheritance from 1979
 *          - Virtual functions from 1983
 *          - Encapsulation(private/public) from 1983
 *          - Multiple inheritance and abstract classes from 1987
 *          - All but abstract classes had been on my wanted list from the start
 *
 * - Many(most) classes don;t fit into a class hierarchy
 * - Most classes that need a hierarchy need only single inheritance
 * - Multiple inheritance interfaces (abstract classes) is necessary for a statistically typed language
 * - There can be no universal base class needed by all and not imposing overheads
 * - Virtual functions calls must be efficient
 *          - Index lookup, not hash-table lookup
 * - The C++ inheritance model has been proven theoretically sound
 *          - 20 years after it was put into wide use
 *          */

/*
 *
 * Compile time computation
 *
 * - Moving computation from run time to compile time
 *          - A key to many performance improvements
 *          - From the earliest days
 *              - Name lookup in inheritance
 *              - Inlining
 * - Saves us from having to handle many run-time errors
 *          - From the earliest days
 *              - C++ was meant to be run where there were no programmers or operators
 * - constexpr puts compile-time evaluation into the hands of the programmer
 *          - From 2010
 *              - At the time when we proposed it, it was deemed "useless and impossible to implement" !*/

/*
 * Compile-time programming
 * - Any sufficiently simple function can be evaluated at compile time
 *          - constexpr: can be evaluated at compile time
 *          - consteval: must be evaluated at compile time
 *
 * - consteval and constexpr functions and concepts
 *          - Cannot have side effects
 *          - Cannot access non-local data
 *          - Cannot have undefined behaviour
 *          - Can use extensive facilities, incl. much of the standard library
 *          - Usually gives much smaller and faster code
 *          - No run-time error handling needed
 *          - Type-rich programming at compile time
 *          */

/*
 * - Error handling
 *
 * - Have an articulated policy for handling errors
 *
 * - Error code and tests
 *          - For failures to do what's required that are common and can be handled locally
 *          - We often forget to test for errors, yielding wrong results
 *          - Doesn't work for constructors and operators, e.g., Matrix x = y+z;
 *          - The alternative is slow and ugly try.....catch
 *
 *
 * - Exceptions
 *          - For failures that are rare and cannot be handled locally
 *          - Propagate errors up the call chain to a handler
 *          - Failure to check for an exception gives termination, not wrong results
 *          - Doesn't work with pointers used as resource handles - consistently use scoped resource handles(RAII)
 *          - The alternative is expensive "error code hell"
 *          */

/*
 * Coordination Between Workers
 *
 * - Problem
 *      - Worker A is writing a document
 *      - Worker A needs an Image that Worker B is producing
 *      - Worker A cannot continue until Worker B has finished
 *      */

/*
 * - One solution is to introduce a manager
 *      - The manager coordinates the two workers
 *      */

/*
 * - Solution
 * - Worker B tells the manager when they have finished
 * - The manager tells Worker A to resume work
 *      - B is working
 *      - A is waiting
 *      - B finishes their work
 *      - B tells manager
 *      - Manager tells A to resume
 *      - A resumes work
 *      */

/*
 * - Similar situations arise with threads
 * - e.g. A program which is performing a download
 *      - One thread fetches the data over the network
 *      - Another thread displays a progress bar
 *      - A third thread will process the data when the download is complete
 *      */

/*
 * Thread Coordination Practical
 * - Communication Between Threads
 * - The threads run concurrently
 *      - The data fetching threads runs continually
 *      - The progress bar thread waits for information
 *      - The processor thread waits until all the data has been received
 *      */

/*
 * When the download is complete
 * - The fetching thread terminates
 * - The progress bar thread terminates
 * - The processor thread runs
 * */

/*
 * Data Sharing Between Threads
 * - The downloaded data is shared by all three threads
 *      - The data fetching thread appends to it
 *      - The progress bar thread calculates its size
 *      - The processor thread uses the data
 *
 * - Potential Data Race
 *      - Multiple threads
 *      - Modification
 *
 *      */

/*
 * - We will use two bools to coordinate the threads
 * - "process" flag
 *      - The fetching thread sets this when it has new data
 *      - The progress bar thread checks this flag
 * - "completed" flag
 *      - the fetching thread sets this when it finishes
 *      - the other two threads check this flag
 * - Potential Data Race
 *      - Multiple threads
 *      - Modification
 * - Use mutexes
 * */

/*
 * Hot Loop
 * - We need to lock the mutex while checking a bool
 *      // In progress bar task function
 *      std::lock_guard data_lck(data_mutex);
 *      while(!update_progress) {}
 *
 * - The thread will run flat out
 *      - The processor core will run at 100%
 *      - Other threads cannot do useful work
 *      - Uses a lot of electricity
 *
 * - The fetcher thread cannot set the flag
 * */

/*
 * Hot Loop Avoidance
 * - To avoid this, unlock the mutex inside the loop
 *          std::unique_lock<std::mutex> data_lck(data_mutex);
 *
 *          while (!update_progress) {
 *              data_lck.unlock();
 *              std::this_thread::sleep_for(10ms);
 *              data_lck.lock();
 *          }
 *
 * - Sleeping allows other threads to use the core
 * - The fetcher thread can set the flag
 * */


/*
 * Implementation with Mutex
 * - This is not ideal
 *      - Too many loops
 *      - Too much explicit locking and unlocking
 *      - How do we choose the sleep duration?
 *
 * - Better solution
 *      - Thread A indicates that it is waiting for something
 *      - Thread B does the "something"
 *      - Thread A is woken up and resumes
 *      */

/*
 * Condition Variable Motivation
 * - Suppose we have two threads
 *      - "Writer" thread modifies a shared string
 *      - "Reader" thread uses the modified string
 * - The two threads need to be coordinated
 * - We must also avoid a data race
 * */

/*
 * - One solution is to introduce a condition variable
 * */

/*
 * - Condition Variable Overview
 *
 * - Scenario
 *      - Thread A tells the condition variable it is waiting
 *      - Thread B notifies the condition variable when it updates the string
 *      - The condition variable wakes thread A up
 *      - Thread A then uses the string
 *      */

/*
 * Condition Variable and Thread Safety
 * - We use a mutex to protect critical sections
 * - The condition variable also uses the same mutex
 *      - Thread coordination
 *      - No data race
 *      */

/*
 * std::condition_variable
 *
 * - Defined in <condition_variable>
 * - wait()
 *      - Takes an argument of type std::unique_lock
 *      - It unlocks its argument and blocks the thread until a notification is received
 *
 * - wait_for() and wait_until()
 *      - Re-lock their argument if a notification is not received in time
 * - notify_one()
 *      - Wake up one of the waiting threads
 *      - The scheduler decides which thread is woken up
 * - notify_all()
 *      - Wake up all the waiting threads*/
/*
 * Condition Variable Scenario
 * - Thread A locks the mutex
 *      - It call the condition variable's wait() member function
 *      - The condition variable unlocks the mutex
 *      - The condition variable blocks this thread
 *
 * - Thread B locks the mutex
 *      - It modifies the string and unlocks the mutex
 *      - It calls notify_one()
 *
 * - The condition variable wakes thread A up
 *      - The wait() call returns with the mutex locked
 *      - Thread A resumes execution and uses the string
 *      */

/*
 * Reader thread
 *
 *      // waiting thread
 *      void reader()
 *      {
 *          // Lock the mutex
 *          std::unique_lock<std::mutex> uniq_lck(mut);
 *
 *          // Call wait() on the condition variable
 *          // Unlocks the mutex and makes this thread sleep
 *          cond_var.wait(uniq_lck);
 *
 *          // The condition variable wakes this thread up locks the mutex
 *
 *          // Use the shared data
 *          // .....
 *      }
 *      */

/*
 * Writer thread
 *
 *      // Notifying thread
 *      void writer()
 *      {
 *          {
 *              // Lock the mutex
 *              std::lock_guard<std::mutex> lck_guard(mut);
 *
 *              // Modify the shared data
 *              sdata = "Populated";
 *          } // Release the lock
 *
 *          // Notify the condition variable
 *          cond_var.notify_one();
 *      }
 *          */

/*
 * std::condition_variable_any
 *
 * - std::condition_variable only works with std::mutex
 *      - Does not work with std::timed_mutex
 * - There is also std::condition_variable_any
 *      - Works with any mutex-like object
 *      - Including our own types
 *      - May have more overhead than std::condition_variable
 *      */

/*
 *  Condition Variables with Predicate
 *  */

/*
 * - Lost Wakeup
 *
 * - The example in the previous lecture has a problem
 * - wait() will block until the condition variable is notified
 * - If the writer calls notify() before the reader calls wait()
 *          - The condition variable is notified when there are no threads waiting
 *          - The reader will never be woken
 *          - The reader could be blocked forever
 * - This is known as a "lost wakeup"
 * */

/*
 * Spurious Wakeup
 * - Occasionally, the reader will be "spuriously" woken up
 *          - The reader thread has called wait()
 *          - The writing thread has not called notify()
 *          - The condition wakes the reader up anyway
 *
 * - This is due to the way that std::condition_variable is implemented
 *          - Avoiding spurious wakeups adds too much overhead
 * - Fortunately, there is a way to solve both spurious and lost wakeups
 *          */

/*
 * wait() with predicate
 * - wait() takes an optional second argument
 *      - A predicate
 * - Typically, the predicate checks a shared bool
 *      - The bool is initialized to false
 *      - It is set to true when the writer sends the notification
 *
 * - The reader thread will call this predicate
 * - It will only call wait() if the predicate returns false
 *      - Also available with wait_for() and wait_until()
 *      */

/*
 * Using wait() with predicate
 *
 * - Add a shared boolean flag, initialized to false
 * - In the wait() call, provide a callable object that checks the flag
 *
 *          // bool flag for predicate
 *          bool condition = false;
 *
 *          // waiting thread
 *          void reader()
 *          {
 *              // lock the mutex
 *              std::unique_lock<std::mutex> uniq_lck(mut);
 *
 *              // Lambda predicate that checks the flag
 *              cond_var.wait(uniq_lck, [] {return condition;});
 *
 *              // ...
 *              */

/* Using wait() with Predicate
 *
 * - In the writer thread, set the flag to true
 *
 *              {
 *                  std::lock_guard<std::mutex> lck_guard(mut);
 *                  sdata = "Populated";
 *
 *                  // Set the flag
 *                  condition = true;
 *              }
 *
 *              // Notify the condition variable
 *              cv.notify_one();
 *          }
 *          */

/*
 * Lost Wakeup Avoidance
 *
 * - The writer notifies the condition variable
 * - The reader thread locks the mutex
 * - The reader thread calls the predicate
 * - If the predicate returns true
 *      - Lost wakeup scenario -  the writer has already sent a notification
 *      - The reader thread continues, with the mutex locked
 * - If the predicate returns false
 *      - Normal scenario
 *      - The reader thread calls wait() again
 *      */

/*
 * Spurious Wakeup Avoidance
 * - The writer notifies the condition variable (or not)
 * - The reader thread locks the mutex
 * - The reader thread calls the predicate
 * - if the predicate returns true
 *      - Genuine wakeup - the writing really has sent a notification
 *      - The reader thread continues, with the mutex locked
 * - If the predicate returns false
 *      - Spurious wakeup scenario - the writing thread has not sent a notification
 *      - The reader thread calls wait() again
 *      */

/*
 * Multiple Threads
 * - Condition variables are particularly useful here
 *      - Multiple threads are waiting for the same event
 * - notify_all()
 *      - The condition variable wakes up all the threads which have called wait()
 *      - The threads could wake up in any order
 *      - All the reader threads process the data
 *
 * - notify_one()
 *      - Only one of the threads which called wait() will be woken up
 *      - The other waiting threads will remain blocked
 *      - A different reader thread processes the data each time
 *      */

/*
 * Program Performing a Download
 * - We will use two condition variables
 * - "data_cv*"
 *      - The fetching thread notifies this when it has new data
 *      - The progress bar waits on it and updates itself
 *
 * - "completed_cv"
 *      - The fetching thread notifies this when the download completes
 *      - The progress bar waits on it and exits
 *      - The processing thread waits on it and processes the data
 *
 * - We use predicates with the condition variables
 *      - Avoid lost and spurious wake-ups
 *      */

/*
 * Progress Bar
 *
 *  - Implemented as a loop
 *      - Wait on data_cv
 *      - Update progress
 *      - Wait on completed_cv
 *      - if the download is compatible, exit
 *
 * - Use blocking wait() on data_cv
 * - Use non-blocking wait_for() on completed_cv
 * */

// Simulation of a program which performs a download
// One thread fetches the data
// Another Thread displays a progress bar
// A third thread processes the data when the download is complete
// Implemented using bools to communicate between the threads


// shared variable for the data being fetched
std::string sdata;

// Flags for thread communication
bool update_progress = false;
bool completed = false;

// Mutexes to protect the shared variables
std::mutex data_mutex;
std::mutex completed_mutex;

// Mutex to protect critical sections
std::mutex mut;

// The condition variable
std::condition_variable cond_var;
// variables for assignment
std::mutex assignment_mutex;

std::string assignment_data {"Neso"};
bool updated_assignment_data = false;

// Thread entry points
void reader_assignment()
{
    std::cout << "Reader thread locking the mutex" << std::endl;
    std::unique_lock<std::mutex> assignment_lock(assignment_mutex);
    while (!updated_assignment_data) {
        assignment_lock.unlock();
        std::this_thread::sleep_for(10ms);
        assignment_lock.lock();
    }
    std::cout << "The new Assignment Data is " << assignment_data << std::endl;
    std::cout << "Reader thread is unlocking the mutex" << std::endl;
}
void writer_assignment()
{
    {
        std::cout << "Writer thread locking mutex" << std::endl;
        std::lock_guard<std::mutex> assignment_lock_guard(assignment_mutex);
        std::cout << "Writer thread is modifying assignment data" << std::endl;
        assignment_data = "Neymar";
        std::cout << "Writer thread is unlocking the mutex" << std::endl;
    }
    updated_assignment_data = true;
}
// Waiting thread
void reader()
{
    // Lock the mutex
    std::cout << "Reader thread locking the mutex" << std::endl;
    std::unique_lock<std::mutex> uniq_lck(mut);
    std::cout << "Reader thread has locked the mutex" << std::endl;

    // call wait()
    // this will unlock the mutex and make this thread
    // sleep until the condition variable wakes up
    std::cout << "Reader thread sleeping...." << std::endl;
    cond_var.wait(uniq_lck);

    // The condition variable has woken this thread up
    // and locked the mutex
    std::cout << "Reader thread wakes up" << std::endl;

    // Display the new value of the string
    std::cout << "Data is \"" << sdata << "\" " << std::endl;
}

// Notifying thread
void writer()
{
    {
        // Lock the mutex
        std::cout << "Writer thread locking mutex" << std::endl;

        // Lock the mutex
        // This will not be explicitly unlocked
        // std::lock_guard is sufficient
        std::lock_guard<std::mutex> lck_guard(mut);
        std::cout << "Writer thread has locked the mutex" << std::endl;

        // pretend to be busy.....
        std::this_thread::sleep_for(2s);

        // Modify the string
        std::cout << "Writer thread modifying data...." << std::endl;
        sdata = "populated";
    }
    // Notify the condition variable
    std::cout << "Writer thread sends notification" << std::endl;
    cond_var.notify_one();
}
// Data Fetching Thread
// our condition variable

//void fetch_data() {
//    for (int i = 0; i < 5; ++i) {
//
//        std::cout << "Fetcher thread waiting for data..." << std::endl;
//        std::this_thread::sleep_for(2s);
//
//        // Update sdata, then notify the progress bar thread
//        // lock the critical section
//        std::lock_guard<std::mutex> data_lck(data_mutex);
//        // we can write now
//        sdata += "Block" + std::to_string(i+1);
//        std::cout << "sdata: " << sdata << std::endl;
//        // set the boolean flag to true
//        update_progress = true;
//        // notify the condition variable
//        download_condition_variable.notify_one();
//    }
//    std::cout << "Fetch sdata has ended" << std::endl;
//    // Tell the progress bar thread to exit
//    // and wake up the processing thread
//    std::lock_guard<std::mutex> completed_lck(completed_mutex);
//    // set the boolean flag to true
//    completed = true;
//    // notify the condition variable
//    download_condition_variable.notify_all();
//}
//// progress bar thread
//void progress_bar() {
//    size_t len = 0;
//
//    while (true) {
//        std::cout << "Progress bar thread waiting for data...." << std::endl;
//
//        // Wait until there is some new data to display
//        std::unique_lock<std::mutex> data_lck(data_mutex);
//
//
//        while(!update_progress) {
//            download_condition_variable.wait(data_lck, [] {return update_progress;});
//            //data_lck.unlock();
//            //std::this_thread::sleep_for(10ms);
//            //data_lck.lock();
//        }
//
//        // wake up and use the new value
//        len = sdata.size();
//
//        // Set the flag back to false
//        update_progress = false;
//        //data_lck.unlock();
//        std::cout << "Received " << len << " bytes so far" << std::endl;
//
//        // Terminate when the download has finished
//        download_condition_variable.wait(data_lck, [] {return completed;});
//        break;
////        std::lock_guard<std::mutex> completed_lck(completed_mutex);
////        if (completed) {
////            std::cout << "Progress bar thread has ended" << std::endl;
////            break;
////        }
//    }
//}
//
//
//void process_data() {
//    std::cout << "Processing thread waiting for data......." << std::endl;
//
//    // Wait until the download is complete
//
//    std::unique_lock<std::mutex> completed_lck(completed_mutex); // Acquire the lock
//
//    while(!completed) {
//        download_condition_variable.wait(completed_lck, [] {return completed;});
//        //completed_lck.unlock();
//        //std::this_thread::sleep_for(10ms);
//        //completed_lck.lock();
//    }
//    //completed_lck.unlock();
//
//    //std::lock_guard<std::mutex> data_lck(data_mutex);
//    std::cout << "Processing sdata: " << sdata << std::endl;
//}

// with predicate
// the shared data
std::string shared_data;

// Mutex to protect critical sections
std::mutex shared_data_mut;

// The condition Variable
std::condition_variable cond_variable;

// bool flag for predicate
bool condition = false;

// waiting thread

void reader_with_condition_variable()
{
    // Lock the mutex
    std::cout << "Reader thread locking mutex" << std::endl;
    std::unique_lock<std::mutex> uniq_lck(shared_data_mut);
    std::cout << "Reader thread has locked the mutex" << std::endl;

    // Call wait()
    // This will unlock the mutex and make this thread
    // sleep until the condition variable wakes up
    std::cout << "Reader thread sleeping....." << std::endl;

    // Lambda predicate that checks the flag
    cond_var.wait(uniq_lck, [] {return condition;});

    // The condition variable has woken this thread up
    // and locked the mutex
    std::cout << "Reader thread wakes up" << std::endl;

    // Display the new value of the string
    std::cout << "Data is \"" << shared_data << "\"" << std::endl;
    std::cout << "Reader thread unlocks the mutex" << std::endl;
}

// Notifying thread
void writer_with_condition_variable()
{
    {
        // Lock the mutex
        std::cout << "Writer thread locking the mutex" << std::endl;

        // Lock the mutex
        // This will not be explicitly unlocked
        // std::lock_guard is sufficient
        std::lock_guard<std::mutex> lck_guard(shared_data_mut);
        std::cout << "Writer thread has locked the mutex" << std::endl;

        // Pretend to be busy....
        std::this_thread::sleep_for(2s);

        // Modify the string
        std::cout << "Writer thread modifying data....." << std::endl;
        shared_data = "Populated";

        // Set the flag
        condition = true;
        std::cout << "Writer thread unlocks the mutex" << std::endl;
    }
    std::cout << "Writer thread sends notification" << std::endl;
    //cond_variable.notify_one();
    cond_variable.notify_all();
}

std::string downloaded_data;
std::mutex data_lock;

std::mutex print_mut;
std::condition_variable download_condition_variable;
bool string_updated = false;
bool download_complete = false;

void fetch_data()
{
    // I want to protect std::cout, multiple threads could be displaying to the console at once
    {
        std::lock_guard<std::mutex> print(print_mut);
        std::cout << "Fetching Data..........." << std::endl;
    }


    // Our loop to simulate the fetching/downloading state

    for (int i = 0; i < 5; ++i) {

        // protect std::cout
        {
            std::lock_guard<std::mutex> print(print_mut);
            std::cout << "Fetcher thread waiting for data......." << std::endl;
        }

        // let's sleep to give other threads time to catch up I think
        std::this_thread::sleep_for(2s);

        // acquire and lock the mutex with our RAII wrapper
        std::lock_guard<std::mutex> lock_guard(data_lock);
        // execute the critical section
        downloaded_data += "Block" + std::to_string(i+1);
        std::cout << "downloaded_data: " << downloaded_data << std::endl;
        string_updated = true;
        // notify the download_condition_variable that we have written to the shared mutable state and waiting threads can read the data
        download_condition_variable.notify_one();
    }
    // when the download is fully finished
    {
        std::lock_guard<std::mutex> print(print_mut);
        std::cout << "The download has completed" << std::endl;
    }
    std::lock_guard<std::mutex> final_lock_guard(data_lock);
    // set the boolean to true
    download_complete = true;
    // notify the condition variable
    download_condition_variable.notify_all();
}

void progress_bar()
{
    size_t len = 0;

    while (true)
    {
        {
            std::lock_guard<std::mutex> print_lock(print_mut);
            std::cout << "Progress thread is waiting for the data......" << std::endl;
        }

        std::unique_lock<std::mutex> uniq_lck(data_lock);
        download_condition_variable.wait_for(uniq_lck, 2s,  [] {return string_updated;});
        // At this point, the mutex is locked and we know either data was updated or download is complete
        len = downloaded_data.size();
        // Reset the flag safely while holding the lock
        string_updated = false;
        uniq_lck.unlock();
        {
            std::lock_guard<std::mutex> print_lock(print_mut);
            std::cout << "Received " << len << " bytes so far....." << std::endl;
        }
        std::unique_lock<std::mutex> final_uniq(data_lock);

        if(download_condition_variable.wait_for(final_uniq, 10ms, [] {return download_complete;})) {
            std::cout << "Progress bar has ended..." << std::endl;
            break;
        }

    }
}

void process_data() {
    {
        std::lock_guard<std::mutex> print(print_mut);
        std::cout << "Processing thread waiting for data......" << std::endl;
    }
    std::unique_lock<std::mutex> uniq_lck(data_lock);
    download_condition_variable.wait(uniq_lck, [] {return download_complete;});

    {
        std::lock_guard<std::mutex> print_lock(print_mut);
        std::cout << "Processing data: " << downloaded_data << std::endl;
    }

}

// solution

// shared variable for the data being fetched
std::string sharedData;

// bool flags for predicates
bool updateProgress = false;
bool completedTask = false;

// Mutexes to protect the shared variables
std::mutex dataMutex;
std::mutex completedMutex;

// The condition variables
std::condition_variable data_cv;
std::condition_variable completed_cv;


// Data fetching thread
void fetchData()
{
    for (int i = 0; i < 5; ++i) {
        std::cout << "Fetcher thread waiting for data....." << std::endl;
        std::this_thread::sleep_for(2s);

        // Update sharedData
        std::unique_lock<std::mutex> uniq_lck(dataMutex);
        sharedData += "Block" + std::to_string(i+1);
        std::cout << "Fetched sharedData: " << sharedData << std::endl;
        updateProgress = true;
        uniq_lck.unlock();
        data_cv.notify_all();
    }
    std::cout << "Fetch sharedData has ended" << std::endl;

    // Tell the progress bar thread to exit
    // and wake up the processing thread
    std::lock_guard<std::mutex> lg(completedMutex);
    completedTask = true;
    completed_cv.notify_all();
}


// Progress bar thread
void progressBar()
{
    size_t len = 0;

    while(true) {
        std::cout << "Progress bar thread waiting for data....." << std::endl;

        // Wait until there is some new data to display
        std::unique_lock<std::mutex> data_lck(dataMutex);
        data_cv.wait(data_lck, [] {return updateProgress;});

        // Wake up and use the new value
        len = sharedData.size();

        // Set our flag back to false
        updateProgress = false;
        data_lck.unlock();

        std::cout << "Received " << len << " bytes so far" << std::endl;

        // Check if the download has finished
        std::unique_lock<std::mutex> compl_lck(completed_mutex);

        // Use wait_for() to avoid blocking
        if (completed_cv.wait_for(compl_lck, 10ms, [] {return completedTask;})) {
            std::cout << "Progress bar thread has ended" << std::endl;
            break;
        }
    }
}

void processData()
{
    std::this_thread::sleep_for(200ms);
    std::cout << "Processing thread waiting for data...." << std::endl;

    // Wait until the download is complete
    std::unique_lock<std::mutex> compl_lck(completedMutex); // Acquire lock

    completed_cv.wait(compl_lck, [] {return completed;});
    compl_lck.unlock();

    std::lock_guard<std::mutex> data_lck(dataMutex);
    std::cout << "Processing sharedData: " << sharedData << std::endl;

    // Process the data.....
}


int main() {

     //start the threads
    std::thread fetcher(fetch_data);
    std::thread prog(progress_bar);
    std::thread processor(process_data);

    fetcher.join();
    prog.join();
    processor.join();

//    std::thread reader(reader_assignment);
//    std::thread writer(writer_assignment);
//
//    reader.join(); writer.join();

    //shared_data = "Empty";

    // Display its initial value
    //std::cout << "Data is: " << shared_data << std::endl;

    // the notification is not lost,
    // even if the writer thread finishes before the reader thread starts
    // or there is a "spurious wakeup" (wait returns without a notification)
//    std::thread write(writer_with_condition_variable);
//    std::this_thread::sleep_for(500ms);
//    std::thread read(reader_with_condition_variable);
//
//    write.join(), read.join();


    return 0;
}
