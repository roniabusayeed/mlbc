#pragma once

#include <string>
#include <iostream>

template <typename T>
inline void print(T obj, const char* end = "\n", bool flush_buffer = false) {
    std::cout << obj << end;
    if (flush_buffer) {
        std::cout.flush();
    }
}
