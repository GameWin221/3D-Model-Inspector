#pragma once

#include <glm.hpp>

#include <string>

#include "Shader.h"

class DirLight
{
public:
	glm::vec3 direction;
	glm::vec3 color = glm::vec3(1, 1, 1);
	float intensity = 1;

	bool active = true;

	DirLight(glm::vec3 spawnDirection = glm::vec3(0, 0, 1));
	void UseOn(Shader& shader, int lightIndex);
};