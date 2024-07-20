#pragma once
#include <optional>
#include <functional>
#include <imgui.h>

#include "util.h"
#include "image.h"

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

    /**
     * @brief Displays an image within the available ImGui content region, maintaining its aspect ratio.
     * This function scales down the image if necessary to fit within the available viewport while maintaining
     * the original aspect ratio. It also centers the image both vertically and horizontally within the viewport.
     *
     * @param image The Image object containing the texture to display, along with its width and height.
     */
    void ImageView(const Image& image);

}}  // namespace widget, ui
