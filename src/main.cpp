#include "app.h"

#include <glad/glad.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <stdexcept>
#include <future>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include <csv2.hpp>

#include "colors.h"
#include "util.h"
#include "constants.h"
#include "docking.h"
#include "fileWatcher.h"
#include "image.h"
#include "widgets.h"

struct UIFlags {
    bool ConfigureDirectories{ false };
};

struct DirectoryConfiguration {
    std::string sourceDirectory { };
    std::string classADirectory { };
    std::string classBDirectory { };
    MediaType mediaType         {MediaType::Image};
    std::string outputFilePath  { };
};

class MLBC : public App {
private:
    bool                                                            m_application_should_close{ false };

    SDL_Window*                                                     m_window;
    SDL_GLContext                                                   m_gl_context;

    std::unique_ptr<ui::Theme>                                      m_theme;
    ImFont*                                                         m_ui_font;
    ImFont*                                                         m_ui_icon_regular_font;
    ImFont*                                                         m_ui_icon_solid_font;
    sf::Music                                                       m_music;
    UIFlags                                                         m_ui_flags;

    std::optional<std::vector<std::string>>                         m_media_sources;
    std::optional<std::vector<std::string>>                         m_media_class_a;
    std::optional<std::vector<std::string>>                         m_media_class_b;

    std::unique_ptr<Watcher>                                        m_media_sources_watcher;
    std::unique_ptr<Watcher>                                        m_media_class_a_watcher;
    std::unique_ptr<Watcher>                                        m_media_class_b_watcher;

    std::optional<DirectoryConfiguration>                           m_directory_configuration;

    std::optional<Image>                                            m_current_media_image_preview;
    std::optional<std::string>                                      m_current_media_filepath;

    glm::vec4                                                       m_preview_bg_color;

    float                                                           m_bias_value{ 0.0f };
    float                                                           m_bias_sensitivity{ 0.1f };

    // Mutexes to protect shared resources.
    std::mutex                                                      mutex_media_sources;
    std::mutex                                                      mutex_media_class_a;
    std::mutex                                                      mutex_media_class_b;

    const char* WINDOW_FILES = "Files";
    const char* WINDOW_MEDIA_PREVIEW = "Media Preview";
    const char* WINDOW_MEDIA_EDITOR = "Media Editor";
    const char* WINDOW_CONFIGURE_DIRECTORIES = "Configure Directories";

    const char* ERROR_MOVING_FILE_POPUP = "Error Moving File";
    std::string m_error_moving_file_pop_up_message;

    bool m_keyboard_label_button_pressed{false };

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
        if (! m_window) {
            SDL_Quit();
            std::cerr << std::string("error: SDL_CreateWindow(): ") + SDL_GetError() << std::endl;
            exit(EXIT_FAILURE);
        }

        m_gl_context = SDL_GL_CreateContext(m_window);
        if (! m_gl_context) {
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            std::cerr << std::string("error: SDL_GL_CreateContext(): ") + SDL_GetError() << std::endl;
            exit(EXIT_FAILURE);
        }
        SDL_GL_MakeCurrent(m_window, m_gl_context);
        SDL_GL_SetSwapInterval(1); // Enable vsync

        // Initialize GLAD.
        if (! gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            SDL_GL_DeleteContext(m_gl_context);
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            std::cerr << "error: Failed to initialize GLAD" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Set the background clear/refresh color.
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

        // Initially the preview background color is the same as the window background color.
        m_preview_bg_color = m_theme->WindowBg;
    }

