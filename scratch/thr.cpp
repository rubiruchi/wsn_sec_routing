#include <thread>
#include <iostream>

using namespace std;

int main (int argc, char *argv[])
{
    thread thr([]{
        cout << "running thread" << endl;
        return;
    });
    thr.join();
    return 0;
}

