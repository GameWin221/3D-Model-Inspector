#include "Shadowmapper.h"

namespace Shadowmapper
{
	glm::uvec2 shadowmapResolution;
	unsigned int depthFBO, depthTex;
	const float near_plane = 1.0f, far_plane = 7.5f;

	void Init(unsigned int resolution)
	{
		shadowmapResolution = glm::uvec2(resolution);

		// Shadowmap FBO
		glGenFramebuffers(1, &depthFBO);

		// Shadowmap depth texture
		glGenTextures(1, &depthTex);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowmapResolution.x, shadowmapResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Attaching the depth texture to the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Use()
	{
		glViewport(0, 0, shadowmapResolution.x, shadowmapResolution.y);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void CalculateShadows(DirLight* light, Shader* depth_shader)
	{
		//for (int i = 0; i < MAXLIGHTS; i++)
		//{
			glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

			//mozliwe ze musi byc -direction zamiast direction
			glm::mat4 lightView = glm::lookAt(light->direction,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));

			glm::mat4 lightSpaceMatrix = lightProjection * lightView;

			depth_shader->Use();
			depth_shader->SetInt("shadowMap", 0);
			depth_shader->SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTex);
		//}
	}
}