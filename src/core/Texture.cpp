#include "Texture.hpp"
#include <spdlog/spdlog.h>

// Use GLFW's OpenGL loader
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// OpenGL types
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef float GLfloat;
typedef char GLchar;

// OpenGL constants
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_RED 0x1903
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_LINEAR 0x2601
#define GL_LINEAR_MIPMAP_LINEAR 0x2703

// OpenGL function pointers
static void(*glGenTextures)(GLsizei, GLuint*) = nullptr;
static void(*glBindTexture)(GLenum, GLuint) = nullptr;
static void(*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) = nullptr;
static void(*glGenerateMipmap)(GLenum) = nullptr;
static void(*glActiveTexture)(GLenum) = nullptr;
static void(*glTexParameteri)(GLenum, GLenum, GLint) = nullptr;
static void(*glDeleteTextures)(GLsizei, const GLuint*) = nullptr;

// Function to load OpenGL functions dynamically
static bool loadOpenGLFunctions() {
    static bool loaded = false;
    if (loaded) return true;

    glGenTextures = (void(*)(GLsizei, GLuint*))glfwGetProcAddress("glGenTextures");
    glBindTexture = (void(*)(GLenum, GLuint))glfwGetProcAddress("glBindTexture");
    glTexImage2D = (void(*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*))glfwGetProcAddress("glTexImage2D");
    glGenerateMipmap = (void(*)(GLenum))glfwGetProcAddress("glGenerateMipmap");
    glActiveTexture = (void(*)(GLenum))glfwGetProcAddress("glActiveTexture");
    glTexParameteri = (void(*)(GLenum, GLenum, GLint))glfwGetProcAddress("glTexParameteri");
    glDeleteTextures = (void(*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteTextures");

    loaded = (glGenTextures && glBindTexture && glTexImage2D && glGenerateMipmap && 
              glActiveTexture && glTexParameteri && glDeleteTextures);
    
    if (loaded) {
        spdlog::info("Texture OpenGL functions loaded successfully");
    } else {
        spdlog::error("Failed to load Texture OpenGL functions");
    }
    
    return loaded;
}

namespace Core {

Texture::Texture() 
    : textureId_(0), width_(0), height_(0), channels_(0), isCubemap_(false) {
}

Texture::Texture(const std::string& filepath) 
    : textureId_(0), width_(0), height_(0), channels_(0), isCubemap_(false) {
    loadFromFile(filepath);
}

Texture::~Texture() {
    cleanup();
}

Texture::Texture(Texture&& other) noexcept 
    : textureId_(other.textureId_), width_(other.width_), 
      height_(other.height_), channels_(other.channels_), isCubemap_(other.isCubemap_) {
    other.textureId_ = 0;
    other.width_ = 0;
    other.height_ = 0;
    other.channels_ = 0;
    other.isCubemap_ = false;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        cleanup();
        
        textureId_ = other.textureId_;
        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;
        isCubemap_ = other.isCubemap_;
        
        other.textureId_ = 0;
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
        other.isCubemap_ = false;
    }
    return *this;
}

bool Texture::loadFromFile(const std::string& filepath) {
    if (!loadOpenGLFunctions()) {
        spdlog::error("Failed to load OpenGL functions for texture loading");
        return false;
    }
    
    cleanup();

    // Set stb_image to flip loaded texture's on the y-axis
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filepath.c_str(), &width_, &height_, &channels_, 0);
    if (!data) {
        spdlog::error("Failed to load texture: {}", filepath);
        return false;
    }

    // Generate texture
    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);

    // Determine format based on channels
    GLenum format;
    switch (channels_) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default:
            spdlog::error("Unsupported number of channels: {}", channels_);
            stbi_image_free(data);
            cleanup();
            return false;
    }

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set default texture parameters
    setWrapMode(GL_REPEAT, GL_REPEAT);
    setFilterMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Free image data
    stbi_image_free(data);

    spdlog::info("Loaded texture: {} ({}x{}, {} channels)", filepath, width_, height_, channels_);
    return true;
}

bool Texture::loadCubemap(const std::vector<std::string>& faces) {
    if (!loadOpenGLFunctions()) {
        spdlog::error("Failed to load OpenGL functions for cubemap loading");
        return false;
    }

    if (faces.size() != 6) {
        spdlog::error("Cubemap requires exactly 6 faces, got {}", faces.size());
        return false;
    }

    cleanup();

    isCubemap_ = true;
    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId_);

    // Cubemap face targets in order: +X, -X, +Y, -Y, +Z, -Z
    GLenum targets[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (int i = 0; i < 6; i++) {
        spdlog::debug("Attempting to load cubemap face {}: {}", i, faces[i]);
        int width, height, channels;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
        
        if (!data) {
            spdlog::error("Failed to load cubemap face: {}", faces[i]);
            spdlog::error("STB error: {}", stbi_failure_reason());
            cleanup();
            return false;
        }

        // Store dimensions from first face
        if (i == 0) {
            width_ = width;
            height_ = height;
            channels_ = channels;
        }

        // Determine format
        GLenum format = GL_RGB;
        if (channels == 4) format = GL_RGBA;
        else if (channels == 1) format = GL_RED;

        glTexImage2D(targets[i], 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    // Set cubemap parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    spdlog::info("Loaded cubemap with {} faces ({}x{}, {} channels)", faces.size(), width_, height_, channels_);
    return true;
}

bool Texture::create(int width, int height, GLenum format) {
    if (!loadOpenGLFunctions()) {
        spdlog::error("Failed to load OpenGL functions for texture creation");
        return false;
    }
    
    cleanup();

    width_ = width;
    height_ = height;
    channels_ = (format == GL_RGBA) ? 4 : (format == GL_RGB) ? 3 : 1;
    isCubemap_ = false;

    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, nullptr);

    // Set default texture parameters
    setWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    setFilterMode(GL_LINEAR, GL_LINEAR);

    spdlog::info("Created empty texture ({}x{}, format: {})", width_, height_, format);
    return true;
}

bool Texture::createDummyTexture() {
    if (!loadOpenGLFunctions()) {
        spdlog::error("Failed to load OpenGL functions for dummy texture creation");
        return false;
    }
    
    cleanup();

    width_ = 1;
    height_ = 1;
    channels_ = 4;
    isCubemap_ = false;

    // Create a 1x1 white pixel
    unsigned char whitePixel[4] = {255, 255, 255, 255}; // RGBA

    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    // Set default texture parameters
    setWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    setFilterMode(GL_LINEAR, GL_LINEAR);

    spdlog::info("Created dummy 1x1 white texture");
    return true;
}

void Texture::bind(unsigned int unit) const {
    if (textureId_ != 0 && loadOpenGLFunctions()) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(isCubemap_ ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureId_);
    }
}

void Texture::unbind() const {
    if (loadOpenGLFunctions()) {
        glBindTexture(isCubemap_ ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, 0);
    }
}

void Texture::setWrapMode(GLenum wrapS, GLenum wrapT) {
    if (textureId_ != 0 && loadOpenGLFunctions()) {
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    }
}

void Texture::setFilterMode(GLenum minFilter, GLenum magFilter) {
    if (textureId_ != 0 && loadOpenGLFunctions()) {
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    }
}

void Texture::cleanup() {
    if (textureId_ != 0 && loadOpenGLFunctions()) {
        glDeleteTextures(1, &textureId_);
        textureId_ = 0;
        width_ = 0;
        height_ = 0;
        channels_ = 0;
    }
}

} // namespace Core