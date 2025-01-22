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
#define SHARED_MEMORY_SIZE sizeof(int)
#define SEMAPHORE_NAME "CounterSemaphore"

int *sharedCounter;

#ifdef _WIN32
HANDLE hMapFile; 
HANDLE hSemaphore;
#endif

void createSharedMemory(bool isParent) {
#ifdef _WIN32
    if (isParent) {
        hSemaphore = CreateSemaphore(NULL, 1, 1, SEMAPHORE_NAME);
        if (hSemaphore == NULL) {
            std::cerr << "Error creating semaphore: " << GetLastError() << std::endl;
            exit(1);
        }

        hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHARED_MEMORY_SIZE, SHARED_MEMORY_NAME);
        if (hMapFile == NULL) {
            std::cerr << "Error creating file mapping: " << GetLastError() << std::endl;
            exit(1);
        }
    } else {
        hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_NAME);
        if (hSemaphore == NULL) {
            std::cerr << "Error opening semaphore: " << GetLastError() << std::endl;
            exit(1);
        }

        hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
        if (hMapFile == NULL) {
            std::cerr << "Error opening file mapping: " << GetLastError() << std::endl;
            exit(1);
        }
    }

    sharedCounter = (int *)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEMORY_SIZE);
    if (sharedCounter == NULL) {
        std::cerr << "Error mapping view of file: " << GetLastError() << std::endl;
        exit(1);
    }
#else
    int fd;
    if (isParent) {
        fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            std::cerr << "Failed to create shared memory." << std::endl;
            exit(1);
        }

        if (ftruncate(fd, SHARED_MEMORY_SIZE) == -1) {
            std::cerr << "Failed to set size for shared memory." << std::endl;
            exit(1);
        }
    } else {
        fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            std::cerr << "Failed to open shared memory." << std::endl;
            exit(1);
        }
    }

    sharedCounter = (int *)mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sharedCounter == MAP_FAILED) {
        std::cerr << "Failed to map shared memory." << std::endl;
        exit(1);
    }

    close(fd);
#endif
}


void destroySharedMemory() {
#ifdef _WIN32
    CloseHandle(hSemaphore);

    UnmapViewOfFile(sharedCounter);
#else
    if (munmap(sharedCounter, SHARED_MEMORY_SIZE) == -1) {
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

    *sharedCounter = value;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif
}

int getCounter() {
#ifdef _WIN32
    WaitForSingleObject(hSemaphore, 5000);
#endif

    int value = *sharedCounter;

#ifdef _WIN32
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif

    return value;
}
