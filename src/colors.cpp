#include <memory>
#include <filesystem>
#include <fstream>

#include "util.h"
#include "colors.h"

namespace ui {
    std::unique_ptr<Theme> deserializeThemeFromJSON(const std::string& filepath) {

        // Error checking.
        if (! (std::filesystem::exists(filepath) && std::filesystem::is_regular_file(filepath))) {
            throw std::runtime_error("invalid file path: " + filepath);
        }

        // Open file for reading.
        std::ifstream infile(filepath);
        if (! infile.is_open()) {
            throw std::runtime_error("couldn't open file: " + filepath);
        }
        
        // Deserialization.
        std::unique_ptr<Theme> theme = std::make_unique<Theme>();
        json data;
        infile >> data;

        // Extract values from JSON object into a Theme struct.
        theme->Text                         = toGLMVec4(data["COLOR_TEXT"]);
        theme->TextDisabled                 = toGLMVec4(data["COLOR_TEXT_DISABLED"]);
        theme->WindowBg                     = toGLMVec4(data["COLOR_WINDOW_BG"]);
        theme->ChildBg                      = toGLMVec4(data["COLOR_CHILD_BG"]);
        theme->PopupBg                      = toGLMVec4(data["COLOR_POPUP_BG"]);
        theme->Border                       = toGLMVec4(data["COLOR_BORDER"]);
        theme->BorderShadow                 = toGLMVec4(data["COLOR_BORDER_SHADOW"]);
        theme->FrameBg                      = toGLMVec4(data["COLOR_FRAME_BG"]);
        theme->FrameBgHovered               = toGLMVec4(data["COLOR_FRAME_BG_HOVERED"]);
        theme->FrameBgActive                = toGLMVec4(data["COLOR_FRAME_BG_ACTIVE"]);
        theme->TitleBg                      = toGLMVec4(data["COLOR_TITLE_BG"]);
        theme->TitleBgActive                = toGLMVec4(data["COLOR_TITLE_BG_ACTIVE"]);
        theme->TitleBgCollapsed             = toGLMVec4(data["COLOR_TITLE_BG_COLLAPSED"]);
        theme->MenuBarBg                    = toGLMVec4(data["COLOR_MENU_BAR_BG"]);
        theme->ScrollbarBg                  = toGLMVec4(data["COLOR_SCROLLBAR_BG"]);
        theme->ScrollbarGrab                = toGLMVec4(data["COLOR_SCROLLBAR_GRAB"]);
        theme->ScrollbarGrabHovered         = toGLMVec4(data["COLOR_SCROLLBAR_GRAB_HOVERED"]);
        theme->ScrollbarGrabActive          = toGLMVec4(data["COLOR_SCROLLBAR_GRAB_ACTIVE"]);
        theme->CheckMark                    = toGLMVec4(data["COLOR_CHECK_MARK"]);
        theme->SliderGrab                   = toGLMVec4(data["COLOR_SLIDER_GRAB"]);
        theme->SliderGrabActive             = toGLMVec4(data["COLOR_SLIDER_GRAB_ACTIVE"]);
        theme->Button                       = toGLMVec4(data["COLOR_BUTTON"]);
        theme->ButtonHovered                = toGLMVec4(data["COLOR_BUTTON_HOVERED"]);
        theme->ButtonActive                 = toGLMVec4(data["COLOR_BUTTON_ACTIVE"]);
        theme->Header                       = toGLMVec4(data["COLOR_HEADER"]);
        theme->HeaderHovered                = toGLMVec4(data["COLOR_HEADER_HOVERED"]);
        theme->HeaderActive                 = toGLMVec4(data["COLOR_HEADER_ACTIVE"]);
        theme->Separator                    = toGLMVec4(data["COLOR_SEPARATOR"]);
        theme->SeparatorHovered             = toGLMVec4(data["COLOR_SEPARATOR_HOVERED"]);
        theme->SeparatorActive              = toGLMVec4(data["COLOR_SEPARATOR_ACTIVE"]);
        theme->ResizeGrip                   = toGLMVec4(data["COLOR_RESIZE_GRIP"]);
        theme->ResizeGripHovered            = toGLMVec4(data["COLOR_RESIZE_GRIP_HOVERED"]);
        theme->ResizeGripActive             = toGLMVec4(data["COLOR_RESIZE_GRIP_ACTIVE"]);
        theme->Tab                          = toGLMVec4(data["COLOR_TAB"]);
        theme->TabHovered                   = toGLMVec4(data["COLOR_TAB_HOVERED"]);
        theme->TabSelected                  = toGLMVec4(data["COLOR_TAB_SELECTED"]);
        theme->TabSelectedOverline          = toGLMVec4(data["COLOR_TAB_SELECTED_OVERLINE"]);
        theme->TabDimmed                    = toGLMVec4(data["COLOR_TAB_DIMMED"]);
        theme->TabDimmedSelected            = toGLMVec4(data["COLOR_TAB_DIMMED_SELECTED"]);
        theme->TabDimmedSelectedOverline    = toGLMVec4(data["COLOR_TAB_DIMMED_SELECTED_OVERLINE"]);
        theme->PlotLines                    = toGLMVec4(data["COLOR_PLOT_LINES"]);
        theme->PlotLinesHovered             = toGLMVec4(data["COLOR_PLOT_LINES_HOVERED"]);
        theme->PlotHistogram                = toGLMVec4(data["COLOR_PLOT_HISTOGRAM"]);
        theme->PlotHistogramHovered         = toGLMVec4(data["COLOR_PLOT_HISTOGRAM_HOVERED"]);
        theme->TableHeaderBg                = toGLMVec4(data["COLOR_TABLE_HEADER_BG"]);
        theme->TableBorderStrong            = toGLMVec4(data["COLOR_TABLE_BORDER_STRONG"]);
        theme->TableBorderLight             = toGLMVec4(data["COLOR_TABLE_BORDER_LIGHT"]);
        theme->TableRowBg                   = toGLMVec4(data["COLOR_TABLE_ROW_BG"]);
        theme->TableRowBgAlt                = toGLMVec4(data["COLOR_TABLE_ROW_BG_ALT"]);
        theme->TextLink                     = toGLMVec4(data["COLOR_TEXT_LINK"]);
        theme->TextSelectedBg               = toGLMVec4(data["COLOR_TEXT_SELECTED_BG"]);
        theme->DragDropTarget               = toGLMVec4(data["COLOR_DRAG_DROP_TARGET"]);
        theme->NavHighlight                 = toGLMVec4(data["COLOR_NAV_HIGHLIGHT"]);
        theme->NavWindowingHighlight        = toGLMVec4(data["COLOR_NAV_WINDOWING_HIGHLIGHT"]);
        theme->NavWindowingDimBg            = toGLMVec4(data["COLOR_NAV_WINDOWING_DIM_BG"]);
        theme->ModalWindowDimBg             = toGLMVec4(data["COLOR_MODAL_WINDOW_DIM_BG"]);

        return theme;
    }

