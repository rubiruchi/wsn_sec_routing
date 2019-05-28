#include <iostream>
#include <vector>
#include <thread>
#include <atomic>

#define NUMTHRD 5

std::atomic<int> thrCount;
pthread_barrier_t thrBar;
pthread_mutex_t cliMut;
int thrAlive;

void thrFunc(int thrNum)
{
    int n = 0;
    do {
        pthread_barrier_wait(&thrBar);
        n = thrCount++;
        pthread_mutex_lock(&cliMut);
        std::cout << "Thread " << thrNum << ": " << n << std::endl;
        pthread_mutex_unlock(&cliMut);
    } while (n < 18);
    pthread_mutex_lock(&cliMut);
    std::cout << "Thread " << thrNum << ": " << "quit" << std::endl;
    pthread_barrier_destroy(&thrBar);
    pthread_barrier_init(&thrBar, NULL, --thrAlive);
    pthread_mutex_unlock(&cliMut);
    return;
}

int main()
{
    thrAlive = NUMTHRD;
    pthread_barrier_init(&thrBar, NULL, thrAlive);
    pthread_mutex_init(&cliMut, NULL);
    thrCount.store(0, std::memory_order_relaxed);

    std::thread *thrds[NUMTHRD];
    for (int i = 0; i < NUMTHRD; i++) {
        thrds[i] = new std::thread(&thrFunc, i);
    }

    for (int i = 0; i < NUMTHRD; i++) thrds[i]->join();
    pthread_barrier_destroy(&thrBar);
    return 0;
}