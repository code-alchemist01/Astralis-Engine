#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    // Non-copyable, non-movable
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    void use() const;
    void unuse() const;
    
    bool isValid() const { return programId_ != 0; }
    GLuint getProgramId() const { return programId_; }

    // Uniform setters
    void setInt(const std::string& name, int value);
    void setUint(const std::string& name, unsigned int value);
    void setBool(const std::string& name, bool value);
    void setFloat(const std::string& name, float value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat4(const std::string& name, const glm::mat4& value);

private:
    GLuint programId_;
    mutable std::unordered_map<std::string, GLint> uniformCache_;

    std::string loadShaderSource(const std::string& filePath);
    GLuint compileShader(const std::string& source, GLenum shaderType);
    GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader);
    GLint getUniformLocation(const std::string& name) const;
    void checkCompileErrors(GLuint shader, const std::string& type);
    void checkLinkErrors(GLuint program);
};