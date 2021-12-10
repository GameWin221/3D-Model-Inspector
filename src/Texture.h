#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <iostream>

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>

#include <ColorfulLogging.h>

#include <chrono>

#include <stb_image.h>

class Texture
{
public:
	int channels;
	unsigned int ID;

	void Use();

	GLuint colorSpace, colorFormat;

	glm::ivec2 size;
	std::string originPath;

	Texture(std::string path);
	~Texture();
};
#endif