#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include <glew.h>
#include <glfw3.h>

#include "Texture.h"
#include "Shader.h"

#include <glm.hpp>

struct Material
{
    Material(Shader* _shader, Texture* albedo = nullptr, Texture* specular = nullptr)
    {
        this->shader = _shader;
        this->albedoTex = albedo;
        this->specularTex = specular;
    }

    Texture* albedoTex;
    Texture* specularTex;

    Shader* shader;

    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

    bool wireframe = false;

    float shininess = 20;
    float specular = 1;
};
#endif