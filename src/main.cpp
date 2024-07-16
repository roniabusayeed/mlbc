#include "app.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <stdexcept>
#include <future>

#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include "colors.h"
#include "util.h"
#include "constants.h"
#include "docking.h"

struct UIFlags {
    bool ConfigureDirectories{ false };
};

enum class MediaType: int32_t {
    IMAGE = 0,
    AUDIO = 1
};

struct ConfigureDirectoriesData {
    std::string sourceDirectory { };
    std::string classADirectory { };
    std::string classBDirectory { };
    MediaType mediaType         {MediaType::IMAGE};
    std::string outputFilePath  { };
};

class MLBC : public App {
private:
    SDL_Window*                     m_window;
    SDL_GLContext                   m_gl_context;

    std::unique_ptr<ui::Theme>      m_theme;
    ImFont*                         m_ui_font;
    ImFont*                         m_ui_icon_regular_font;
    ImFont*                         m_ui_icon_solid_font;
    sf::Music                       m_music;
    UIFlags                         m_ui_flags;

    const char* WINDOW_FILES = "Files";
    const char* WINDOW_MEDIA_PREVIEW = "Media Preview";
    const char* WINDOW_MEDIA_EDITOR = "Media Editor";
    const char* WINDOW_CONFIGURE_DIRECTORIES = "Configure Directories";

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
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking.

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
        setupDockingLayout([=](ImGuiID& dockspace_id){
            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            // window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we
            // want (which ever one we DON'T set as NULL, will be returned by the function) out_id_at_dir is the id of the
            // node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction.
            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.35f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.4f, nullptr, &dockspace_id);
            auto dock_id_main = dockspace_id;

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow(WINDOW_FILES, dock_id_left);
            ImGui::DockBuilderDockWindow(WINDOW_MEDIA_EDITOR, dock_id_down);
            ImGui::DockBuilderDockWindow(WINDOW_MEDIA_PREVIEW, dock_id_main);
        });

        showMainMenuBar();
        
        // Docked windows.
        ImGuiWindowFlags docked_window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
        if (ImGui::Begin(WINDOW_FILES, nullptr, docked_window_flags)) {

        }
        ImGui::End();

        if (ImGui::Begin(WINDOW_MEDIA_PREVIEW, nullptr, docked_window_flags)) {

        }
        ImGui::End();

        if (ImGui::Begin(WINDOW_MEDIA_EDITOR, nullptr, docked_window_flags)) {

        }
        ImGui::End();

        // Other windows.
        if (m_ui_flags.ConfigureDirectories) {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            
            showConfigureDirectoriesWindow([](std::optional<ConfigureDirectoriesData> data) {
                if (!data) {
                    std::cout << "Cancelled directory configuration" << std::endl;
                } else {
                    std::cout << "Directory Configurations:" << std::endl;
                    std::cout << "Source: " << data->sourceDirectory << std::endl;
                    std::cout << "Class A: " << data->classADirectory << std::endl;
                    std::cout << "Class B: " << data->classBDirectory << std::endl;
                    std::cout << "Media Type: " << (data->mediaType == MediaType::IMAGE ? "Image" : "Audio") << std::endl;
                    std::cout << "Output: " << data->outputFilePath << std::endl;
                }
            });
        }
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
            ImGuiIO& io = ImGui::GetIO();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
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

