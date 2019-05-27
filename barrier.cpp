#include <iostream>
#include <vector>
#include <thread>
#include <atomic>

#define NUMTHRD 5

std::atomic<int> thrCount;
pthread_barrier_t thrBar;

void thrFunc(int n)
{
    int n = 0;
    int thrNum = n;
    do {
        pthread_barrier_wait(&thrBar);
        n = thrCount.fetch_add(std::memory_order_relaxed);
        std::cout << "Thread " << thrNum << ": " << n << std::endl;
    } while (n < 20);
    return;
}

int main()
{
    pthread_barrier_init(&thrBar, NULL, NUMTHRD);
    thrCount.store(0, std::memory_order_relaxed);

    std::thread *thrds[NUMTHRD];
    for (int i = 0; i < NUMTHRD; i++) {
        thrds[i] = new std::thread(&thrFunc);
    }

    for (int i = 0; i < NUMTHRD; i++) thrds[i]->join();
    pthread_barrier_destroy(&thrBar);
    return 0;
}