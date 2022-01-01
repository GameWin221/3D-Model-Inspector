#pragma once

#ifndef APPLICATION
#define APPLICATION

// GLEW / GLFW
#include <glew.h>
#include <glfw3.h>

// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// CL
#include <ColorfulLogging.h>

// C++
#include <iostream>
#include <string>
#include <array>
#include <fstream>

// Header Files
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Vertex.h"
#include "OBJLoader.h"
#include "Model.h"
#include "Light.h"
#include "Framebuffer.h"
#include "Shadowmapper.h"

//#include "../settings.config"

namespace Application
{
	extern bool isRunning();

	extern void ProcessWindowInput();
	extern void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	extern void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

	extern void LoadLights(std::string saveName);
	extern void SaveLights(std::string saveName);

	extern void DrawImGui();
	extern void InitImGui();
	extern void InitGL();
	
	extern void Init();
	extern void Update();
	extern void Render();
	extern void Exit();
};
#endif