private:
    void showMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Configure")) {
                if (ImGui::MenuItem("Directories")) { m_ui_flags.ConfigureDirectories = true; }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void showConfigureDirectoriesWindow(std::function<void(std::optional<ConfigureDirectoriesData>)> on_submit_callback) {
        static ConfigureDirectoriesData data;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        ImGui::SetNextWindowSize({600.0f, 400.0f}, ImGuiCond_Once);
        
        if (ImGui::Begin(WINDOW_CONFIGURE_DIRECTORIES, nullptr, window_flags)) {
            ImVec2 available_space = ImGui::GetContentRegionAvail();
            ImVec2 child_window_size = {available_space.x, available_space.y - ImGui::GetFrameHeightWithSpacing()};
            std::string child_window_label = WINDOW_CONFIGURE_DIRECTORIES + std::string("_CHILD");
            if (ImGui::BeginChild(child_window_label.c_str(), child_window_size)) {
                
                // Use a table to align the text boxes and buttons vertically.
                ImGuiTableFlags table_flags = ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;
                if (ImGui::BeginTable("ConfigureDirectoriesTable", 3, table_flags)) {

                    // Setup columns.
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed);

                    // Define spacings.
                    const float horizontal_spacing = 4.0f;
                    const float vertical_spacing = 4.0f;

                    // Row for Source directory.
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Source");
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::InputText("##sourceDirectory", &data.sourceDirectory);
                    ImGui::PopStyleVar();
                    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Dummy({horizontal_spacing, 0.0f});
                    ImGui::SameLine();
                    if (ImGui::Button("Select##sourceDirectory")) {
                        openFileDialogAsync(
                            [](std::optional<std::string> path) {
                                if (path) {
                                    data.sourceDirectory = *path;
                                }
                            },
                            true
                        );
                    }
                    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Dummy({0.0f, vertical_spacing});

                    // Row for Class A directory.
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Class A");
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::InputText("##classADirectory", &data.classADirectory);
                    ImGui::PopStyleVar();
                    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Dummy({horizontal_spacing, 0.0f});
                    ImGui::SameLine();
                    if (ImGui::Button("Select##classADirectory")) {
                        openFileDialogAsync(
                            [](std::optional<std::string> path) {
                                if (path) {
                                    data.classADirectory = *path;
                                }
                            },
                            true
                        );    
                    }
                    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Dummy({0.0f, vertical_spacing});

                    // Row for Class B directory.
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Class B");
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::InputText("##classBDirectory", &data.classBDirectory);
                    ImGui::PopStyleVar();
                    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Dummy({horizontal_spacing, 0.0f});
                    ImGui::SameLine();
                    if (ImGui::Button("Select##classBDirectory")) {
                        openFileDialogAsync(
                            [](std::optional<std::string> path) {
                                if (path) {
                                    data.classBDirectory = *path;
                                }
                            },
                            true
                        );
                    }
                    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Dummy({0.0f, vertical_spacing});

                    // Row for Media Type combo box
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Media Type");
                    
                    ImGui::TableSetColumnIndex(1);
                    const float COMBO_WIDTH = 100.0f;
                    ImGui::SetNextItemWidth(COMBO_WIDTH);
                    const char* items[] = { "Image", "Audio"};

                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    
                    // Pass in the preview value visible before opening the combo (it could technically be 
                    // different contents or not pulled from items[])
                    const char* combo_preview_value = items[static_cast<int>(data.mediaType)];
                    if (ImGui::BeginCombo("##mediaTypeCombo", combo_preview_value, ImGuiComboFlags_None)) {
                        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                        {
                            const bool is_selected = (static_cast<int>(data.mediaType) == n);
                            if (ImGui::Selectable(items[n], is_selected)) {
                                data.mediaType = static_cast<MediaType>(n);
                            }

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PopStyleVar();

                    ImGui::TableSetColumnIndex(2);
                    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Dummy({0.0f, vertical_spacing});

                    // Row for Output directory.
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Output");
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::InputText("##outputFilePath", &data.outputFilePath);
                    ImGui::PopStyleVar();

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Dummy({horizontal_spacing, 0.0f});
                    ImGui::SameLine();
                    if (ImGui::Button("Select##outputFilePath")) {
                        saveFileDialogAsync(
                            [](std::optional<std::string> path) {
                                if (path) {
                                    data.outputFilePath = *path;
                                }
                            },
                            m_window
                        );
                    }
                    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Dummy({0.0f, vertical_spacing});

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            // Align the two buttons to the right side
            const ImVec2 button_size = {60.0f, 0.0f};
            float total_buttons_width = button_size.x * 2 + ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - total_buttons_width);
            if (ImGui::Button("Cancel", button_size)) {
                m_ui_flags.ConfigureDirectories = false;
                on_submit_callback(std::nullopt);
            }
            ImGui::SameLine();
            if (ImGui::Button("Apply", button_size)) {

                // TODO: Validate the data.
                // If validation is successful, set  ConfigureDirectories flag to false.
                m_ui_flags.ConfigureDirectories = false;
                on_submit_callback(data);
            }
        }
        ImGui::End();
    }
};


int main() {
    const char*   APP_TITLE     = "MLBC";
    const int32_t APP_WDITH     = 1280;
    const int32_t APP_HEIGHT    = 720;

    MLBC mlbc(APP_TITLE, APP_WDITH, APP_HEIGHT);
    mlbc.run();
}
