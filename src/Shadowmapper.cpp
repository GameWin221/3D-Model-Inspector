#include "Shadowmapper.h"

namespace Shadowmapper
{
	float shadowMapResolution;
	unsigned int depthMapFBO;
	unsigned int depthMap[MAXLIGHTS];
	const float near_plane = 1.0f, far_plane = 7.5f;
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };

	bool shadows = true;

	void EnableShadows()
	{
		shadows = true;
	}

	void DisableShadows()
	{
		shadows = false;
	}

	void GenShadowmaps(unsigned int resolution)
	{
		shadowMapResolution = resolution;

		glGenFramebuffers(1, &depthMapFBO);
		// Create depth textures
		for (int i = 0; i < MAXLIGHTS; i++)
		{
			glGenTextures(1, &depthMap[i]);
			glBindTexture(GL_TEXTURE_2D, depthMap[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderShadowmaps(std::array<DirLight*, MAXLIGHTS> lightsArr, Shader* mainShader, Model* targetModel)
	{
		if (shadows)
		{
			int wX, wY;
			glfwGetFramebufferSize(glfwGetCurrentContext(), &wX, &wY);

			for (int i = 0; i < MAXLIGHTS; i++)
			{
				glm::mat4 lightSpaceMatrix, lightProjection, lightView;
				float near_plane = 0.5f, far_plane = 9.0f;
				lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
				lightView = glm::lookAt(lightsArr[i]->direction * 5.0f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
				lightSpaceMatrix = lightProjection * lightView;

				glEnable(GL_DEPTH_TEST);

				glViewport(0, 0, shadowMapResolution, shadowMapResolution);
				glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap[i], 0);
				glClear(GL_DEPTH_BUFFER_BIT);
				glActiveTexture(GL_TEXTURE0);

				if (targetModel != nullptr)
				{
					targetModel->Update();
					targetModel->RenderAsShadowmap(lightsArr[i], lightSpaceMatrix);
				}

				mainShader->Use();
				mainShader->SetMatrix4("lightSpaceMatrix[" + std::to_string(i) + "]", lightSpaceMatrix);
				mainShader->SetInt("shadowMap[" + std::to_string(i) + "]", 3 + i);
				mainShader->SetInt("shadows", 1);

				glActiveTexture(GL_TEXTURE3 + i);
				glBindTexture(GL_TEXTURE_2D, depthMap[i]);
			}

			// Reset viewport
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, wX, wY);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			mainShader->Use();
			mainShader->SetInt("shadows", 0);
		}
	}
}