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
#include <vector>
#include <string>

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

	extern void processInput(GLFWwindow* window, Camera& cam, double& delta_time);
	extern void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	extern void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

	extern void DrawImGui();
	extern void InitImGui(GLFWwindow* target_window);

	extern GLFWwindow* InitGL();
	
	extern void Init();
	extern void Update();
	extern void Render();
	extern void Exit();
};
#endif