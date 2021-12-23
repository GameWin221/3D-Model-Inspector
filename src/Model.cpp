#include "Model.h"

Shader* depthShader = nullptr;

Model::Model(std::string path, Material* _material)
{
	if (depthShader == nullptr)
		depthShader = new Shader("Shaders/shadow_mapping_vertex.glsl", "Shaders/shadow_mapping_fragment.glsl");

	ExtractFromOBJ(path, this->vertices, this->indices, this->triCount, this->vertCount);

	this->material = _material;

	this->material->shader->SetInt("material.diffuseTex", 0);
	this->material->shader->SetInt("material.specularTex", 1);

	this->material->color = glm::vec3(1.0f, 1.0f, 1.0f);

	this->transform = glm::mat4(1.0f);

	this->position = glm::vec3(0.0f);
	this->rotation = glm::vec3(0.0f);
	this->scale = glm::vec3(1.0f);

	// Generating the buffers
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	// Binding data to the buffers
	glBindVertexArray(this->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), this->vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Model::~Model()
{
	this->vertices.clear();
	this->indices.clear();

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

glm::mat4 mat(1.0f);
void Model::Update()
{
	this->transform = glm::translate(mat, this->position);
	this->transform = glm::rotate(this->transform, glm::radians(this->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	this->transform = glm::rotate(this->transform, glm::radians(this->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	this->transform = glm::rotate(this->transform, glm::radians(this->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	this->transform = glm::scale(this->transform, this->scale);
}
void Model::Render(Camera* target_camera)
{
	int windowWidth, windowHeight;

	glfwGetFramebufferSize(glfwGetCurrentContext(), &windowWidth, &windowHeight);

	this->material->shader->Use();
	this->material->shader->SetMatrix4("projection", target_camera->GetProjectionMatrix(windowWidth, windowHeight));
	this->material->shader->SetMatrix4("view", target_camera->GetViewMatrix());
	this->material->shader->SetMatrix4("model", this->transform);

	this->material->shader->SetVec3("viewPos", target_camera->position);
		
	this->material->shader->SetVec3("material.color", this->material->color);
	this->material->shader->SetFloat("material.specular", this->material->specular);
	this->material->shader->SetFloat("material.shininess", this->material->shininess);
	this->material->shader->SetBool("material.wireframe", this->material->wireframe);

	this->material->shader->SetInt("material.diffuseTex", 0);
	this->material->shader->SetInt("material.specularTex", 1);

	// Diffuse Map
	glActiveTexture(GL_TEXTURE0);
	this->material->albedoTex->Use();

	// Specular Map
	glActiveTexture(GL_TEXTURE1);
	this->material->specularTex->Use();

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);

	if (this->material->wireframe)
	{
		glEnable(GL_CULL_FACE);

		this->material->shader->SetBool("material.wireframe", 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);

		this->material->shader->SetBool("material.wireframe", 1);

		glLineWidth(1.5f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	}
	else
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Model::RenderAsShadowmap(DirLight* source_light, glm::mat4& lSpaceMatrix)
{
	int windowWidth, windowHeight;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glfwGetFramebufferSize(glfwGetCurrentContext(), &windowWidth, &windowHeight);
	
	depthShader->Use();
	depthShader->SetMatrix4("model", this->transform);
	depthShader->SetMatrix4("lightSpaceMatrix", lSpaceMatrix);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);

	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}