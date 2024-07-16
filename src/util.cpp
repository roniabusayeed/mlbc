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
#include "imgui.h"
#include "constants.h"

std::ostream& operator<<(std::ostream& os, const IPrintable& printable) {
    printable.print(os);
    return os;
}

FormattedTime::FormattedTime(bool display_fractional_seconds) : hours(0), minutes(0), seconds(0), milliseconds(0), microseconds(0), display_fractional_seconds(display_fractional_seconds) {}
    
FormattedTime::FormattedTime(int64_t microseconds, bool display_fractional_seconds)
    : display_fractional_seconds(display_fractional_seconds) {
    int64_t tmp = microseconds;
    
    this->microseconds = tmp % 1000;
    tmp /= 1000;
    
    milliseconds = tmp % 1000;
    tmp /= 1000;

    seconds = tmp % 60;
    tmp /= 60;

    minutes = tmp % 60;
    tmp /= 60;

    hours = tmp;
}

void FormattedTime::print(std::ostream& os) const {
    os << std::setfill('0');
    os << std::setw(2) << hours << ":";
    os << std::setw(2) << static_cast<int>(minutes) << ":";
    os << std::setw(2) << static_cast<int>(seconds);

    if (display_fractional_seconds) {
        os << "." << std::setw(3) << milliseconds;
        os << std::setw(3) << microseconds;
    }
}

FormattedTime FormattedTime::fromSeconds(int64_t seconds, bool display_fractional_seconds) {
    return FormattedTime(seconds * 1000000LL, display_fractional_seconds);
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

namespace ui {
    ImFont* loadFont(const std::string& filepath, float size) {
        ImFont* loaded_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filepath.c_str(), size);
        
        if (!loaded_font) {
            throw std::runtime_error("couldn't load UI font: " + filepath);
        }

        return loaded_font;
    }

    ImFont* loadIconFont(const std::string& filepath, float size) {
        float icon_font_size = size;
        // float icon_font_size = size * 2.0f / 3.0f;

        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        
        ImFontConfig icons_config; 
        icons_config.MergeMode = true; 
        icons_config.PixelSnapH = true; 
        icons_config.GlyphMinAdvanceX = icon_font_size;
        
        ImFont* loaded_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filepath.c_str(), icon_font_size, &icons_config, icons_ranges);
        
        if (!loaded_font) {
            throw std::runtime_error("couldn't load UI icon font: " + filepath);
        }

        return loaded_font;
    }

    std::pair<std::optional<std::string>, std::optional<std::string>> extractTitleAndID(std::optional<std::string> label) {
        
        // If the label is std::nullopt or an empty string then both the display string
        // the id string are std::nullopt.
        if (!label || !label->length()) {
            return {std::nullopt, std::nullopt};
        }

        std::string label_str = label.value();
        std::optional<std::string> display_string;
        std::optional<std::string> id_string;

        // If the label has "###", the preceding substring of non-zero length is the display string
        // and the following substring of non-zero length is the id string.
        size_t triple_hash_pos = label_str.find("###");
        if (triple_hash_pos != std::string::npos) {
            display_string = (triple_hash_pos > 0) ? std::optional(label_str.substr(0, triple_hash_pos)) : std::nullopt;
            id_string = label_str.substr(triple_hash_pos + 3);
            if (!id_string.value().length()) {
                id_string = std::nullopt;
            }
            return {display_string, id_string};
        }

        // If the label has "##", the preceding substring of non-zero length is the display string
        // and the entire label string is the display string.
        size_t double_hash_pos = label_str.find("##");
        if (double_hash_pos != std::string::npos) {
            display_string = (double_hash_pos > 0) ? std::optional(label_str.substr(0, double_hash_pos)) : std::nullopt;
            id_string = label_str;
            return {display_string, id_string};
        }

        // If neither ### nor ## is found, the label is both display string and id string.
        return {label, label};
    }
}

std::vector<std::string> getValidExtensions(MediaType media_type) {
    if (media_type == MediaType::Image) {
        return {".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tiff"};
    }
    if (media_type == MediaType::Audio) {
        return {".mp3", ".wav", ".flac", ".aac", ".ogg", ".m4a"};
    }
    throw std::invalid_argument("Unsupported media type");
}

std::vector<std::string> loadMediaFiles(const std::string& directory_path, MediaType media_type) {
    namespace fs = std::filesystem;

    std::vector<std::string> media_files;
    auto valid_extensions = getValidExtensions(media_type);

    try {
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                auto file_path = entry.path();
                auto extension = file_path.extension().string();
                if (std::find(valid_extensions.begin(), valid_extensions.end(), extension) != valid_extensions.end()) {
                    media_files.push_back(file_path.string());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Error accessing directory: " + std::string(e.what()));
    }

    return media_files;
}
