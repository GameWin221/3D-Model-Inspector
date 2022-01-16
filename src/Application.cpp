#include "Application.h"

namespace Application
{
	glm::ivec2 windowSize(1920, 1080);

	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;
	Framebuffer* fb = nullptr;

	// Set MAXLIGHTS in the "fragment_core_shaded.glsl", "vertex_core_shaded.glsl" and "Shadowmapper.h" to the same value as here
	#define MAXLIGHTS 3
	std::array<DirLight*, MAXLIGHTS> lights;

	int activatedLights = 1;

	Texture* albedoTexture = nullptr;
	Texture* specularTexture = nullptr;

	Shader* shader = nullptr;
	Material* material = nullptr;
	Model* model = nullptr;

	double mouse_x, mouse_y;
	double deltaTime;

	float modelRotation[3];
	float modelPosition[3];
	float modelScale[3] = { 1, 1, 1 };
	float modelColor[3] = { 1, 1, 1 };

	float aLightColor[3] = { 0.1f, 0.1f, 0.1f };

	void Init()
	{
		InitGL();
		InitImGui();

		camera = new Camera(glm::vec3(0.0f, 0.0f, 2.0f));
		shader = new Shader("Shaders/vertex_core_shaded.glsl", "Shaders/fragment_core_shaded.glsl");
		material = new Material(shader, nullptr, nullptr);
		fb = new Framebuffer(windowSize);

		for (int i = 0; i < MAXLIGHTS; i++)
		{
			lights[i] = new DirLight();
			lights[i]->active = false;
		}
		lights[0]->active = true;

		Shadowmapper::GenShadowmaps(2048);
	}

	void Update()
	{
		deltaTime = 1.0 / ImGui::GetIO().Framerate;

		// Input
		ProcessWindowInput();
		glfwPollEvents();

		if (model != nullptr)
		{
			model->position = glm::vec3(modelPosition[0], modelPosition[1], modelPosition[2]);
			model->rotation = glm::vec3(modelRotation[0], modelRotation[1], modelRotation[2]);
			model->scale = glm::vec3(modelScale[0], modelScale[1], modelScale[2]);
			model->material->color = glm::vec3(modelColor[0], modelColor[1], modelColor[2]);
		}
	}

	void Render()
	{
		// Cast shadows (render shadowmaps)
		Shadowmapper::RenderShadowmaps(lights, shader, model);

		// Set the 'ambientColor' vec3 uniform
		shader->Use();
		shader->SetVec3("ambientColor", glm::vec3(aLightColor[0], aLightColor[1], aLightColor[2]));
		
		// Bind the framebuffer
		fb->Use();
		fb->Clear(glm::vec3(aLightColor[0] / 2.0f, aLightColor[1] / 2.0f, aLightColor[2] / 2.0f));

		// Rendering
		for (int i=0; i<MAXLIGHTS; i++)
			lights[i]->UseOn(shader, i);

		if (model != nullptr)
		{
			model->Update();
			model->Render(camera);
		}

		// Showing the framebuffer
		fb->Display();

		// Rendering UI
		DrawImGui();

		// Polling events and swapping the buffers
		glfwSwapBuffers(window);
	}

	void Exit()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		delete window;
		delete camera;
		delete albedoTexture;
		delete specularTexture;
		delete shader;
		delete material;
		delete model;

		for (auto& lgt : lights)
			delete lgt;

