#pragma once

#include <future>
#include <optional>
#include <string>
#include <SDL2/SDL.h>

std::future<std::optional<std::string>> openFileDialogAsync(bool directory = false);
std::future<std::optional<std::string>> saveFileDialogAsync(SDL_Window* parent_window);
