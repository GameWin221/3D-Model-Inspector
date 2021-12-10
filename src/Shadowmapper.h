#pragma once

#ifndef SHADOWMAPPER_H
#define SHADOWMAPPER_H

#define MAXLIGHTS 3

#include "Light.h"
#include "Model.h"
#include "Shader.h"

#include <glm.hpp>

#include <glew.h>
#include <glfw3.h>

#include <vector>

namespace Shadowmapper
{
	void Init(unsigned int resolution);

	void Use();
	//void CalculateShadows(DirLight* light, Shader* depth_shader);
};
#endif