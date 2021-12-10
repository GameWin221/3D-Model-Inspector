#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Texture.h"

class Shader
{
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void Use();

    void SetMatrix4(const std::string& name, glm::mat4 value) const;
    void SetBool   (const std::string& name, bool      value) const;
    void SetInt    (const std::string& name, int       value) const;
    void SetFloat  (const std::string& name, float     value) const;
    void SetDouble (const std::string& name, float     value) const;
    void SetVec4   (const std::string& name, glm::vec4 value) const;
    void SetVec3   (const std::string& name, glm::vec3 value) const;
    void SetVec2   (const std::string& name, glm::vec2 value) const;

    int GetUniformPos(const std::string& name);
};
#endif