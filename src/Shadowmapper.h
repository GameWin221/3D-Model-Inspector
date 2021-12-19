#pragma once

#ifndef SHADOWMAPPER_H
#define SHADOWMAPPER_H

#define MAXLIGHTS 3

#include "Light.h"
#include "Model.h"
#include "Light.h"
#include "Shader.h"

#include <glm.hpp>

#include <glew.h>
#include <glfw3.h>

#include <array>

namespace Shadowmapper
{
	void EnableShadows();
	void DisableShadows();
	void GenShadowmaps(unsigned int resolution);
	void RenderShadowmaps(std::array<DirLight*, MAXLIGHTS> lightsArr, Shader* mainShader, Model* targetModel);
};
#endif