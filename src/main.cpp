#include "app.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <stdexcept>
#include <future>

#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include "colors.h"
#include "util.h"
#include "constants.h"

class MLBC : public App {
private:
    SDL_Window* m_window;
    SDL_GLContext m_gl_context;

    std::unique_ptr<ui::Theme> m_theme;
    ImFont* m_ui_font;
    ImFont* m_ui_icon_regular_font;
    ImFont* m_ui_icon_solid_font;

    sf::Music m_music;

public:
    MLBC(const char* name, int32_t width, int32_t height) {
        // Setup SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
            throw std::runtime_error(std::string("error: couldn't initialize SDL: ") + SDL_GetError());
        }

        // Decide GL+GLSL versions
    #if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    #elif defined(__APPLE__)
        // GL 3.2 Core + GLSL 150
        const char* glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    #else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    #endif

        // From 2.0.18: Enable native IME.
    #ifdef SDL_HINT_IME_SHOW_UI
        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
    #endif

        // Create window with graphics context
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        m_window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
        if (m_window == nullptr) {
            throw std::runtime_error(std::string("error: SDL_CreateWindow(): ") + SDL_GetError());
        }

        m_gl_context = SDL_GL_CreateContext(m_window);
        SDL_GL_MakeCurrent(m_window, m_gl_context);
        SDL_GL_SetSwapInterval(1); // Enable vsync

        // TODO: Load OpenGL functions.

        // Set the background clear/refreh color.
        glClearColor(ui::COLOR_DARK_GREY.x, ui::COLOR_DARK_GREY.y, ui::COLOR_DARK_GREY.z, ui::COLOR_DARK_GREY.w);

        // Setup Dear ImGui context.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls.
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls.
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable docking.
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable multiple viewports.

        // Setup Dear ImGui style.
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends.
        ImGui_ImplSDL2_InitForOpenGL(m_window, m_gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version);
        
        // Load application theme.
        m_theme = ui::deserializeThemeFromJSON(APPLICATION_THEME_FILEPATH);

        // Setup ImGui style using the loaded theme.
        ui::setupImGuiStyleFromTheme(*m_theme);
        
        // Load application UI font.
        m_ui_font = ui::loadFont(APPLICATION_UI_FONT_FILEPATH, APPLICATION_UI_FONT_SIZE);

        // Load application UI icon font.
        m_ui_icon_regular_font = ui::loadIconFont(APPLICATION_UI_ICON_FONT_REGULAR_FILEPATH, APPLICATION_UI_FONT_SIZE);
        m_ui_icon_solid_font = ui::loadIconFont(APPLICATION_UI_ICON_FONT_SOLID_FILEPATH, APPLICATION_UI_FONT_SIZE);
    }

    ~MLBC() {
        
        // Free ImGui resources.
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        // Free SDL resources.
        SDL_GL_DeleteContext(m_gl_context);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    void startUp() override {

    }

    void update() override {
        
    }

    void run() override {
        startUp();

        bool done = false;
        while (!done) {
        
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT) {
                    done = true;
                }
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window)) {
                    done = true;
                }
            }

            // Start the Dear ImGui frame.
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            // Draw UI.
            ImGui::PushFont(m_ui_font);
            update();
            ImGui::PopFont();
            
            // Rendering.
            ImGui::Render();
            glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
                SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            }

            SDL_GL_SwapWindow(m_window);
        }
    }
};


int main() {
    const char*   APP_TITLE     = "MLBC";
    const int32_t APP_WDITH     = 600;
    const int32_t APP_HEIGHT    = 350;

    MLBC mlbc(APP_TITLE, APP_WDITH, APP_HEIGHT);
    mlbc.run();
}
