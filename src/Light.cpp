#include "Light.h"

DirLight::DirLight(glm::vec3 spawnDirection)
{
	this->direction = spawnDirection;
}

void DirLight::UseOn(Shader* shader, int lightIndex)
{
	std::string index = std::to_string(lightIndex);

	shader->Use();
	if (active)
	{
		shader->SetVec3("dirLights[" + index + "].color", this->color);
		shader->SetVec3("dirLights[" + index + "].direction", this->direction);
		shader->SetFloat("dirLights[" + index + "].intensity", this->intensity);
	}
	else
	{
		shader->SetVec3("dirLights[" + index + "].color", glm::vec3(0, 0, 0));
		shader->SetVec3("dirLights[" + index + "].direction", glm::vec3(0, 0, 0));
		shader->SetFloat("dirLights[" + index + "].intensity", 0);
	}
}