    ~MLBC() override {
        
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
            // window ID to split, direction, fraction (between 0 and 1), the final two setting let us choose which id we
            // want (whichever one we DON'T set as NULL, will be returned by the function) out_id_at_dir is the id of the
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
        ImGuiWindowFlags docked_window_flags = ImGuiWindowFlags_None;
        docked_window_flags |= ImGuiWindowFlags_NoMove;
        docked_window_flags |= ImGuiWindowFlags_NoResize;
        docked_window_flags |= ImGuiWindowFlags_NoCollapse;
        docked_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        if (ImGui::Begin(WINDOW_MEDIA_EDITOR, nullptr, docked_window_flags)) {

            // Calculate layout measurements.
            float available_width = ImGui::GetContentRegionAvail().x;
            float button_width = 60.0f;
            float horizontal_item_spacing = ImGui::GetStyle().ItemSpacing.x;
            float slider_width = available_width - button_width - horizontal_item_spacing;
            
            if (! m_current_media_filepath) { 
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            // Bias slider.
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushItemWidth(slider_width);
            ImGui::SliderFloat("###bias-slider-float", &m_bias_value, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::PopItemWidth();
            ImGui::PopStyleVar();

            ImGui::SameLine();
            
            // Label button.
            bool label_button_clicked = ImGui::Button("Label###label-button", {button_width, 0.0f});

            // Bias sensitivity slider.
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushItemWidth(slider_width / 4.0f);
            ImGui::SliderFloat("###bias-sensitivity-slider-float", &m_bias_sensitivity, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::PopItemWidth();
            ImGui::PopStyleVar();
            
            if (! m_current_media_filepath) {
                ImGui::PopStyleVar();
                ImGui::PopItemFlag();
            }

            // Preview background color picker.
            ImGuiColorEditFlags preview_bg_color_edit_flags = ImGuiColorEditFlags_None;
            preview_bg_color_edit_flags |= ImGuiColorEditFlags_NoInputs;
            preview_bg_color_edit_flags |= ImGuiColorEditFlags_AlphaBar;
            ImGui::ColorEdit4("Preview Background", glm::value_ptr(m_preview_bg_color), preview_bg_color_edit_flags);

            // Label button click handler.
            if (label_button_clicked) {
                labelButtonClickHandler();
            }

            if (m_keyboard_label_button_pressed) {
                if (m_current_media_filepath) {
                    labelButtonClickHandler();
                }
                m_keyboard_label_button_pressed = false;    // Reset the value for next use.
            }

            // Error moving file popup.
            ImGui::SetNextWindowSize(toImVec2(ERROR_POPUP_DIALOG_WINDOW_SIZE));
            if (ImGui::BeginPopupModal(ERROR_MOVING_FILE_POPUP, nullptr, ImGuiWindowFlags_NoResize)) {
                ImGui::TextWrapped("%s", m_error_moving_file_pop_up_message.c_str());
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing());
                if (ImGui::Button("OK", {ImGui::GetContentRegionAvail().x, 0.0f}) ) { ImGui::CloseCurrentPopup(); }
                ImGui::SetItemDefaultFocus();
                ImGui::EndPopup();
            }
        }
        ImGui::End();
        
        if (ImGui::Begin(WINDOW_FILES, nullptr, docked_window_flags)) {
            if (m_directory_configuration) {
                
                // Build header labels.

                // Build header label for media sources.
                size_t media_sources_count = 0;
                {
                    std::lock_guard<std::mutex> lock(mutex_media_sources);
                    media_sources_count = m_media_sources->size();
                }
                std::stringstream media_sources_header_label_ss;
                media_sources_header_label_ss << "Source  [" << media_sources_count << "]";
                media_sources_header_label_ss << "###media-sources-header";
                std::string media_sources_header_label = media_sources_header_label_ss.str();

                // Build header label for media class a.
                size_t media_class_a_count = 0;
                {
                    std::lock_guard<std::mutex> lock(mutex_media_class_a);
                    media_class_a_count = m_media_class_a->size();
                }
                std::stringstream media_class_a_header_label_ss;
                media_class_a_header_label_ss << "Class A [" << media_class_a_count << "]";
                media_class_a_header_label_ss << "###media-class-a-header";
                std::string media_class_a_header_label = media_class_a_header_label_ss.str();

                // Build header label for media class b.
                size_t media_class_b_count = 0;
                {
                    std::lock_guard<std::mutex> lock(mutex_media_class_b);
                    media_class_b_count = m_media_class_b->size();
                }
                std::stringstream media_class_b_header_label_ss;
                media_class_b_header_label_ss << "Class B [" << media_class_b_count << "]";
                media_class_b_header_label_ss << "###media-class-b-header";
                std::string media_class_b_header_label = media_class_b_header_label_ss.str();

                // Source.
                if (ImGui::CollapsingHeader(media_sources_header_label.c_str())) {
                    std::lock_guard<std::mutex> lock(mutex_media_sources);
                    if (m_media_sources.has_value()) {
                        ImGui::Indent();
                        filesListView(
                            m_media_sources.value(),
                            m_directory_configuration->mediaType,
                            [this](const std::vector<std::string>& filepaths, MediaType media_type, int selected_index){
                                loadCurrentPreviewAndFilepath(filepaths.at(selected_index));
                            }
                        );
                        ImGui::Unindent();
                    }  
                }

                // Class A.
                if (ImGui::CollapsingHeader(media_class_a_header_label.c_str())) {
                    std::lock_guard<std::mutex> lock(mutex_media_class_a);
                    if (m_media_class_a.has_value()) {
                        ImGui::Indent();
                        filesListView(
                            m_media_class_a.value(),
                            m_directory_configuration->mediaType,
                            [this](const std::vector<std::string>& filepaths, MediaType media_type, int selected_index){
                                loadCurrentPreviewAndFilepath(filepaths.at(selected_index));
                            }
                        );
                        ImGui::Unindent();
                    }
                }

                if (ImGui::CollapsingHeader(media_class_b_header_label.c_str())) {
                    std::lock_guard<std::mutex> lock(mutex_media_class_b);
                    if (m_media_class_b.has_value()) {
                        ImGui::Indent();
                        filesListView(
                            m_media_class_b.value(),
                            m_directory_configuration->mediaType,
                            [this](const std::vector<std::string>& filepaths, MediaType media_type, int selected_index){
                                loadCurrentPreviewAndFilepath(filepaths.at(selected_index));
                            }
                        );
                        ImGui::Unindent();
                    }
                }
            }
        }
        ImGui::End();

        ImGui::PushStyleColor(ImGuiCol_WindowBg, toImVec4(m_preview_bg_color));
        if (ImGui::Begin(WINDOW_MEDIA_PREVIEW, nullptr, docked_window_flags | ImGuiWindowFlags_NoNav)) {
            if (m_directory_configuration.has_value()) {
                if (m_directory_configuration->mediaType == MediaType::Image) {
                    if (m_current_media_filepath && m_current_media_image_preview) {
                        SelectableText(m_current_media_filepath.value());
                        ui::widget::ImageView(m_current_media_image_preview.value());
                    }
                } 
                else if (m_directory_configuration->mediaType == MediaType::Audio) {

                }
            }
        }
        ImGui::End();
        ImGui::PopStyleColor();

        // Other windows.
        if (m_ui_flags.ConfigureDirectories) {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            
            showConfigureDirectoriesWindow([this](std::optional<DirectoryConfiguration> data) {
                if (data) {

                    m_directory_configuration = data;

                    // Initially load (asynchronously) the configured directories when a 
                    // directory configuration is set.

                    std::future<void> media_sources_future = std::async([this, data] {
                        std::lock_guard<std::mutex> lock(mutex_media_sources);
                        m_media_sources = loadMediaFiles(data->sourceDirectory, data->mediaType);
                    });

                    std::future<void> media_class_a_future = std::async([this, data] {
                        std::lock_guard<std::mutex> lock(mutex_media_class_a);
                        m_media_class_a = loadMediaFiles(data->classADirectory, data->mediaType);
                    });

                    std::future<void> media_class_b_future = std::async([this, data] {
                        std::lock_guard<std::mutex> lock(mutex_media_class_b);
                        m_media_class_b = loadMediaFiles(data->classBDirectory, data->mediaType);
                    });

                    // Set up watchers to asynchronously watch the configured directories for changes.
                    // If there are any changes to the configured directories, reload them.

                    try {
                        m_media_sources_watcher  = std::make_unique<Watcher>(
                            data->sourceDirectory,
                            [this, data](const std::string& directory, const std::string& file, efsw::Action action, const std::string& old_file) {
                                std::lock_guard<std::mutex> lock(mutex_media_sources);
                                m_media_sources = loadMediaFiles(data->sourceDirectory, data->mediaType);
                            }
                        );
                    } catch (const std::runtime_error& re) {
                        std::cerr << re.what() << std::endl;
                    }

                    try {
                        m_media_class_a_watcher  = std::make_unique<Watcher>(
                            data->classADirectory,
                            [this, data](const std::string& directory, const std::string& file, efsw::Action action, const std::string& old_file) {
                                std::lock_guard<std::mutex> lock(mutex_media_class_a);
                                m_media_class_a = loadMediaFiles(data->classADirectory, data->mediaType);
                            }
                        );
                    } catch (const std::runtime_error& re) {
                        std::cerr << re.what() << std::endl;
                    }
                    
                    try {
                        m_media_class_b_watcher  = std::make_unique<Watcher>(
                            data->classBDirectory,
                            [this, data](const std::string& directory, const std::string& file, efsw::Action action, const std::string& old_file) {
                                std::lock_guard<std::mutex> lock(mutex_media_class_b);
                                m_media_class_b = loadMediaFiles(data->classBDirectory, data->mediaType);
                            }
                        );
                    } catch (const std::runtime_error& re) {
                        std::cerr << re.what() << std::endl;
                    }

                    // Wait on the futures to finish.
                    media_sources_future.get();
                    media_class_a_future.get();
                    media_class_b_future.get();

                    // Load the first media source (if any) for preview.
                    {
                        std::lock_guard<std::mutex> lock(mutex_media_sources);
                        if (! m_media_sources->empty()) {
                            loadCurrentPreviewAndFilepath(m_media_sources->front());
                        }
                    }
                }
            });
        }
    }

    void run() override {
        startUp();

        m_application_should_close = false;
        while (!m_application_should_close) {
        
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT) {
                    m_application_should_close = true;
                }
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window)) {
                    m_application_should_close = true;
                }
                if (event.type == SDL_KEYDOWN) {
                    handleKeyPress(event.key.keysym.sym);
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
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Close Directories")) {
                    {
                        std::lock_guard<std::mutex> lock(mutex_media_sources);
                        m_media_sources = std::nullopt;
                    }
                    
                    {
                        std::lock_guard<std::mutex> lock(mutex_media_class_a);
                        m_media_class_a = std::nullopt;
                    }
                    
                    {
                        std::lock_guard<std::mutex> lock(mutex_media_class_b);
                        m_media_class_b = std::nullopt;
                    }

                    m_directory_configuration = std::nullopt;

                    m_media_sources_watcher = nullptr;
                    m_media_class_a_watcher = nullptr;
                    m_media_class_b_watcher = nullptr;
                }
                if (ImGui::MenuItem("Close Preview")) {
                    clearCurrentPreviewAndFilepath();
                }
                if (ImGui::MenuItem("Exit")) { m_application_should_close = true; }
                ImGui::EndMenu();
            }
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

