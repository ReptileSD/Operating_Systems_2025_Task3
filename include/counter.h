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

void createSharedMemory(bool isParent);
void destroySharedMemory();
void setCounter(int value);
int getCounter();

#endif 
