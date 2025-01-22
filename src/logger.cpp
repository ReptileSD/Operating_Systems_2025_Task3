#include "logger.h"
#include <fstream>
#include <ctime>
#include <sstream>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <signal.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string getCurrentTime() {
    std::ostringstream oss;
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    oss << (localTime->tm_year + 1900) << "-"
        << (localTime->tm_mon + 1) << "-"
        << localTime->tm_mday << " "
        << localTime->tm_hour << ":"
        << localTime->tm_min << ":"
        << localTime->tm_sec;

#if defined(_WIN32) || defined(_WIN64)
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    oss << "." << systemTime.wMilliseconds;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    oss << "." << (tv.tv_usec / 1000);
#endif

    return oss.str();
}

void writeToLog(const std::string& message) {
    const std::string logDir = "logs";
    const std::string logFilePath = logDir + "/program.log";

#ifdef _WIN32
    _mkdir(logDir.c_str());
#else
    mkdir(logDir.c_str(), 0755);
#endif

    std::ofstream log(logFilePath, std::ios::app);

    if (log.is_open()) {
        std::string currentTime = getCurrentTime();
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        log << "[" << currentTime << "] "
            << message << std::endl;
    } else {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}