    void showConfigureDirectoriesWindow(std::function<void(std::optional<DirectoryConfiguration>)> on_submit_callback) {
        static DirectoryConfiguration data;

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

            static std::optional<std::string> message;
            const char* INVALID_DIRECTORY_CONFIGURATION_POPUP = "Invalid Directory Configuration";

            if (ImGui::Button("Apply", button_size)) {
                auto validation_result = validateDirectoryConfiguration(data);
                bool success = validation_result.first;
                message = validation_result.second;

                if (!success) {
                    ImGui::OpenPopup(INVALID_DIRECTORY_CONFIGURATION_POPUP);
                } else {

                    // If the outputFilename doesn't have .csv extension, add one.
                    const char* CSV_EXT = ".csv";
                    if (! data.outputFilePath.ends_with(CSV_EXT)) {
                        data.outputFilePath.append(CSV_EXT);
                    }

                    // If validation is successful, set  ConfigureDirectories flag to false.
                    m_ui_flags.ConfigureDirectories = false;
                    on_submit_callback(data);
                }
            }

            // Popup Modal window for invalid directory configuration.
            {
                // Always center this window when appearing
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                ImGui::SetNextWindowSize(toImVec2(ERROR_POPUP_DIALOG_WINDOW_SIZE));
                if (ImGui::BeginPopupModal(INVALID_DIRECTORY_CONFIGURATION_POPUP, nullptr, ImGuiWindowFlags_NoResize)) {
                    ImGui::TextWrapped("%s", message->c_str());
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing());
                    if (ImGui::Button("OK", {ImGui::GetContentRegionAvail().x, 0.0f})) { ImGui::CloseCurrentPopup(); }
                    ImGui::SetItemDefaultFocus();
                    ImGui::EndPopup();
                }
            }
        }
        ImGui::End();
    }

    void filesListView(
        const std::vector<std::string>& filepaths,
        MediaType media_type,
        std::function<void(const std::vector<std::string>&, MediaType, int)> on_file_selected_callback
    ) {
        namespace fs = std::filesystem;
        for (int i = 0; i < static_cast<int>(filepaths.size()); i++) {

            // Build label.
            std::string file_icon;
            if (media_type == MediaType::Image) { file_icon = ICON_FA_FILE_IMAGE; }
            else if (media_type == MediaType::Audio) { file_icon = ICON_FA_FILE_AUDIO; }
            else { file_icon = ICON_FA_FILE; }
            std::string file_entry = file_icon + " " + fs::path(filepaths.at(i)).filename().string();
            
            if (ImGui::Selectable(file_entry.c_str())) {
                on_file_selected_callback(filepaths, media_type, i);
            }
        }
    }

    void clearCurrentPreviewAndFilepath() {

        // If directories have not been configured, then closing
        // preview should have no effect.
        if (! m_directory_configuration) {
            return;
        }

        if (m_directory_configuration->mediaType == MediaType::Image) {
            m_current_media_image_preview = std::nullopt;
        } else if (m_directory_configuration->mediaType == MediaType::Audio) {
            // clear audio specific previews in the future.
        }

        m_current_media_filepath = std::nullopt;
    }

    void loadCurrentPreviewAndFilepath(const std::string& filepath) {
        
        // If directories have not been configured, then closing
        // preview should have no effect.
        if (! m_directory_configuration) {
            return;
        }

        if (m_directory_configuration->mediaType == MediaType::Image) {
            m_current_media_image_preview = Image::loadFromFile(filepath);
        } else if (m_directory_configuration->mediaType == MediaType::Audio) {
            // load audio specific previews in the future.
        }

        m_current_media_filepath = filepath;
    }

    void SelectableText(const std::string& text, bool fit_width = true) {
        if (fit_width) {
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        }
        
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
        ImGui::InputText("##display-selectable-text", const_cast<char*>(text.c_str()), text.size() + 1, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
    }

    std::pair<bool, std::optional<std::string>> validateDirectoryConfiguration(const DirectoryConfiguration& dir_config) {
        namespace fs = std::filesystem;

        const char* home_dir = std::getenv("HOME");
        if (home_dir == nullptr) {
            return {false, "Unable to determine the user's home directory."};
        }
        fs::path home_path(home_dir);

        auto check_directory = [&](const fs::path& dir) -> std::optional<std::string> {
            if (dir.empty()) {
                return "directory is empty.";
            }
            if (dir.string().find(home_path.string()) != 0) {
                return "directory " + dir.string() + " is not under the user's home directory.";
            }
            if (!fs::exists(dir)) {
                return "directory " + dir.string() + " does not exist.";
            }
            if (!fs::is_directory(dir)) {
                return "path " + dir.string() + " is not a directory.";
            }
            auto perms = fs::status(dir).permissions();
            if ((perms & fs::perms::owner_write) == fs::perms::none ||
                (perms & fs::perms::owner_read) == fs::perms::none ||
                (perms & fs::perms::owner_exec) == fs::perms::none) {
                return "directory " + dir.string() + " does not have sufficient permissions.";
            }
            return std::nullopt;
        };

        std::optional<std::string> message;

        message = check_directory(dir_config.sourceDirectory);
        if (message) return {false, "Source " + message.value()};

        message = check_directory(dir_config.classADirectory);
        if (message) return {false, "Class A " + message.value()};

        message = check_directory(dir_config.classBDirectory);
        if (message) return {false, "Class B " + message.value()};

        // Check if directories are distinct.
        if (fs::equivalent(dir_config.sourceDirectory, dir_config.classADirectory) ||
            fs::equivalent(dir_config.sourceDirectory, dir_config.classBDirectory) ||
            fs::equivalent(dir_config.classADirectory, dir_config.classBDirectory)) {
            return {false, "Source, Class A, and Class B directories must be distinct."};
        }

        fs::path csv_dir = fs::path(dir_config.outputFilePath).parent_path();
        message = check_directory(csv_dir);
        if (message) return {false, "Output file " + message.value()};

        return {true, std::nullopt};
    }

    void updateOutputFile() {
        using namespace csv2;
        namespace fs = std::filesystem;

        const std::vector<std::string> header = {"file", "bias"};
        const std::string filename = fs::path(m_current_media_filepath.value()).filename().string();

        // Load the content of the csv file to memory.
        // Remove record(s) of the current file from the loaded contents.
        // Add the record of the current file to the loaded contents.
        // Write the loaded contents back to file.

        std::vector<std::vector<std::string>> rows;

        // If the file exists, assume it's valid.
        if (fs::exists(m_directory_configuration->outputFilePath)) {
            Reader<
                delimiter<','>,
                quote_character<'"'>,
                first_row_is_header<true>,
                trim_policy::trim_whitespace
            > reader;
        
            // Load the content of the csv file to memory.
            if (reader.mmap(m_directory_configuration->outputFilePath)) {
                for (auto row : reader) {
                    std::vector<std::string> row_vector;
                    for (auto cell : row) {
                        row_vector.emplace_back(cell.read_view());
                    }
                    
                    // Keep the row only if it's not a record of the current file and
                    // number of fields matches that of the header titles.
                    if (row_vector.size() == header.size() && row_vector.front() != filename) {
                        rows.emplace_back(row_vector);
                    }
                }
            } else {
                throw std::runtime_error("couldn't read CSV file " + m_directory_configuration->outputFilePath);
            }
        }

        // Add the record of the current file to the loaded contents.
        rows.push_back({filename, toString(m_bias_value)});

        // Write the loaded contents back to file.
        std::ofstream outfile(m_directory_configuration->outputFilePath);
        if (! outfile.is_open()) {
            throw std::runtime_error("couldn't open file " + m_directory_configuration->outputFilePath + " for writing.");
        }

        // Write header.
        for (size_t i = 0; i < header.size(); i++) {
            if (i) {
                outfile << ',';
            }
            outfile << header.at(i);
        }
        outfile << '\n';

        // Write update rows.
        Writer<delimiter<','>> writer(outfile);
        writer.write_rows(rows);
        outfile.close();
    }

    void labelButtonClickHandler() {
        moveFile(
            m_current_media_filepath.value(),
            (m_bias_value > 0.5f ? m_directory_configuration->classADirectory : m_directory_configuration->classBDirectory),
            [&](const std::string& error_message) {
                m_error_moving_file_pop_up_message = error_message;
                ImGui::OpenPopup(ERROR_MOVING_FILE_POPUP);
            }
        );

        // Upldate the output file.
        updateOutputFile();

        // Load the next media for preview (if any).
        {
            std::lock_guard<std::mutex> lock(mutex_media_sources);
            auto it = std::find(m_media_sources->begin(), m_media_sources->end(), m_current_media_filepath.value());
            if (it != m_media_sources->end()) {
                m_media_sources->erase(it);
            }

            // Load the next media for preview if available.
            // Otherwise, clear the media preview.
            if (!m_media_sources->empty()) {
                loadCurrentPreviewAndFilepath(m_media_sources->front());
            } else {
                clearCurrentPreviewAndFilepath();
            }
        }
    }

    void handleKeyPress(SDL_Keycode key) {
        switch (key)
        {
        case SDLK_a:
            m_bias_value -= m_bias_sensitivity;
            if (m_bias_value < 0.0f) {
                m_bias_value = 0.0f;
            }
            break;
        
        case SDLK_d:
            m_bias_value += m_bias_sensitivity;
            if (m_bias_value > 1.0f) {
                m_bias_value = 1.0f;
            }
            break;

        case SDLK_l:
            m_keyboard_label_button_pressed = true;
            break;
        }
    }
};


int main() {
    const char*   APP_TITLE     = "MLBC";
    const int32_t APP_WIDTH     = 1280;
    const int32_t APP_HEIGHT    = 720;

    MLBC mlbc(APP_TITLE, APP_WIDTH, APP_HEIGHT);
    mlbc.run();
}
