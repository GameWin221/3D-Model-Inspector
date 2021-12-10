#pragma once

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Shader.h"

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>

#include <iostream>

#include "../settings.config"

class Framebuffer
{
public:
	Framebuffer(glm::uvec2 size);
	~Framebuffer();

	void Use();
	void Clear(glm::vec4 clearColor = glm::vec4(0, 0, 0, 1));
	void Display(unsigned int textureID = NULL);

	void ReloadShader();
	void Resize(glm::uvec2 size);

	glm::uvec2 fbSize;

	Shader* shaderPtr = nullptr;

private:
	unsigned int quadVAO, quadVBO;
	
	// Multisampled Objects
	unsigned int framebuffer;
	unsigned int colorTextureMSAA;
	unsigned int rbo;

	// Singlesampled Objects
	unsigned int intermediateFBO;
	unsigned int screenTexture;
};

#endif