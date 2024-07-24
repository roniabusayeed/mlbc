#pragma once

#include <string>
#include <iostream>
#include <cassert>
#include <future>
#include <chrono>
#include <concepts>
#include <filesystem>

#include <glm/glm.hpp>
#include <json.hpp>

#include "imgui.h"
#include "IconsFontAwesome.h"

template <typename T>
concept Printable = requires(T obj) {
    // { expression-1-that-has-to-be-valid };
    // { expression-2-that-has-to-be-valid };
    // { expression-3-that-has-to-be-valid };
    // ...
    { std::cout << obj };
};

template <typename T> 
    requires Printable<T>
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

    bool display_fractional_seconds;

    FormattedTime(bool display_fractional_seconds = false);
    explicit FormattedTime(int64_t microseconds, bool display_fractional_seconds = false);
    void print(std::ostream& os) const override;
    
    static FormattedTime fromSeconds(int64_t seconds, bool display_fractional_seconds = false);
};

inline ImVec2 toImVec2(const glm::vec2& value) { return {value.x, value.y}; }
inline ImVec4 toImVec4(const glm::vec4& value) { return {value.x, value.y, value.z, value.w}; }

inline glm::vec4 toGLMVec4(const std::vector<float>& value) {
    assert(value.size() == 4);
    return {value[0], value[1], value[2], value[3]};
}

using json = nlohmann::json;

std::string getExecutableDirectory();

template <typename... T>
    requires (std::convertible_to<T, std::string> && ...)
inline std::string joinPaths(const T&... path) {
    std::vector path_segments({std::string(path)...});
    namespace fs = std::filesystem;
    fs::path result;
    for (const auto& path_segment : path_segments) {
        result /= path_segment;
    }
    return result.string();
}

template<typename T>
inline bool isFutureReady(std::future<T>& future) {
    return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

#include "fileDialog.h"

namespace ui {
    ImFont* loadFont(const std::string& filepath, float size);
    ImFont* loadIconFont(const std::string& filepath, float size);
    std::pair<std::optional<std::string>, std::optional<std::string>> extractTitleAndID(std::optional<std::string> label);
}

enum class MediaType: int32_t {
    Image = 0,
    Audio = 1
};

std::vector<std::string> getValidExtensions(MediaType media_type);
std::vector<std::string> loadMediaFiles(const std::string& directory, MediaType media_type);
void loadMediaFilesAsync(const std::string& directory, MediaType mediaType, std::function<void(const std::vector<std::string>&)> on_media_files_loaded);
void moveFile(const std::string& filepath, const std::string& dest_directory, std::function<void(const std::string& error_message)> error_callback);

template <typename T> 
    requires Printable<T>
std::string toString(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}
