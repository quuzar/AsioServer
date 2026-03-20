#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <iostream>

class Logger {
public:
    static void log(const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        struct tm tm_buf;
        localtime_s(&tm_buf, &time); 

        std::cout << "["
            << std::put_time(&tm_buf, "%H:%M:%S")
            << "] " << msg << std::endl;
    }
};