#include "logger.h"
#include "counter.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <signal.h>
#include <sstream>
#include <limits>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif

bool running = true;
bool childProcessRunning = false;
bool isLeader = false;


pid_t getProcessId() {
#if defined(_WIN32) || defined(_WIN64)
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

void getCurrentTime(std::time_t &sec, int &msec) {
#if defined(_WIN32) || defined(_WIN64)
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    sec = std::time(nullptr);
    msec = systemTime.wMilliseconds;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;
#endif
}

int calculateElapsedTime(std::time_t startSec, int startMsec) {
    std::time_t currentSec;
    int currentMsec;
    getCurrentTime(currentSec, currentMsec);

    return (currentSec - startSec) * 1000 + (currentMsec - startMsec);
}

void handleEvents(std::time_t &currentSec, int &currentMsec) {
    static std::time_t lastIncrementEvent = currentSec;
    static std::time_t lastIncrementEventMsec = currentMsec;
    static std::time_t lastLogEvent = currentSec;
    static std::time_t lastLogEventMsec = currentMsec;
    static std::time_t lastChildEvent = currentSec;
    static std::time_t lastChildEventMsec = currentMsec;

    pid_t pid = getProcessId();

    if ((currentSec - lastIncrementEvent) * 1000 + (currentMsec - lastIncrementEventMsec) >= 300) {
        int value = getCounter();
        setCounter(value + 1);
        lastIncrementEvent = currentSec;
        lastIncrementEventMsec = currentMsec;
    }

    if (((currentSec - lastLogEvent) * 1000 + (currentMsec - lastLogEventMsec) >= 1000) and (isLeader)) {
        writeToLog("PID: " + std::to_string(pid) +
                   ", Current counter value: " + std::to_string(getCounter()));
        lastLogEvent = currentSec; 
        lastLogEventMsec = currentMsec;
    }

    if (((currentSec - lastChildEvent) * 1000 + (currentMsec - lastChildEventMsec) >= 3000) and (isLeader)) {
        if (!childProcessRunning) {
            writeToLog("PID: " + std::to_string(pid) +
                       " - Launching child processes...");
            #if defined(_WIN32) || defined(_WIN64)
            STARTUPINFO si1 = { sizeof(STARTUPINFO) };
            PROCESS_INFORMATION pi1;
            if (!CreateProcess(
                    "child_program_1.exe", 
                    NULL, 
                    NULL, 
                    NULL, 
                    TRUE, 
                    0, 
                    NULL, 
                    NULL, 
                    &si1, 
                    &pi1
            )) {
                std::cerr << "Error launching child process 1." << std::endl;
            } else {
                CloseHandle(pi1.hProcess);
                CloseHandle(pi1.hThread);
            }

            STARTUPINFO si2 = { sizeof(STARTUPINFO) };
            PROCESS_INFORMATION pi2;

            if (!CreateProcess(
                    "child_program_2.exe", 
                    NULL, 
                    NULL, 
                    NULL, 
                    FALSE, 
                    0, 
                    NULL, 
                    NULL, 
                    &si2, 
                    &pi2
            )) {
                std::cerr << "Error launching child process 2." << std::endl;
            } else {
                CloseHandle(pi2.hProcess);
                CloseHandle(pi2.hThread);
            }


            childProcessRunning = true;
            WaitForSingleObject(pi1.hProcess, INFINITE);
            WaitForSingleObject(pi2.hProcess, INFINITE);

            #else
            pid_t pid1 = fork();
            if (pid1 == -1) {
                std::cerr << "Fork failed for child program 1" << std::endl;
            } else if (pid1 == 0) {
                std::cout << "Launching child program 1..." << std::endl;
                if (execlp("./child_program_1", "child_program_1", (char*)NULL) == -1) {
                    perror("Execlp failed for child program 1");
                    exit(1);
                }
            }

            pid_t pid2 = fork();
            if (pid2 == -1) {
                std::cerr << "Fork failed for child program 2" << std::endl;
            } else if (pid2 == 0) {
                std::cout << "Launching child program 2..." << std::endl;
                if (execlp("./child_program_2", "child_program_2", (char*)NULL) == -1) {
                    perror("Execlp failed for child program 2");
                    exit(1);
                }
            }

            int status;
            if (pid1 > 0) {
                waitpid(pid1, &status, 0);
            }
            if (pid2 > 0) {
                waitpid(pid2, &status, 0);  
            }

            #endif
            childProcessRunning = false;
        } else {
            writeToLog("Child process is still running. Skipping launch.");
        }
        lastChildEvent = currentSec; 
        lastChildEventMsec = currentMsec;
    }
}



void handleUserCommands(pid_t pid = -1) {
    std::string command;
    while (running) {
        #ifdef _WIN32
        std::cout << "Enter command (show, reset, set <value>, exit): ";
        #else
        std::cout << "Enter command (show, reset, set <value>, ctrl+C for exit): ";
        #endif
        std::cin >> command;
        if (command == "exit") {
        #ifdef _WIN32
            running = false;
            break;
        #endif
        } else if (command == "show") {
            int value = getCounter();
            std::cout << "Current counter value: " << value << std::endl;
        } else if (command == "reset") {
            setCounter(0);
            std::cout << "Counter reset." << std::endl;
        } else if (command == "set") {
            int newValue;
            if (std::cin >> newValue) {
                setCounter(newValue);
                std::cout << "Counter set to " << newValue << "." << std::endl;
            } else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid value. Please enter a valid integer." << std::endl;
            }
        } else {
            std::cout << "Unknown command. Available commands: show, reset, set <value>, exit." << std::endl;
        }
    }
}

