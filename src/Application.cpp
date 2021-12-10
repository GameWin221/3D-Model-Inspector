#include "Application.h"

namespace Application
{
	glm::ivec2 windowSize(1920, 1080);

	GLFWwindow* window = nullptr;

	Camera* camera = nullptr;

	Framebuffer* fb = nullptr;

	// Set MAXLIGHTS in the "fragment_core_shaded.glsl" and "Shadowmapper.h" to the same value as here
	#define MAXLIGHTS 3
	//#define SHADOWMAP_RES 1024


	DirLight* lights[MAXLIGHTS];
	int activatedLights = 1;

	const float dLightYDir = 0.1f;

	Texture* albe_texture = nullptr;
	Texture* spec_texture = nullptr;

	Shader* shader = nullptr;

	Material* material = nullptr;
	Model* model = nullptr;

	double mouse_x, mouse_y;
	double deltaTime;

	float modelRotation[3];
	float modelPosition[3];
	float modelScale[3] = { 1, 1, 1 };

	float aLightColor[3] = { 0.1f, 0.1f, 0.1f };
	const float aLightIntensity = 0.75f;

	bool shadows = true;
	const unsigned int SHADOW_SIZE = 1024;
	unsigned int depthMapFBO;
	unsigned int depthMap;
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };

	void Init()
	{
		window = InitGL();
		InitImGui(window);

		camera = new Camera(glm::vec3(0.0f, 0.0f, 2.0f));

		shader = new Shader("Shaders/vertex_core_shaded.glsl", "Shaders/fragment_core_shaded.glsl");

		material = new Material(shader, nullptr, nullptr);
		fb = new Framebuffer(windowSize);

		//Shadowmapper::Init(SHADOWMAP_RES);

		for (int i = 0; i < MAXLIGHTS; i++)
		{
			lights[i] = new DirLight();
			lights[i]->active = false;
		}
		lights[0]->active = true;

		glGenFramebuffers(1, &depthMapFBO);
		// create depth texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void Update()
	{
		deltaTime = 1.0 / ImGui::GetIO().Framerate;

		// Input
		processInput(window, *camera, deltaTime);
		glfwPollEvents();

		if (model != nullptr)
		{
			model->position = glm::vec3(modelPosition[0], modelPosition[1], modelPosition[2]);
			model->rotation = glm::vec3(modelRotation[0], modelRotation[1], modelRotation[2]);
			model->scale = glm::vec3(modelScale[0], modelScale[1], modelScale[2]);
		}
	}

	void Render()
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightSpaceMatrix;
		if (shadows)
		{
			glm::mat4 lightProjection, lightView;
			float near_plane = 1.0f, far_plane = 7.5f;
			lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
			lightView = glm::lookAt(lights[0]->direction * 5.0f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
			lightSpaceMatrix = lightProjection * lightView;

			glEnable(GL_DEPTH_TEST);

			glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);

			if (model != nullptr)
			{
				model->Update();
				model->RenderAsShadowmap(*lights[0], lightSpaceMatrix);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// reset viewport
			glViewport(0, 0, windowSize.x, windowSize.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}


		// Rendering to the screen
		fb->Use();
		fb->Clear(glm::vec4(aLightColor[0] / 2.0f, aLightColor[1] / 2.0f, aLightColor[2] / 2.0f, 1.0f));

		shader->Use();
		shader->SetVec3("ambientColor", glm::vec3(aLightColor[0], aLightColor[1], aLightColor[2]));
		shader->SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);

		if (shadows)
			shader->SetInt("shadows", 1);
		else
			shader->SetInt("shadows", 0);

		shader->SetInt("shadowMap", 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		// Rendering
		for (int i = 0; i < MAXLIGHTS; i++)
			lights[i]->UseOn(*shader, i);

		//glActiveTexture(GL_TEXTURE0);

		if (model != nullptr)
		{
			model->Update();
			model->Render(*camera);
		}

		fb->Display();
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
		delete albe_texture;
		delete spec_texture;
		delete shader;
		delete material;
		delete model;

		for (auto& lgt : lights)
			delete lgt;

		glfwTerminate();
	}

	bool isRunning() { return !glfwWindowShouldClose(window); }

	void processInput(GLFWwindow* window, Camera& cam, double& delta_time)
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
				cam.Move(glm::vec3(0.0f, 0.0f, 1.0f * delta_time));
			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				cam.Move(glm::vec3(0.0f, 0.0f, -1.0f * delta_time));

			// Left / Right
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				cam.Move(glm::vec3(-1.0f * delta_time, 0.0f, 0.0f));
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				cam.Move(glm::vec3(1.0f * delta_time, 0.0f, 0.0f));

			// Up / Down
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				cam.Move(glm::vec3(0.0f, 1.0f * delta_time, 0.0f));
			else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				cam.Move(glm::vec3(0.0f, -1.0f * delta_time, 0.0f));

			// Camera Rotation
			if (delta_magnitude > 0)
				cam.Rotate(-delta_x / 10, delta_y / 10);
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

	void DrawImGui()
	{
		// ImGui creating new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui window
		{
			static float f = 0.0f;
			static bool orbital;
			static char modelpath[80] = "";

			static char albedopath[80] = "";
			static char specularpath[80] = "";
			static char normalpath[80] = "";

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

				/*
				static bool normalTex;
				ImGui::Checkbox("Normal Texture", &normalTex);
				if (normalTex)
					ImGui::InputText("Normal Texture Path", normalpath, IM_ARRAYSIZE(normalpath));
				*/

				if (ImGui::Button("(Re)Load the Model and Textures"))
				{
					std::cout << '\n';
					cl::Log("> Loading Begins\n", cl::Level::Info);
					cl::BenchmarkBegin("Main Loading");

					delete albe_texture;
					delete spec_texture;
					delete model;

					if (albedoTex)
						albe_texture = new Texture("OBJs/" + std::string(albedopath));
					else
						albe_texture = new Texture("OBJs/NoTextureWhite.jpg");

					if (specularTex)
						spec_texture = new Texture("OBJs/" + std::string(specularpath));
					else
						spec_texture = new Texture("OBJs/NoTextureWhite.jpg");


					material = new Material(shader, albe_texture, spec_texture);

					if(modelpath != "")
						model = new Model("OBJs/" + std::string(modelpath), material);

					double loadTime = cl::BenchmarkStop("Main Loading");
					cl::BenchmarkStopAll();

					cl::Log("Loading Finished - It took " + std::to_string(loadTime * 1000) + " milliseconds\n", cl::Level::Success);
				}

				ImGui::TreePop();
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::TreeNode("Model Info"))
			{
				if (model != nullptr)
				{
					//ImGui::InputFloat3("Model Position", modelPosition);
					ImGui::InputFloat3("Model Rotation", modelRotation);
					ImGui::InputFloat3("Model Scale", modelScale);

					ImGui::Spacing();

					if (ImGui::Checkbox("Wireframe Mode", &material->wireframe))
					{
						if (material->wireframe)
						{
							glDisable(GL_CULL_FACE);
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						}
						else
						{
							glEnable(GL_CULL_FACE);
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						}
					}

					ImGui::SliderFloat("Shininess", &material->shininess, 1, 256);
					ImGui::SliderFloat("Specular", &material->specular, 0, 3);
				}
				else
					ImGui::Text("No model loaded!");

				ImGui::TreePop();
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::TreeNode("Camera Info"))
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
					{
						camera->position = glm::vec3(0, 0, 0);
					}
				}

				ImGui::SliderFloat("FOV", &camera->fov, camera->minFov, camera->maxFov);
				

				static float gamma = 2.0f;
				if (ImGui::SliderFloat("Gamma", &gamma, 0, 4))
				{
					fb->shaderPtr->SetFloat("gamma", gamma);
				}

				static float exposure = 1.0f;
				if (ImGui::SliderFloat("Exposure", &exposure, 0, 5))
				{
					fb->shaderPtr->SetFloat("exposure", exposure);
				}

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

				ImGui::Checkbox("Shadows", &shadows);

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

							static float hdir[MAXLIGHTS];
							static float vdir[MAXLIGHTS];

							/*
							static bool vinitialized = false;
							if(!vinitialized)
								for (auto& _vdir : vdir)
								{
									_vdir = 1.57079f;
									vinitialized = true;
								}
							*/

							static bool hinitialized = false;
							if (!hinitialized)
								for (auto& _hdir : hdir)
								{
									_hdir = 1.57079f;
									hinitialized = true;
								}

							ImGui::SliderAngle("Horizontal Direction", &hdir[i], 0.0f, 360.0f);
							ImGui::SliderAngle("Vertical Direction", &vdir[i], -90.0f, 90.0f);

							float height = 1 - abs(sin(vdir[i]));
							lights[i]->direction = glm::vec3(cos(hdir[i]) * height, sin(vdir[i]), sin(hdir[i]) * height);

							static float col[MAXLIGHTS][3];
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
		}

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void InitImGui(GLFWwindow* target_window)
	{
		// Setting up the ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(target_window, true);
		ImGui_ImplOpenGL3_Init();

		ImGui::StyleColorsDark();
	}

	GLFWwindow* InitGL()
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		GLFWwindow* window = glfwCreateWindow(windowSize.x, windowSize.y, "Model Inspector", NULL, NULL);
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
		
		// Anti-Aliasing
		//glEnable(GL_MULTISAMPLE);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		return window;
	}
}