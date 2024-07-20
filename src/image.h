#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <imgui.h>
#include <string>
#include <stdexcept>

class Image {
private:
    ImTextureID     m_gpu_texture;
    int32_t         m_width;
    int32_t         m_height;

    // Private constructor to ensure objects are created through loadFromFile().
    Image(ImTextureID gpu_texture, int32_t width, int32_t height);

    // Function to create OpenGL texture from image data.
    static uint32_t createTexture(uint8_t* data, int32_t width, int32_t height);

public:
    const ImTextureID& getTexture() const;
    int32_t getWidth() const;
    int32_t getHeight() const;

    // Static method to load image from file.
    static Image loadFromFile(const std::string& filepath);

    ~Image();

    // Disable copying Image objects.
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Enable move semantics.
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;
};
