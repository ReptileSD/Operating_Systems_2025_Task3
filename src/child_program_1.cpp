#include "logger.h"
#include "counter.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h> 
#else
#include <unistd.h>
#endif

int main() {
#if defined(_WIN32) || defined(_WIN64)
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif

    writeToLog("Child process 1 started. PID: " + std::to_string(pid));
    createSharedMemory(false);

    int value = getCounter();

    setCounter(value + 10);


    writeToLog("Child process 1 exiting. PID: " + std::to_string(pid));

    return 0;
}