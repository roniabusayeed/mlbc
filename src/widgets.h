#pragma once
#include <optional>
#include <functional>
#include <imgui.h>

#include "util.h"

namespace ui {
namespace widget {

    /**
     * @return `true` if the value has been changed after the user interaction has ended. Returns `false`
     * otherwise.
     */
    bool DragReset(
        std::optional<std::string> label,
        float& value,
        std::function<std::string(float)> formatter,
        float min,
        float max,
        float reset,
        std::optional<float> drag_width = std::nullopt,
        std::optional<float> reset_button_width = std::nullopt,
        std::optional<float> vertical_padding = std::nullopt,
        std::optional<float> horizontal_widget_sapcing = std::nullopt
    );
}}  // namespace widget, ui
