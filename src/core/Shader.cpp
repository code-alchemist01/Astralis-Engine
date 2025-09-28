#include "Shader.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <iostream>

// Use GLFW's OpenGL loader
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// OpenGL types
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef float GLfloat;
typedef char GLchar;

// OpenGL constants
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_FALSE 0

static GLuint(*glCreateShader)(GLenum) = nullptr;
static void(*glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = nullptr;
static void(*glCompileShader)(GLuint) = nullptr;
static void(*glGetShaderiv)(GLuint, GLenum, GLint*) = nullptr;
static void(*glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = nullptr;
static void(*glDeleteShader)(GLuint) = nullptr;
static GLuint(*glCreateProgram)() = nullptr;
static void(*glAttachShader)(GLuint, GLuint) = nullptr;
static void(*glLinkProgram)(GLuint) = nullptr;
static void(*glGetProgramiv)(GLuint, GLenum, GLint*) = nullptr;
static void(*glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = nullptr;
static void(*glDeleteProgram)(GLuint) = nullptr;
static void(*glUseProgram)(GLuint) = nullptr;
static GLint(*glGetUniformLocation)(GLuint, const GLchar*) = nullptr;
static void(*glUniform1i)(GLint, GLint) = nullptr;
static void(*glUniform1f)(GLint, GLfloat) = nullptr;
static void(*glUniform3fv)(GLint, GLsizei, const GLfloat*) = nullptr;
static void(*glUniform4fv)(GLint, GLsizei, const GLfloat*) = nullptr;
static void(*glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = nullptr;

static bool loadOpenGLFunctions() {
    static bool loaded = false;
    if (loaded) return true;
    
    glCreateShader = (GLuint(*)(GLenum))glfwGetProcAddress("glCreateShader");
    glShaderSource = (void(*)(GLuint, GLsizei, const GLchar* const*, const GLint*))glfwGetProcAddress("glShaderSource");
    glCompileShader = (void(*)(GLuint))glfwGetProcAddress("glCompileShader");
    glGetShaderiv = (void(*)(GLuint, GLenum, GLint*))glfwGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (void(*)(GLuint, GLsizei, GLsizei*, GLchar*))glfwGetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (void(*)(GLuint))glfwGetProcAddress("glDeleteShader");
    glCreateProgram = (GLuint(*)())glfwGetProcAddress("glCreateProgram");
    glAttachShader = (void(*)(GLuint, GLuint))glfwGetProcAddress("glAttachShader");
    glLinkProgram = (void(*)(GLuint))glfwGetProcAddress("glLinkProgram");
    glGetProgramiv = (void(*)(GLuint, GLenum, GLint*))glfwGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (void(*)(GLuint, GLsizei, GLsizei*, GLchar*))glfwGetProcAddress("glGetProgramInfoLog");
    glDeleteProgram = (void(*)(GLuint))glfwGetProcAddress("glDeleteProgram");
    glUseProgram = (void(*)(GLuint))glfwGetProcAddress("glUseProgram");
    glGetUniformLocation = (GLint(*)(GLuint, const GLchar*))glfwGetProcAddress("glGetUniformLocation");
    glUniform1i = (void(*)(GLint, GLint))glfwGetProcAddress("glUniform1i");
    glUniform1f = (void(*)(GLint, GLfloat))glfwGetProcAddress("glUniform1f");
    glUniform3fv = (void(*)(GLint, GLsizei, const GLfloat*))glfwGetProcAddress("glUniform3fv");
    glUniform4fv = (void(*)(GLint, GLsizei, const GLfloat*))glfwGetProcAddress("glUniform4fv");
    glUniformMatrix4fv = (void(*)(GLint, GLsizei, GLboolean, const GLfloat*))glfwGetProcAddress("glUniformMatrix4fv");
    
    loaded = (glCreateShader && glShaderSource && glCompileShader && glGetShaderiv && 
              glGetShaderInfoLog && glDeleteShader && glCreateProgram && glAttachShader && 
              glLinkProgram && glGetProgramiv && glGetProgramInfoLog && glDeleteProgram && 
              glUseProgram && glGetUniformLocation && glUniform1i && glUniform1f && 
              glUniform3fv && glUniform4fv && glUniformMatrix4fv);
    
    return loaded;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    : programId_(0) {
    
    spdlog::info("Loading shader: {} + {}", vertexPath, fragmentPath);
    
    // Load OpenGL functions dynamically
    if (!loadOpenGLFunctions()) {
        spdlog::error("Failed to load OpenGL functions");
        return;
    }
    
    spdlog::info("OpenGL functions loaded successfully");
    
    try {
        // Load shader sources
        std::string vertexCode = loadShaderSource(vertexPath);
        std::string fragmentCode = loadShaderSource(fragmentPath);
        
        if (vertexCode.empty() || fragmentCode.empty()) {
            spdlog::error("Failed to load shader sources");
            return;
        }
        
        // Compile shaders
        GLuint vertexShader = compileShader(vertexCode, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentCode, GL_FRAGMENT_SHADER);
        
        if (vertexShader == 0 || fragmentShader == 0) {
            spdlog::error("Failed to compile shaders");
            if (vertexShader != 0) glDeleteShader(vertexShader);
            if (fragmentShader != 0) glDeleteShader(fragmentShader);
            return;
        }
        
        // Create shader program
        programId_ = createShaderProgram(vertexShader, fragmentShader);
        
        // Clean up individual shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        if (programId_ != 0) {
            spdlog::info("Shader program created successfully with ID: {}", programId_);
        } else {
            spdlog::error("Failed to create shader program");
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Exception during shader creation: {}", e.what());
        programId_ = 0;
    }
}

Shader::~Shader() {
    if (programId_ != 0) {
        glDeleteProgram(programId_);
        spdlog::debug("Shader program {} deleted", programId_);
    }
}

void Shader::use() const {
    if (programId_ != 0) {
        glUseProgram(programId_);
    } else {
        spdlog::warn("Attempting to use invalid shader program");
    }
}

void Shader::unuse() const {
    glUseProgram(0);
}

void Shader::setInt(const std::string& name, int value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void Shader::setUint(const std::string& name, unsigned int value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, static_cast<int>(value));
    }
}

void Shader::setBool(const std::string& name, bool value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value ? 1 : 0);
    }
}

void Shader::setFloat(const std::string& name, float value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, &value[0]);
    }
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform4fv(location, 1, &value[0]);
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }
}

std::string Shader::loadShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        spdlog::error("Failed to open shader file: {}", filePath);
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    std::string source = buffer.str();
    spdlog::debug("Loaded shader source from {}: {} characters", filePath, source.length());
    return source;
}

GLuint Shader::compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    checkCompileErrors(shader, shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint Shader::createShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    checkLinkErrors(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

GLint Shader::getUniformLocation(const std::string& name) const {
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) {
        return it->second;
    }
    
    GLint location = glGetUniformLocation(programId_, name.c_str());
    uniformCache_[name] = location;
    
    if (location == -1) {
        spdlog::warn("Uniform '{}' not found in shader program {}", name, programId_);
    }
    
    return location;
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        spdlog::error("Shader compilation error ({}): {}", type, infoLog);
    }
}

void Shader::checkLinkErrors(GLuint program) {
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        spdlog::error("Shader program linking error: {}", infoLog);
    }
}