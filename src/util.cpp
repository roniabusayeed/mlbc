#include <iomanip>
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