#if defined(_WIN32) || defined(_WIN64)
DWORD WINAPI CommandThread(LPVOID lpParam) {
    handleUserCommands();
    return 0;
}
#endif

void handleSignals(int signal) {
    running = false; 
    if (isLeader) {
        setIsLeader(false);
    }
    exit(0); 
}

void setupSignalHandlers() {
#if !defined(_WIN32) && !defined(_WIN64)
    struct sigaction sa;
    sa.sa_handler = handleSignals;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#endif
}

bool isLeaderAlive(pid_t leaderPID) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, leaderPID);
    if (hProcess == NULL) {
        return false; 
    }
    CloseHandle(hProcess);
    return true;
#else
    return (kill(leaderPID, 0) == 0); 
#endif
}

void checkAndReassignLeadership() {
    bool leadershipChanged = false;
    std::cout << getLeaderPID() << "\n";

    if (!isLeaderAlive(getLeaderPID())) {
        setLeaderPID(getProcessId());
        setIsLeader(true);
        isLeader = true;
        leadershipChanged = true;
        std::cout << "Process " << getProcessId() << " became the new leader." << std::endl;
    }

    if (leadershipChanged) {
        writeToLog("Process " + std::to_string(getProcessId()) + " took over leadership.");
    }
}

int main() {
    createSharedMemory(true, getProcessId());

    if (!getIsLeader()) {
        setIsLeader(true);
        isLeader = true;
    }

    setCounter(0);
    setupSignalHandlers();

    std::time_t currentSec;
    int currentMsec;
    getCurrentTime(currentSec, currentMsec);

#if defined(_WIN32) || defined(_WIN64)
    DWORD threadId;
    HANDLE userCommandThread = CreateThread(
        NULL,
        0,
        CommandThread,
        NULL,
        0,
        &threadId
    );
    if (userCommandThread == NULL) {
        std::cerr << "Error creating thread for handling user commands." << std::endl;
        return 1;
    }
#else
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Error creating child process for handling user commands." << std::endl;
        return 1;
    } else if (pid == 0) {
        handleUserCommands(getpid());
        exit(0);
    }
#endif

    while (running) {
        getCurrentTime(currentSec, currentMsec);
        checkAndReassignLeadership();
        handleEvents(currentSec, currentMsec);

#if defined(_WIN32) || defined(_WIN64)
        Sleep(100); 
#else
        usleep(100000);
#endif
    }

    if (isLeader) {
        setIsLeader(false);
    }

#if defined(_WIN32) || defined(_WIN64)
    WaitForSingleObject(userCommandThread, INFINITE);
    CloseHandle(userCommandThread);
#else
    int status;
    waitpid(pid, &status, 0); 
#endif

    destroySharedMemory();

    return 0;
}
