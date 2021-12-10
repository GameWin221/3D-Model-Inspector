#include "Framebuffer.h"

Shader* screenShader;
bool shaderLoaded = false;

float quadVertices[] = {
	// Positions   // TexCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

Framebuffer::Framebuffer(glm::uvec2 size)
{
	// Load the main shader if it wasn't loaded yet
	if (!shaderLoaded)
	{
		screenShader = new Shader("Shaders/vertex_framebuffer.glsl", "Shaders/fragment_framebuffer.glsl");
		shaderLoaded = true;
	}

	this->shaderPtr = screenShader;

	// Creating the VAO and VBO for the framebuffer
	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &this->quadVBO);

	glBindVertexArray(this->quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// MSAA Framebuffer
	glGenFramebuffers(1, &this->framebuffer);

	// MSAA Color Texture
	glGenTextures(1, &this->colorTextureMSAA);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->colorTextureMSAA);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_LEVEL, GL_RGB16F, size.x, size.y, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// MSAA Renderbuffer Object
	glGenRenderbuffers(1, &this->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_LEVEL, GL_DEPTH24_STENCIL8, size.x, size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Attach to the framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, this->colorTextureMSAA, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbo);
	
	// Logging the errors
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Failed to create a framebuffer!\n";

	// Intermediate Framebuffer
	glGenFramebuffers(1, &this->intermediateFBO);
	
	// Main Color Texture
	glGenTextures(1, &this->screenTexture);
	glBindTexture(GL_TEXTURE_2D, this->screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Attach to the framebuffer (color attachment)
	glBindFramebuffer(GL_FRAMEBUFFER, this->intermediateFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenTexture, 0);

	// Logging the errors
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Failed to create an intermediate framebuffer!\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	screenShader->Use();
	screenShader->SetInt("screenTexture", 0);

	this->fbSize = size;
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &this->framebuffer);
	glDeleteFramebuffers(1, &this->intermediateFBO);

	glDeleteRenderbuffers(1, &this->rbo);

	glDeleteTextures(1, &this->colorTextureMSAA);
	glDeleteTextures(1, &this->screenTexture);
}

// Render Functions
void Framebuffer::Use()
{
	glViewport(0, 0, this->fbSize.x, this->fbSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
	glEnable(GL_DEPTH_TEST);
}
void Framebuffer::Clear(glm::vec4 clearColor)
{
	// Clear the framebuffer's content (color, depth, stencils)
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
void Framebuffer::Display(unsigned int textureID)
{
	// GL_READ_FRAMEBUFFER is the framebuffer that will be converted to a GL_DRAW_FRAMEBUFFER (MSAA FBO to FBO)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);

	// Converting the multisampled framebuffer to a normal framebuffer ('framebuffer' to 'intermediateFBO')
	int w = this->fbSize.x,
		h = this->fbSize.y;

	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Unbinding the GL_FRAMEBUFFER in order to render the screenTexture to the main window
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	// Rendering the screenTexture
	screenShader->Use();
	glBindVertexArray(quadVAO);

	glActiveTexture(GL_TEXTURE0);

	if(textureID == NULL)
		glBindTexture(GL_TEXTURE_2D, screenTexture);
	else
		glBindTexture(GL_TEXTURE_2D, textureID);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
}
// -----------


void Framebuffer::Resize(glm::uvec2 size)
{
	// If the framebuffer exists and works
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		// Resizing the 'colorTextureMSAA'
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->colorTextureMSAA);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_LEVEL, GL_RGB, size.x, size.y, GL_TRUE);

		// Resisizing the 'rbo'
		glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_LEVEL, GL_DEPTH24_STENCIL8, size.x, size.y);

		// Resizing the 'screenTexture'
		glBindTexture(GL_TEXTURE_2D, this->screenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		// Unbinding
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		this->fbSize = size;
	}
	else
		std::cout << "Failed to resize the framebuffer - No framebuffer created yet\n";
}
void Framebuffer::ReloadShader()
{
	screenShader->~Shader();
	screenShader = new Shader("Shaders/vertex_framebuffer.glsl", "Shaders/fragment_framebuffer.glsl");
}