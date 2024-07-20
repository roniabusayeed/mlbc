#include "image.h"

Image::Image(ImTextureID gpu_texture, int32_t width, int32_t height)
    : m_gpu_texture(gpu_texture), m_width(width), m_height(height) {}

uint32_t Image::createTexture(uint8_t* data, int32_t width, int32_t height) {
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

const ImTextureID& Image::getTexture() const {
    return m_gpu_texture;
}

int32_t Image::getWidth() const {
    return m_width;
}

int32_t Image::getHeight() const {
    return m_height;
}

Image Image::loadFromFile(const std::string& filepath) {
    int32_t width;
    int32_t height;
    int32_t channels;

    uint8_t* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
    if (data) {
        uint32_t texture = createTexture(data, width, height);
        stbi_image_free(data);
        return Image(
            reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture)),
            width,
            height
        );
    }

    throw std::runtime_error("error: couldn't load image: " + filepath);
}

Image::~Image() {

    // Clean up OpenGL texture.
    if (m_gpu_texture) {
        uint32_t texture = static_cast<uint32_t>(reinterpret_cast<intptr_t>(m_gpu_texture));
        glDeleteTextures(1, &texture);
        m_gpu_texture = 0;
    }
}

Image::Image(Image&& other) noexcept
    : m_gpu_texture(other.m_gpu_texture), m_width(other.m_width), m_height(other.m_height) {
    other.m_gpu_texture = 0;
    other.m_width = 0;
    other.m_height = 0;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        if (m_gpu_texture) {
            uint32_t texture = static_cast<uint32_t>(reinterpret_cast<intptr_t>(m_gpu_texture));
            glDeleteTextures(1, &texture);
        }

        m_gpu_texture = other.m_gpu_texture;
        m_width = other.m_width;
        m_height = other.m_height;

        other.m_gpu_texture = 0;
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}
