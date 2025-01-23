#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#define SHARED_MEMORY_NAME "CounterSharedMemory"
#define SHARED_MEMORY_SIZE sizeof(SharedMemory)
#define SEMAPHORE_NAME "CounterSemaphore"

struct SharedMemory {
    int counter;
    bool isLeader;
    pid_t leaderPID;
};

SharedMemory *sharedMemory;

#ifdef _WIN32
HANDLE hMapFile; 
HANDLE hSemaphore;
#endif

void createSharedMemory(bool initialize = false, pid_t ProcessID = NULL) {
#ifdef _WIN32
    hSemaphore = CreateSemaphore(NULL, 1, 1, SEMAPHORE_NAME);
    if (hSemaphore == NULL) {
        std::cerr << "Error creating semaphore: " << GetLastError() << std::endl;
        exit(1);
    }

    bool isFirstProcess = (GetLastError() != ERROR_ALREADY_EXISTS);

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHARED_MEMORY_SIZE, SHARED_MEMORY_NAME);
    if (hMapFile == NULL) {
        std::cerr << "Error creating or opening file mapping: " << GetLastError() << std::endl;
        exit(1);
    }

    sharedMemory = (SharedMemory *)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEMORY_SIZE);
    if (sharedMemory == NULL) {
        std::cerr << "Error mapping view of file: " << GetLastError() << std::endl;
        exit(1);
    }

    if (isFirstProcess) {
        WaitForSingleObject(hSemaphore, INFINITE); 
        sharedMemory->counter = 0;
        sharedMemory->isLeader = false;
        sharedMemory->leaderPID = ProcessID;
        ReleaseSemaphore(hSemaphore, 1, NULL);
    }
#else
int fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
bool isFirstProcess = (fd != -1);

if (!isFirstProcess) {
    fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        std::cerr << "Failed to open existing shared memory." << std::endl;
        exit(1);
    }
}

if (isFirstProcess) {
    if (ftruncate(fd, SHARED_MEMORY_SIZE) == -1) {
        std::cerr << "Failed to set size for shared memory." << std::endl;
        shm_unlink(SHARED_MEMORY_NAME);
        exit(1);
    }
}

sharedMemory = (SharedMemory *)mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
if (sharedMemory == MAP_FAILED) {
    std::cerr << "Failed to map shared memory." << std::endl;
    if (isFirstProcess) {
        shm_unlink(SHARED_MEMORY_NAME);
    }
    exit(1);
}

if (isFirstProcess) {
    sharedMemory->counter = 0;
    sharedMemory->isLeader = false;
    sharedMemory->leaderPID = ProcessID;
    std::cout << "Leader PID initialized to: " << ProcessID << std::endl; // Отладка
}

close(fd);

#endif
}


void destroySharedMemory() {
#ifdef _WIN32
    CloseHandle(hSemaphore);

    UnmapViewOfFile(sharedMemory);
#else
    if (munmap(sharedMemory, SHARED_MEMORY_SIZE) == -1) {
        std::cerr << "Failed to unmap shared memory." << std::endl;
    }

    if (shm_unlink(SHARED_MEMORY_NAME) == -1) {
        std::cerr << "Failed to unlink shared memory." << std::endl;
    }
#endif
}

void setCounter(int value) {
#ifdef _WIN32
    WaitForSingleObject(hSemaphore, INFINITE);
#endif

    sharedMemory->counter = value;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif
}

int getCounter() {
#ifdef _WIN32
    WaitForSingleObject(hSemaphore, 5000);
#endif

    int value = sharedMemory->counter;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif

    return value;
}

bool getIsLeader() {
#ifdef _WIN32
    WaitForSingleObject(hSemaphore, 5000);
#endif

    bool isLeader = sharedMemory->isLeader;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif

    return isLeader;
}

void setIsLeader(bool value) {
#ifdef _WIN32
    WaitForSingleObject(hSemaphore, INFINITE);
#endif

    sharedMemory->isLeader = value;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif
}

void setLeaderPID(pid_t value) {
#ifdef _WIN32
    WaitForSingleObject(hSemaphore, INFINITE);
#endif

    sharedMemory->leaderPID = value;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif
}

pid_t getLeaderPID() {
#ifdef _WIN32
    WaitForSingleObject(hSemaphore, 5000);
#endif

    pid_t leaderPID = sharedMemory->leaderPID;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif

    return leaderPID;
}