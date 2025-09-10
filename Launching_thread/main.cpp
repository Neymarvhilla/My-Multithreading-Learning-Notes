#include <iostream>
#include <thread>
#include <vector>


// functor class with overloaded () operator

class Hello {
public:
    void operator() () {
        std::cout << "Hello Functor Thread" << std::endl;
    }
};

void say_hello () {
    std::cout << "Hello Thread" << std::endl;
}

void fizzbuzz() {
    for (int i = 0; i <= 50; ++i) {
        if (i % 3 == 0 && i % 5 == 0) {
            std::cout << "FizzBuzz" << std::endl;
        }
        else if (i % 3 == 0) {
            std::cout << "Fizz" << std::endl;
        }
        else if (i % 5 == 0) {
            std::cout << "Buzz" << std::endl;
        }
        else {
            std::cout << i << std::endl;
        }
    }
}

void my_vec() {
    std::vector<int> nums{1,2,3,4,};
    for (auto &value : nums) {
        std::cout << "number: " << value << std::endl;
    }
}


/*
 * A thread requires an entry point, which should be a callable object (functions, functor, lambdas etc.)
 * Operating system by default creates a thread with main function as its entry point.
 *
 * In a multithreaded architectures, all threads are under a single process, and they all share the processes resources.
 *
 * A process can be single threaded with the main function as its entry point or multithreaded with main function as the entry point for the main thread.
 *
 * Single Threaded process
 *  - A process with a single thread of execution with the main function acting as an entry point. All tasks are executed sequentially within this thread.
 *  characteristics
 *      it has one thread which runs the programs logic
 *      it uses a single stack for function calls and local variables.
 *      simpler to manage but limited in parallelism, as it can't leverage multiple cpu cores effectively.
 *
 * Multi Threaded process
 * - A process with multiple threads where the main function is the entry point for the main thread, which can spawn additional threads to perform tasks concurrently within the same process.
 *      characteristics
 *          All threads share the process memory space but have their own stacks for local variables and function calls.
 *          the OS schedules threads across CPU cores, enabling parallelism.
 *          Requires synchronization to avoid issues like data races*/
int main() {


    // create an std::thread object
    // and pass the task function to the constructor
   Hello hello;
   // std::thread thread1(hello);
   std::thread thread1(hello);


    // wait for the thread to complete
    thread1.join();
    // using a lambda as an entry point in the thread constructor
    std::thread thread2([] () {std::cout << "say hello to thread 2";});
    thread2.join();

    std::thread thread3(fizzbuzz);

    thread3.join();

    std::thread thread4 (my_vec);
    thread4.join();



    return 0;
}
