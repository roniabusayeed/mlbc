#pragma once

#include <future>
#include <optional>
#include <string>
#include <GLFW/glfw3.h>

std::future<std::optional<std::string>> openFileDialogAsync(GLFWwindow* parent_window);