    void setupImGuiStyleFromTheme(const Theme& theme, float alpha_threshold) {
        ImGuiStyle& style = ImGui::GetStyle();

        // Set style colors (from Theme).
        style.Colors[ImGuiCol_Text]                         = toImVec4(theme.Text);
        style.Colors[ImGuiCol_TextDisabled]                 = toImVec4(theme.TextDisabled);
        style.Colors[ImGuiCol_WindowBg]                     = toImVec4(theme.WindowBg);
        style.Colors[ImGuiCol_ChildBg]                      = toImVec4(theme.ChildBg);
        style.Colors[ImGuiCol_PopupBg]                      = toImVec4(theme.PopupBg);
        style.Colors[ImGuiCol_Border]                       = toImVec4(theme.Border);
        style.Colors[ImGuiCol_BorderShadow]                 = toImVec4(theme.BorderShadow);
        style.Colors[ImGuiCol_FrameBg]                      = toImVec4(theme.FrameBg);
        style.Colors[ImGuiCol_FrameBgHovered]               = toImVec4(theme.FrameBgHovered);
        style.Colors[ImGuiCol_FrameBgActive]                = toImVec4(theme.FrameBgActive);
        style.Colors[ImGuiCol_TitleBg]                      = toImVec4(theme.TitleBg);
        style.Colors[ImGuiCol_TitleBgActive]                = toImVec4(theme.TitleBgActive);
        style.Colors[ImGuiCol_TitleBgCollapsed]             = toImVec4(theme.TitleBgCollapsed);
        style.Colors[ImGuiCol_MenuBarBg]                    = toImVec4(theme.MenuBarBg);
        style.Colors[ImGuiCol_ScrollbarBg]                  = toImVec4(theme.ScrollbarBg);
        style.Colors[ImGuiCol_ScrollbarGrab]                = toImVec4(theme.ScrollbarGrab);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]         = toImVec4(theme.ScrollbarGrabHovered);
        style.Colors[ImGuiCol_ScrollbarGrabActive]          = toImVec4(theme.ScrollbarGrabActive);
        style.Colors[ImGuiCol_CheckMark]                    = toImVec4(theme.CheckMark);
        style.Colors[ImGuiCol_SliderGrab]                   = toImVec4(theme.SliderGrab);
        style.Colors[ImGuiCol_SliderGrabActive]             = toImVec4(theme.SliderGrabActive);
        style.Colors[ImGuiCol_Button]                       = toImVec4(theme.Button);
        style.Colors[ImGuiCol_ButtonHovered]                = toImVec4(theme.ButtonHovered);
        style.Colors[ImGuiCol_ButtonActive]                 = toImVec4(theme.ButtonActive);
        style.Colors[ImGuiCol_Header]                       = toImVec4(theme.Header);
        style.Colors[ImGuiCol_HeaderHovered]                = toImVec4(theme.HeaderHovered);
        style.Colors[ImGuiCol_HeaderActive]                 = toImVec4(theme.HeaderActive);
        style.Colors[ImGuiCol_Separator]                    = toImVec4(theme.Separator);
        style.Colors[ImGuiCol_SeparatorHovered]             = toImVec4(theme.SeparatorHovered);
        style.Colors[ImGuiCol_SeparatorActive]              = toImVec4(theme.SeparatorActive);
        style.Colors[ImGuiCol_ResizeGrip]                   = toImVec4(theme.ResizeGrip);
        style.Colors[ImGuiCol_ResizeGripHovered]            = toImVec4(theme.ResizeGripHovered);
        style.Colors[ImGuiCol_ResizeGripActive]             = toImVec4(theme.ResizeGripActive);
        style.Colors[ImGuiCol_Tab]                          = toImVec4(theme.Tab);
        style.Colors[ImGuiCol_TabHovered]                   = toImVec4(theme.TabHovered);
        style.Colors[ImGuiCol_TabSelected]                  = toImVec4(theme.TabSelected);
        style.Colors[ImGuiCol_TabSelectedOverline]          = toImVec4(theme.TabSelectedOverline);
        style.Colors[ImGuiCol_TabDimmed]                    = toImVec4(theme.TabDimmed);
        style.Colors[ImGuiCol_TabDimmedSelected]            = toImVec4(theme.TabDimmedSelected);
        style.Colors[ImGuiCol_TabDimmedSelectedOverline]    = toImVec4(theme.TabDimmedSelectedOverline);
        style.Colors[ImGuiCol_PlotLines]                    = toImVec4(theme.PlotLines);
        style.Colors[ImGuiCol_PlotLinesHovered]             = toImVec4(theme.PlotLinesHovered);
        style.Colors[ImGuiCol_PlotHistogram]                = toImVec4(theme.PlotHistogram);
        style.Colors[ImGuiCol_PlotHistogramHovered]         = toImVec4(theme.PlotHistogramHovered);
        style.Colors[ImGuiCol_TableHeaderBg]                = toImVec4(theme.TableHeaderBg);
        style.Colors[ImGuiCol_TableBorderStrong]            = toImVec4(theme.TableBorderStrong);
        style.Colors[ImGuiCol_TableBorderLight]             = toImVec4(theme.TableBorderLight);
        style.Colors[ImGuiCol_TableRowBg]                   = toImVec4(theme.TableRowBg);
        style.Colors[ImGuiCol_TableRowBgAlt]                = toImVec4(theme.TableRowBgAlt);
        style.Colors[ImGuiCol_TextLink]                     = toImVec4(theme.TextLink);
        style.Colors[ImGuiCol_TextSelectedBg]               = toImVec4(theme.TextSelectedBg);
        style.Colors[ImGuiCol_DragDropTarget]               = toImVec4(theme.DragDropTarget);
        style.Colors[ImGuiCol_NavHighlight]                 = toImVec4(theme.NavHighlight);
        style.Colors[ImGuiCol_NavWindowingHighlight]        = toImVec4(theme.NavWindowingHighlight);
        style.Colors[ImGuiCol_NavWindowingDimBg]            = toImVec4(theme.NavWindowingDimBg);
        style.Colors[ImGuiCol_ModalWindowDimBg]             = toImVec4(theme.ModalWindowDimBg);

