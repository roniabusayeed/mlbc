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
}