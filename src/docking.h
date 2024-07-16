#pragma once
#include <functional>
#include <imgui.h>

void setupDockingLayout(std::function<void(ImGuiID&)> layout_builder);
