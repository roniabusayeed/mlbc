#include "fileDialog.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <AppKit/AppKit.h>

void openFileDialogAsync(std::function<void(std::optional<std::string>)> callback, bool directory) {
    
    // Dispatch the file dialog to the main queue asynchronously.
    dispatch_async(dispatch_get_main_queue(), [callback = std::move(callback), directory]() mutable {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanChooseDirectories:directory];
        [panel setCanChooseFiles:!directory];
        [panel setCanCreateDirectories:YES];

        // Run the panel in a non-modal way.
        NSInteger result = [panel runModal];

        if (result == NSModalResponseOK) {
            NSURL* selectedURL = [[panel URLs] firstObject];
            std::string path = std::string([[selectedURL path] UTF8String]);
            callback(std::optional<std::string>{path});
        } else {
            callback(std::nullopt);
        }
    });
}

void saveFileDialogAsync(std::function<void(std::optional<std::string>)> callback, SDL_Window* parent_window) {
    
    // Dispatch the file dialog to the main queue asynchronously.
    dispatch_async(dispatch_get_main_queue(), [callback = std::move(callback), parent_window]() mutable {
        NSSavePanel* panel = [NSSavePanel savePanel];

        // Set properties of the save panel.
        [panel setCanCreateDirectories:YES];
        [panel setShowsTagField:NO];

        // Retrieve the NSWindow from the SDL_Window.
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(parent_window, &wmInfo);
        NSWindow* nsParentWindow = wmInfo.info.cocoa.window;

        // Begin the sheet modal for the parent window.
        [panel beginSheetModalForWindow:nsParentWindow completionHandler:^(NSModalResponse result) {
            if (result == NSModalResponseOK) {
                NSURL* selectedFile = [panel URL];
                std::string filePath = std::string([[selectedFile path] UTF8String]);
                callback(std::optional<std::string>{filePath});
            } else {
                callback(std::nullopt);
            }
        }];
    });
}
