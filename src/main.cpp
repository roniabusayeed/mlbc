#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstdio>
#include <stdexcept>
#include <memory>

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include <SFML/Audio.hpp>

#include "app.h"
#include "util.h"
#include "colors.h"

class MLBCApp : public App {
private:
    GLFWwindow* m_window;
    std::unique_ptr<ui::Theme> m_theme;
    std::future<std::optional<std::string>> m_open_file_dialog_future;
    sf::Music music;

public:
    MLBCApp(const char* title, int32_t width, int32_t height) {
        glfwSetErrorCallback(glfwErrorCallback);
        if (!glfwInit()) {
            throw std::runtime_error("couldn't initialize GLFW library");
        }

        // Decide GL+GLSL versions
    #if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

        // Create window with graphics context
        m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (m_window == nullptr) {
            throw std::runtime_error("couldn't create GLFW window");
        }
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    ~MLBCApp() {
        
        // Free ImGui resources.
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        // Free GLFW resources.
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void startUp() override {
        
        // Load application theme.
        std::string theme_filepath = joinPaths(getExecutableDirectory(), "res/themes/dark-theme.json");
        std::unique_ptr<ui::Theme> theme = ui::deserializeThemeFromJSON(theme_filepath);
    }
    
    void update() override {
        ImGui::Begin("Window");
        if (ImGui::Button("Open")) {
            if (!m_open_file_dialog_future.valid()) {
                m_open_file_dialog_future = openFileDialogAsync(m_window);
            }
        }
        ImGui::End();

        if (m_open_file_dialog_future.valid() && isFutureReady(m_open_file_dialog_future)) {
            auto open_file_dialog_result = m_open_file_dialog_future.get();
            if (open_file_dialog_result.has_value()) {

                std::string filepath = open_file_dialog_result.value();
                if (!music.openFromFile(filepath)) {
                    std::cerr << "couldn't open file: " << filepath << std::endl;
                } else {
                    music.play();
                }
            }
        }
    }

    void run() override {
        startUp();

        while (! glfwWindowShouldClose(m_window)) {
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            update();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(m_window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(m_window);
        }
    }

private:
    static void glfwErrorCallback(int error, const char* description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }
};

int main() {
    const char*   APP_TITLE     = "MLBC";
    const int32_t APP_WDITH     = 800;
    const int32_t APP_HEIGHT    = 600;

    MLBCApp app(APP_TITLE, APP_WDITH, APP_HEIGHT);
    app.run();
}