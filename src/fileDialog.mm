#include "fileDialog.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <AppKit/AppKit.h>


std::future<std::optional<std::string>> openFileDialogAsync(GLFWwindow* parent_window) {
    
    // Create a promise to return the file path
    std::shared_ptr<std::promise<std::optional<std::string>>> promise = std::make_shared<std::promise<std::optional<std::string>>>();
    auto future = promise->get_future();

    // Dispatch the file dialog to the main queue asynchronously
    dispatch_async(dispatch_get_main_queue(), [promise = std::move(promise), parent_window]() mutable {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanChooseDirectories:NO];
        [panel setCanChooseFiles:YES];

        // Retrieve the NSWindow from the GLFWwindow
        NSWindow* nsParentWindow = glfwGetCocoaWindow(parent_window);

        // Begin the sheet modal for the parent window
        [panel beginSheetModalForWindow:nsParentWindow completionHandler:^(NSModalResponse result) {
            if (result == NSModalResponseOK) {
                NSURL* selectedFile = [[panel URLs] firstObject];
                std::string filePath = std::string([[selectedFile path] UTF8String]);
                promise->set_value(std::optional<std::string>{filePath});
            } else {
                promise->set_value(std::nullopt);
            }
        }];
    });

    return future;
}