		glfwTerminate();
	}

	void ProcessWindowInput()
	{
		// Mouse position one frame before
		double temp_mouse_x = mouse_x;
		double temp_mouse_y = mouse_y;

		// Updating the mouse position
		glfwGetCursorPos(window, &mouse_x, &mouse_y);

		// Getting the mouse's velocity
		double delta_x = temp_mouse_x - mouse_x;
		double delta_y = temp_mouse_y - mouse_y;
		float delta_magnitude = sqrt(pow(delta_x, 2) + pow(delta_y, 2));

		// If the user is holding the Right Mouse Button
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			// Forward / Backward
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				camera->Move(glm::vec3(0.0f, 0.0f, 1.0f * deltaTime));
			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				camera->Move(glm::vec3(0.0f, 0.0f, -1.0f * deltaTime));

			// Left / Right
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				camera->Move(glm::vec3(-1.0f * deltaTime, 0.0f, 0.0f));
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				camera->Move(glm::vec3(1.0f * deltaTime, 0.0f, 0.0f));

			// Up / Down
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				camera->Move(glm::vec3(0.0f, 1.0f * deltaTime, 0.0f));
			else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				camera->Move(glm::vec3(0.0f, -1.0f * deltaTime, 0.0f));

			// Camera Rotation
			if (delta_magnitude > 0)
				camera->Rotate(-delta_x / 10, delta_y / 10);
		}
	}

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		yoffset /= -8.0f;
		if (camera->orbital &&
			camera->orbitalDistance + yoffset < camera->maxOrbitalDistance &&
			camera->orbitalDistance + yoffset > 0)
		{
			camera->orbitalDistance += yoffset;
			camera->UpdateVectors();
		}

	}

	void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
		windowSize = glm::uvec2(width, height);

		fb->Resize(glm::uvec2(width, height));
	}

	void SaveLights(std::string saveName, std::array<float, MAXLIGHTS>& hDirArray, std::array<float, MAXLIGHTS>& vDirArray)
	{
		std::ofstream file(saveName + ".lights");

		file << MAXLIGHTS << '\n';
		file << activatedLights << '\n';

		int i = 0;
		for (const auto& l : lights)
		{
			file << "Light\n";
			file << l->active << '\n';
			file << l->color.r << " " << l->color.g << " " << l->color.b << '\n';
			file << hDirArray[i] << " " << vDirArray[i] << '\n';
			file << l->intensity << '\n';
			i++;
		}

		file.close();
	}

	void LoadLights(std::string saveName, std::array<float, MAXLIGHTS>& hDirArray, std::array<float, MAXLIGHTS>& vDirArray)
	{
		std::ifstream file(saveName + ".lights");

		if (file.is_open())
		{
			std::string line = "";

			// Check the MAXLIGHTS variable
			std::getline(file, line);
			if (std::stoi(line) == MAXLIGHTS)
			{
				// 'activatedLights' variable
				std::getline(file, line);
				activatedLights = std::stoi(line);

				// Iterating through all of the saved lights
				int lightIndex = 0;
				while (std::getline(file, line))
				{
					if (line == "Light")
					{
						// DirLight.active
						std::getline(file, line);
						lights[lightIndex]->active = (bool)std::stoi(line);

						// DirLight.color
						std::getline(file, line);
						std::istringstream ssColor(line);

						std::string color;
						// Red
						ssColor >> color;
						lights[lightIndex]->color.r = std::stof(color);

						// Green
						ssColor >> color;
						lights[lightIndex]->color.g = std::stof(color);

						// Blue
						ssColor >> color;
						lights[lightIndex]->color.b = std::stof(color);

						// DirLight.direction
						std::getline(file, line);
						std::istringstream ssDirection(line);

						std::string direction = "";
						// hDir
						ssDirection >> direction;
						hDirArray[lightIndex] = std::stof(direction);

						// vDir
						ssDirection >> direction;
						vDirArray[lightIndex] = std::stof(direction);

						float height = 1.0f - abs(sin(vDirArray[lightIndex]));
						lights[lightIndex]->direction = glm::vec3(cos(hDirArray[lightIndex]) * height, sin(vDirArray[lightIndex]), sin(hDirArray[lightIndex]) * height);

						// DirLight.intensity
						std::getline(file, line);
						lights[lightIndex]->intensity = std::stof(line);

						lightIndex++;
					}
				}
				cl::Log("Successfully loaded \"" + saveName + ".lights\"!\n", cl::Success);
			}
			else
				cl::Log("Failed to load \"" + saveName + ".lights\"! 'MAXLIGHTS' in the save file differs from the 'MAXLIGHTS' in this instance!\n", cl::Error);
		}
		else
			cl::Log("Failed to load \"" + saveName + ".lights\"! The file doesn't exist!\n", cl::Error);

		file.close();
	}

	void DrawImGui()
	{
		// ImGui creating new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui window
		static char modelpath[80] = "";
		static char albedopath[80] = "";
		static char specularpath[80] = "";
		//static char normalpath[80] = "";

		static std::array<float, MAXLIGHTS> hdir;
		static std::array<float, MAXLIGHTS> vdir;
		static float col[MAXLIGHTS][3];
		static char saveFile[40] = "lightpreset";

		static bool hinitialized = false;
		if (!hinitialized)
			for (auto& _hdir : hdir)
			{
				_hdir = 1.57079f;
				hinitialized = true;
			}

		ImGui::Begin("Inspector");

		if (ImGui::TreeNode("Model Loader"))
		{
			ImGui::Text("Default Path: \"OBJs/[further_path]\"");
			ImGui::InputText(".OBJ Path", modelpath, IM_ARRAYSIZE(modelpath));
			ImGui::Spacing();

			static bool albedoTex;
			ImGui::Checkbox("Albedo Texture", &albedoTex);
			if (albedoTex)
				ImGui::InputText("Albedo Texture Path", albedopath, IM_ARRAYSIZE(albedopath));

			ImGui::Spacing();

			static bool specularTex;
			ImGui::Checkbox("Specular Texture", &specularTex);
			if (specularTex)
				ImGui::InputText("Specular Texture Path", specularpath, IM_ARRAYSIZE(specularpath));

			ImGui::Spacing();

			if (ImGui::Button("(Re)Load the Model and Textures"))
			{
				std::cout << '\n';
				cl::Log("> Loading Begins\n", cl::Level::Info);
				cl::BenchmarkBegin("Main Loading");

				delete albedoTexture;
				delete specularTexture;
				delete model;

				if (albedoTex)
					albedoTexture = new Texture("OBJs/" + std::string(albedopath));
				else
					albedoTexture = new Texture("OBJs/NoTextureWhite.jpg");

				if (specularTex)
					specularTexture = new Texture("OBJs/" + std::string(specularpath));
				else
					specularTexture = new Texture("OBJs/NoTextureWhite.jpg");


				material = new Material(shader, albedoTexture, specularTexture);

				if (modelpath != "")
					model = new Model("OBJs/" + std::string(modelpath), material);
				else
					model = nullptr;

				double loadTime = cl::BenchmarkStop("Main Loading");
				cl::BenchmarkStopAll();

				cl::Log("> Loading Finished - It took " + std::to_string(loadTime * 1000) + " milliseconds\n", cl::Level::Success);
			}

			ImGui::TreePop();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::TreeNode("Model Settings"))
		{
			if (model != nullptr)
			{
				if (model->vertCount > 0)
				{
					std::stringstream a;
					a << "Vertices: " << model->vertCount;
					ImGui::Text(a.str().c_str());

					std::stringstream b;
					b << "Triangles: " << model->triCount;
					ImGui::Text(b.str().c_str());
					ImGui::TreePop();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::InputFloat3("Model Rotation", modelRotation);
					ImGui::InputFloat3("Model Scale", modelScale);
					ImGui::ColorEdit3("Model Color", modelColor);

					ImGui::Spacing();

					ImGui::SliderFloat("Shininess", &material->shininess, 1, 256);
					ImGui::SliderFloat("Specular", &material->specular, 0, 3);
				}
				else
					ImGui::Text("No model loaded!");
			}
			else
				ImGui::Text("No model loaded!");
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::TreeNode("Rendering Debug Settings"))
		{
			ImGui::Checkbox("Wireframe Mode", &material->wireframe);
		
			ImGui::Spacing();

			static bool shadows = true;
			if (ImGui::Checkbox("Shadows", &shadows))
			{
				if (shadows)
					Shadowmapper::EnableShadows();
				else
					Shadowmapper::DisableShadows();
			}

			ImGui::TreePop();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::TreeNode("Camera Settings"))
		{
			ImGui::Checkbox("Orbital Cam", &camera->orbital);
			if (camera->orbital)
			{
				if (ImGui::SliderFloat("Distance", &camera->orbitalDistance, 0, 10.0f))
					camera->UpdateVectors();
			}
			else
			{
				ImGui::SliderFloat("Speed", &camera->speed, 1, 300);

				if (ImGui::Button("Go to (0, 0 ,0)"))
					camera->position = glm::vec3(0, 0, 0);
				
			}

			ImGui::SliderFloat("FOV", &camera->fov, camera->minFov, camera->maxFov);

			ImGui::SliderFloat("Gamma", &fb->gamma, 0, 4);

			ImGui::SliderFloat("Exposure", &fb->exposure, 0, 5);

			

			ImGui::TreePop();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::TreeNode("Lights"))
		{
			ImGui::ColorEdit3("Ambient Color", aLightColor);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::InputText("Save file name", saveFile, 40);
			if (ImGui::Button("Save current lights"))
				SaveLights(saveFile, hdir, vdir);

			if (ImGui::Button("Load lights"))
				LoadLights(saveFile, hdir, vdir);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::Button("Add Light"))
			{
				if (activatedLights < MAXLIGHTS)
				{
					activatedLights++;
					for (int i = 0; i < activatedLights; i++)
						lights[i]->active = true;
				}
			}
			if (ImGui::Button("Remove Light"))
			{
				if (activatedLights > 0)
				{
					activatedLights--;
					for (int i = 0; i < MAXLIGHTS; i++)
						lights[i]->active = false;

					for (int i = 0; i < activatedLights; i++)
						lights[i]->active = true;
				}
			}

			for (int i = 0; i < MAXLIGHTS; i++)
				{
					if (lights[i]->active)
					{
						std::stringstream ss;
						ss << "Directional Light " << i;
						if (ImGui::TreeNode(ss.str().c_str()))
						{
							ImGui::SliderFloat("Intensity", &lights[i]->intensity, 0, 2);

							ImGui::SliderAngle("Horizontal Direction", &hdir[i], 0.0f, 360.0f);
							ImGui::SliderAngle("Vertical Direction", &vdir[i], -89.0f, 89.0f);

							float height = 1.0f - abs(sin(vdir[i]));
							lights[i]->direction = glm::vec3(cos(hdir[i]) * height, sin(vdir[i]), sin(hdir[i]) * height);

							col[i][0] = lights[i]->color.x;
							col[i][1] = lights[i]->color.y;
							col[i][2] = lights[i]->color.z;
							if (ImGui::ColorEdit3("Color", col[i]))
							{
								lights[i]->color.x = col[i][0];
								lights[i]->color.y = col[i][1];
								lights[i]->color.z = col[i][2];
							}

							ImGui::TreePop();
							ImGui::Separator();
							ImGui::Spacing();
						}
					}
				}
			ImGui::TreePop();
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button("Reload Shaders"))
		{
			shader->~Shader();
			shader = new Shader("Shaders/vertex_core_shaded.glsl", "Shaders/fragment_core_shaded.glsl");
			material->shader = shader;

			fb->ReloadShader();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Debug info
		ImGui::Text("Average %.3f ms/frame | (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void InitImGui()
	{
		// Setting up the ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();

		ImGui::StyleColorsDark();
	}

	void InitGL()
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		window = glfwCreateWindow(windowSize.x, windowSize.y, "Model Inspector", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window!" << std::endl;
			glfwTerminate();
			exit(-1);
		}
		glfwMakeContextCurrent(window);

		glewInit();

		glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
		glfwSetScrollCallback(window, scroll_callback);

		// Allowing for alpha blending (Transparency)
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	bool isRunning() { return !glfwWindowShouldClose(window); }
}