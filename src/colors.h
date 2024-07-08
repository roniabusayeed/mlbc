#pragma once
#include <string>
#include <memory>
#include <glm/glm.hpp>


namespace ui
{
    struct Theme {
        glm::vec4 Text;
        glm::vec4 TextDisabled;
        glm::vec4 WindowBg;
        glm::vec4 ChildBg;
        glm::vec4 PopupBg;
        glm::vec4 Border;
        glm::vec4 BorderShadow;
        glm::vec4 FrameBg;
        glm::vec4 FrameBgHovered;
        glm::vec4 FrameBgActive;
        glm::vec4 TitleBg;
        glm::vec4 TitleBgActive;
        glm::vec4 TitleBgCollapsed;
        glm::vec4 MenuBarBg;
        glm::vec4 ScrollbarBg;
        glm::vec4 ScrollbarGrab;
        glm::vec4 ScrollbarGrabHovered;
        glm::vec4 ScrollbarGrabActive;
        glm::vec4 CheckMark;
        glm::vec4 SliderGrab;
        glm::vec4 SliderGrabActive;
        glm::vec4 Button;
        glm::vec4 ButtonHovered;
        glm::vec4 ButtonActive;
        glm::vec4 Header;
        glm::vec4 HeaderHovered;
        glm::vec4 HeaderActive;
        glm::vec4 Separator;
        glm::vec4 SeparatorHovered;
        glm::vec4 SeparatorActive;
        glm::vec4 ResizeGrip;
        glm::vec4 ResizeGripHovered;
        glm::vec4 ResizeGripActive;
        glm::vec4 Tab;
        glm::vec4 TabHovered;
        glm::vec4 TabSelected;
        glm::vec4 TabSelectedOverline;
        glm::vec4 TabDimmed;
        glm::vec4 TabDimmedSelected;
        glm::vec4 TabDimmedSelectedOverline;
        glm::vec4 PlotLines;
        glm::vec4 PlotLinesHovered;
        glm::vec4 PlotHistogram;
        glm::vec4 PlotHistogramHovered;
        glm::vec4 TableHeaderBg;
        glm::vec4 TableBorderStrong;
        glm::vec4 TableBorderLight;
        glm::vec4 TableRowBg;
        glm::vec4 TableRowBgAlt;
        glm::vec4 TextLink;
        glm::vec4 TextSelectedBg;
        glm::vec4 DragDropTarget;
        glm::vec4 NavHighlight;
        glm::vec4 NavWindowingHighlight;
        glm::vec4 NavWindowingDimBg;
        glm::vec4 ModalWindowDimBg;
    };

    std::unique_ptr<Theme> deserializeThemeFromJSON(const std::string& filepath);

    // Other constants.
    const glm::vec4 COLOR_DARK_GREY = {0.2f, 0.2f, 0.2f, 1.0f};

}   // namespace colors
