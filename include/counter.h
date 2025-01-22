#ifndef COUNTER_H
#define COUNTER_H

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

void createSharedMemory(bool initialize = false, pid_t ProcessID = NULL);
void destroySharedMemory();
void setCounter(int value);
int getCounter();
void setIsLeader(bool value);
bool getIsLeader();
void setLeaderPID(pid_t value);
pid_t getLeaderPID();


#endif 
