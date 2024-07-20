#include "widgets.h"
#include "image.h"

namespace ui {
namespace widget {
    bool DragReset(
        std::optional<std::string> label,
        float& value,
        std::function<std::string(float)> formatter,
        float min,
        float max,
        float reset,
        std::optional<float> drag_width,
        std::optional<float> reset_button_width,
        std::optional<float> vertical_padding,
        std::optional<float> horizontal_widget_sapcing
    ) {
        std::pair<std::optional<std::string>, std::optional<std::string>> title_id_pair = ui::extractTitleAndID(label);
        const std::optional<std::string>& title = title_id_pair.first;
        const std::optional<std::string>& id = title_id_pair.second;

        const std::string id_suffix = "drag-reset";
        const std::string suffixed_id = id ? (id.value() + id_suffix) : id_suffix;
        ImGui::PushID(suffixed_id.c_str());

        bool value_changed = false;

        if (vertical_padding) { ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {ImGui::GetStyle().FramePadding.x, *vertical_padding}); }
        if (horizontal_widget_sapcing) { ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {*horizontal_widget_sapcing, ImGui::GetStyle().ItemSpacing.y}); }

        std::string formatted_display = formatter(value);
        if (drag_width) {
            ImGui::SetNextItemWidth(*drag_width);
        } else {
            const float drag_float_text_width = ImGui::CalcTextSize(formatted_display.c_str()).x;
            const float drag_float_text_width_padding = 16.0f;
            ImGui::SetNextItemWidth(drag_float_text_width + drag_float_text_width_padding);
        }
        ImGui::DragFloat("", &value, 1.0f, min, max, formatted_display.c_str(), ImGuiSliderFlags_AlwaysClamp);
        
        static float previous_value = value;
        static bool was_active = false;
        if (ImGui::IsItemActive()) {
            was_active = true;
        } else if (was_active) {
            was_active = false;

            if (value != previous_value) {
                value_changed = true;
                previous_value = value;
            } else {
                value_changed = false;
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ROTATE_LEFT, {reset_button_width ? *reset_button_width : 0.0f, 0.f})) {
            value = reset;

            if (value != previous_value) {
                value_changed = true;
                previous_value = value;
            } else {
                value_changed = false;
            }
        }

        if (horizontal_widget_sapcing) { ImGui::PopStyleVar(); }

        if (title) {
            ImGui::SameLine();
            ImGui::Text("%s", title->c_str());
        }

        if (vertical_padding) { ImGui::PopStyleVar(); }
        

        ImGui::PopID();
        return value_changed;
    }

    void ImageView(const Image& image) {
        ImVec2 image_size = {static_cast<float>(image.getWidth()), static_cast<float>(image.getHeight())};
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();

        float image_aspect_ratio = image_size.x / image_size.y;
        float viewport_aspect_ratio = viewport_size.x / viewport_size.y;

        // Calculate the final display size.
        ImVec2 display_size = image_size;

        if (display_size.x > viewport_size.x || display_size.y > viewport_size.y) {
            if (viewport_aspect_ratio > image_aspect_ratio) {
                display_size.y = viewport_size.y;
                display_size.x = display_size.y * image_aspect_ratio;
            } else {
                display_size.x = viewport_size.x;
                display_size.y = display_size.x / image_aspect_ratio;
            }
        }

        // Calculate padding to center the image.
        ImVec2 padding = {(viewport_size.x - display_size.x) * 0.5f, (viewport_size.y - display_size.y) * 0.5f};

        // Apply padding.
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding.x);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);

        // Display the image.
        ImGui::Image(image.getTexture(), display_size);
    }
}}
