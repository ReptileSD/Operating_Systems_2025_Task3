#include "logger.h"
#include "counter.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>

#endif

int main() {
#if defined(_WIN32) || defined(_WIN64)
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif

    writeToLog("Child process 2 started. PID: " + std::to_string(pid));
    createSharedMemory(false);


    int value = getCounter();
    setCounter(value * 2);

    std::time_t startTimeSec;
    int startMilliSec;
#if defined(_WIN32) || defined(_WIN64)
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    startTimeSec = std::time(nullptr);
    startMilliSec = systemTime.wMilliseconds;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    startTimeSec = tv.tv_sec;
    startMilliSec = tv.tv_usec / 1000;
#endif

    while (true) {
#if defined(_WIN32) || defined(_WIN64)
        GetSystemTime(&systemTime);
        std::time_t currentTimeSec = std::time(nullptr);
        int currentMilliSec = systemTime.wMilliseconds;
#else
        gettimeofday(&tv, nullptr);
        std::time_t currentTimeSec = tv.tv_sec;
        int currentMilliSec = tv.tv_usec / 1000;
#endif

        int elapsedMilliSec = (currentTimeSec - startTimeSec) * 1000 + (currentMilliSec - startMilliSec);
        if (elapsedMilliSec >= 2000) {
            break;
        }
    }

    value = getCounter();
    setCounter(value / 2);

    writeToLog("Child process 2 exiting. PID: " + std::to_string(pid));

    return 0;
}
