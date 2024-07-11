#include <iomanip>
#include <stdexcept>
#include <vector>
#include <climits>
#include <string>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#include <mach-o/dyld.h> // For _NSGetExecutablePath on macOS
#elif __linux__
#include <unistd.h>     // For readlink function on Linux
#endif

#include "util.h"

std::ostream& operator<<(std::ostream& os, const IPrintable& printable) {
    printable.print(os);
    return os;
}

FormattedTime::FormattedTime() : hours(0), minutes(0), seconds(0), milliseconds(0), microseconds(0) {}
    
FormattedTime::FormattedTime(int64_t microseconds) {
    int64_t temp = microseconds;
    
    this->microseconds = temp % 1000;
    temp /= 1000;
    
    milliseconds = temp % 1000;
    temp /= 1000;

    seconds = temp % 60;
    temp /= 60;

    minutes = temp % 60;
    temp /= 60;

    hours = temp;
}

void FormattedTime::print(std::ostream& os) const {
    os << std::setfill('0');
    os << std::setw(2) << hours << ":";
    os << std::setw(2) << minutes << ":";
    os << std::setw(2) << seconds << ".";
    os << std::setw(3) << milliseconds;
    os << std::setw(3) << microseconds;
}

std::string getExecutableDirectory() {
    std::vector<char> buffer(PATH_MAX);

#ifdef __APPLE__
    uint32_t len = buffer.size();
    if (_NSGetExecutablePath(buffer.data(), &len) != 0) {
        throw std::runtime_error("failed to get executable path");
    }
#elif __linux__
    ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (len == -1) {
        throw std::runtime_error("failed to get executable path");
    }
    buffer[len] = '\0'; // Null-terminate the string.
#else
    throw std::runtime_error("unsupported operating system");
#endif

    // Find the last occurrence of the directory separator.
    std::string exePath(buffer.data());
    size_t pos = exePath.find_last_of('/');
    if (pos == std::string::npos) {
        return "";
    } else {
        return exePath.substr(0, pos);
    }
}
