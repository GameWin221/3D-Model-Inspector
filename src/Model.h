#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Vertex.h"
#include "Material.h"
#include "Texture.h"
#include "Camera.h"
#include "Light.h"
#include "OBJLoader.h"

class Model
{
public:
	Model(std::string path, Material* _material);
	~Model();

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4 transform;

	void Update();
	void Render(Camera& target_camera);
	void RenderAsShadowmap(DirLight& source_light, glm::mat4& lSpaceMatrix);

	Material* material;

private:

	GLuint VBO, VAO, EBO;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

#endif