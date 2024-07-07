#pragma once

#include <string>
#include <iostream>

template <typename T>
inline void print(T obj, const std::string& end = "\n", bool flush_buffer = false) {
    std::cout << obj << end;
    if (flush_buffer) {
        std::cout.flush();
    }
}

class IPrintable {
public:
    virtual void print(std::ostream& os) const = 0;
    virtual ~IPrintable() = default;
};

std::ostream& operator<<(std::ostream& os, const IPrintable& printable);

struct FormattedTime : public IPrintable {
    int32_t hours;
    int8_t minutes;
    int8_t seconds;
    int16_t milliseconds;
    int16_t microseconds;

    FormattedTime();
    explicit FormattedTime(int64_t microseconds);

    void print(std::ostream& os) const override;
};
