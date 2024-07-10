#pragma once

#include <future>
#include <optional>
#include <string>
#include <SDL2/SDL.h>

/**
 * @brief Asynchronously opens a file or folder selection dialog.
 *
 * @param callback A function that will be called with the selected file or folder path,
 * wrapped in std::optional<std::string>. If no file or folder is selected, std::nullopt will be passed.
 
 * @param directory If true, the dialog will allow selecting directories; otherwise, it will 
 * allow selecting files. Default is false.
 */
void openFileDialogAsync(std::function<void(std::optional<std::string>)> callback, bool directory = false);

/**
 * @brief Asynchronously opens a save file dialog.
 * 
 * @param callback A function that will be called with the selected file path,
 * wrapped in std::optional<std::string>. If no file is selected, std::nullopt will be passed.
 * 
 * @param parent_window The SDL_Window* that represents the parent window. The save dialog
 * will be modal to this window.
 */
void saveFileDialogAsync(std::function<void(std::optional<std::string>)> callback, SDL_Window* parent_window);
