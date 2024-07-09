#pragma once

#include <string>
#include <iostream>
#include <cassert>
#include <future>
#include <chrono>

#include <glm/glm.hpp>
#include <json.hpp>

#include "imgui.h"

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

inline ImVec2 toImVec2(const glm::vec2& value) { return {value.x, value.y}; }
inline ImVec4 toImVec4(const glm::vec4& value) { return {value.x, value.y, value.z, value.w}; }

inline glm::vec4 toGLMVec4(const std::vector<float>& value) {
    assert(value.size() == 4);
    return {value[0], value[1], value[2], value[3]};
}

using json = nlohmann::json;

std::string getExecutableDirectory();
std::string joinPaths(const std::string& path1, const std::string& path2);

template<typename T>
bool isFutureReady(std::future<T>& future) {
    return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

#include "fileDialog.h"