        // Adjusts the alpha values of the ImGui colors based on the alpha threshold.
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const auto color_id = static_cast<ImGuiCol>(i);
            auto& color = style.Colors[i];
            if (
                (color_id != ImGuiCol_NavWindowingDimBg) &&
                (color_id != ImGuiCol_ModalWindowDimBg) &&
                (color.w < 1.0f || color_id == ImGuiCol_FrameBg || color_id == ImGuiCol_WindowBg || color_id == ImGuiCol_ChildBg)
            ) {
                color.w *= alpha_threshold;
            }
        }

        // Set the border sizes and rounding.
        // TODO: Consider exposing the following styles to the Theme API.
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.WindowBorderSize = 1.0f;
        
        style.FrameRounding = 3.0f;
        style.GrabRounding = 3.0f;
        style.PopupRounding = 3.0f;
        style.GrabRounding = 3.0f;
        style.WindowRounding = 10.0f;
        
        style.Alpha = 1.0f;
    }

    void Theme::pushColor(ImGuiCol idx, const glm::vec4& color) {
        ImGui::PushStyleColor(idx, toImVec4(color));
        m_pushed_color_count++;
    }

    void Theme::push() {
        pushColor(ImGuiCol_Text, Text);
        pushColor(ImGuiCol_TextDisabled, TextDisabled);
        pushColor(ImGuiCol_WindowBg, WindowBg);
        pushColor(ImGuiCol_ChildBg, ChildBg);
        pushColor(ImGuiCol_PopupBg, PopupBg);
        pushColor(ImGuiCol_Border, Border);
        pushColor(ImGuiCol_BorderShadow, BorderShadow);
        pushColor(ImGuiCol_FrameBg, FrameBg);
        pushColor(ImGuiCol_FrameBgHovered, FrameBgHovered);
        pushColor(ImGuiCol_FrameBgActive, FrameBgActive);
        pushColor(ImGuiCol_TitleBg, TitleBg);
        pushColor(ImGuiCol_TitleBgActive, TitleBgActive);
        pushColor(ImGuiCol_TitleBgCollapsed, TitleBgCollapsed);
        pushColor(ImGuiCol_MenuBarBg, MenuBarBg);
        pushColor(ImGuiCol_ScrollbarBg, ScrollbarBg);
        pushColor(ImGuiCol_ScrollbarGrab, ScrollbarGrab);
        pushColor(ImGuiCol_ScrollbarGrabHovered, ScrollbarGrabHovered);
        pushColor(ImGuiCol_ScrollbarGrabActive, ScrollbarGrabActive);
        pushColor(ImGuiCol_CheckMark, CheckMark);
        pushColor(ImGuiCol_SliderGrab, SliderGrab);
        pushColor(ImGuiCol_SliderGrabActive, SliderGrabActive);
        pushColor(ImGuiCol_Button, Button);
        pushColor(ImGuiCol_ButtonHovered, ButtonHovered);
        pushColor(ImGuiCol_ButtonActive, ButtonActive);
        pushColor(ImGuiCol_Header, Header);
        pushColor(ImGuiCol_HeaderHovered, HeaderHovered);
        pushColor(ImGuiCol_HeaderActive, HeaderActive);
        pushColor(ImGuiCol_Separator, Separator);
        pushColor(ImGuiCol_SeparatorHovered, SeparatorHovered);
        pushColor(ImGuiCol_SeparatorActive, SeparatorActive);
        pushColor(ImGuiCol_ResizeGrip, ResizeGrip);
        pushColor(ImGuiCol_ResizeGripHovered, ResizeGripHovered);
        pushColor(ImGuiCol_ResizeGripActive, ResizeGripActive);
        pushColor(ImGuiCol_Tab, Tab);
        pushColor(ImGuiCol_TabHovered, TabHovered);
        pushColor(ImGuiCol_TabSelected, TabSelected);
        pushColor(ImGuiCol_TabSelectedOverline, TabSelectedOverline);
        pushColor(ImGuiCol_TabDimmed, TabDimmed);
        pushColor(ImGuiCol_TabDimmedSelected, TabDimmedSelected);
        pushColor(ImGuiCol_TabDimmedSelectedOverline, TabDimmedSelectedOverline);
        pushColor(ImGuiCol_PlotLines, PlotLines);
        pushColor(ImGuiCol_PlotLinesHovered, PlotLinesHovered);
        pushColor(ImGuiCol_PlotHistogram, PlotHistogram);
        pushColor(ImGuiCol_PlotHistogramHovered, PlotHistogramHovered);
        pushColor(ImGuiCol_TableHeaderBg, TableHeaderBg);
        pushColor(ImGuiCol_TableBorderStrong, TableBorderStrong);
        pushColor(ImGuiCol_TableBorderLight, TableBorderLight);
        pushColor(ImGuiCol_TableRowBg, TableRowBg);
        pushColor(ImGuiCol_TableRowBgAlt, TableRowBgAlt);
        pushColor(ImGuiCol_TextLink, TextLink);
        pushColor(ImGuiCol_TextSelectedBg, TextSelectedBg);
        pushColor(ImGuiCol_DragDropTarget, DragDropTarget);
        pushColor(ImGuiCol_NavHighlight, NavHighlight);
        pushColor(ImGuiCol_NavWindowingHighlight, NavWindowingHighlight);
        pushColor(ImGuiCol_NavWindowingDimBg, NavWindowingDimBg);
        pushColor(ImGuiCol_ModalWindowDimBg, ModalWindowDimBg);

        m_push_count++;
    }

    void Theme::pop() {
        ImGui::PopStyleColor(m_pushed_color_count);
        m_pushed_color_count = 0;

        m_push_count--;
    }

    Theme::~Theme() {
        if (m_push_count != 0) {
            std::runtime_error("mismatched number of Theme::push() and Theme::pop() calls");
        }
    }

    // Function to get a contrasting grayscale text color
    glm::vec4 getContrastingTextColor(const glm::vec4& backgroundColor) {
        
        // Helper lambda to calculate the relative luminance of a color
        auto channelLuminance = [](float channel) -> float {
            return (channel <= 0.03928f) ? (channel / 12.92f) : (std::pow((channel + 0.055f) / 1.055f, 2.4f));
        };

        // Calculate the relative luminance of the background color
        float backgroundLuminance = 0.2126f * channelLuminance(backgroundColor.r) + 
                                    0.7152f * channelLuminance(backgroundColor.g) + 
                                    0.0722f * channelLuminance(backgroundColor.b);

        // Choose white or black based on the background luminance
        if (backgroundLuminance > 0.5f) {
            return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Dark text for light background
        } else {
            return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Light text for dark background
        }
    }
}
