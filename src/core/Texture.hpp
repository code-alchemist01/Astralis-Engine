#pragma once

#include <string>
#include <vector>

// Forward declarations for OpenGL types
typedef unsigned int GLuint;
typedef unsigned int GLenum;

// OpenGL constants
#define GL_RGBA 0x1908

namespace Core {

class Texture {
public:
    Texture();
    explicit Texture(const std::string& filepath);
    ~Texture();

    // Delete copy constructor and assignment operator
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Move constructor and assignment operator
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Load texture from file
    bool loadFromFile(const std::string& filepath);

    // Load cubemap from 6 files
    bool loadCubemap(const std::vector<std::string>& faces);

    // Create empty texture with specified dimensions
    bool create(int width, int height, GLenum format = GL_RGBA);

    // Create a dummy 1x1 white texture
    bool createDummyTexture();

    // Bind texture to specified texture unit
    void bind(unsigned int unit = 0) const;

    // Unbind texture
    void unbind() const;

    // Getters
    unsigned int getId() const { return textureId_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getChannels() const { return channels_; }
    bool isValid() const { return textureId_ != 0; }

    // Set texture parameters
    void setWrapMode(GLenum wrapS, GLenum wrapT);
    void setFilterMode(GLenum minFilter, GLenum magFilter);

private:
    unsigned int textureId_;
    int width_;
    int height_;
    int channels_;
    bool isCubemap_;

    void cleanup();
};

} // namespace Core