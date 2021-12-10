#include "Texture.h"

void Texture::Use()
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

Texture::Texture(std::string path)
{
	this->originPath = path;

	// Checking the extension to get the right color format
	std::string extension = path.substr(path.length() - 4, path.length());
	std::string errorCause = "";

	if (extension == ".png")
	{
		colorSpace = GL_RGBA16F;
		colorFormat = GL_RGBA;
	}
	else if (extension == ".jpg" || extension == "jpeg")
	{
		colorSpace = GL_RGB16F;
		colorFormat = GL_RGB;
	}
	else
	{
		colorSpace = GL_RGB16F;
		colorFormat = GL_RGB;
		errorCause = " - Unknown file extension";
	}

	// NOTE: stbi library loads textures upside down and that's why these should be flipped afterwards
	stbi_set_flip_vertically_on_load(true);

	// Used to measure elapsed time a bit later;
	cl::BenchmarkBegin("Loading");
	cl::Log("Loading a texture from \"" + path + "\". . .\n", cl::Level::Info);

	// Loading the actual texture
	unsigned char* texture_data = stbi_load(path.c_str(), &this->size.x, &this->size.y, &channels, 0);

	double loadTime = cl::BenchmarkStop("Loading");

	// Creating the texture
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// If the texture was loaded properly, bind the texture etc..
	if (texture_data)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, colorSpace, this->size.x, this->size.y, 0, colorFormat, GL_UNSIGNED_BYTE, texture_data);

		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glGenerateMipmap(GL_TEXTURE_2D);

		cl::Log("Succesfully loaded the texture in " + std::to_string(loadTime * 1000) + " milliseconds\n", cl::Level::Success);
	}

	// If not, load the pink-black checker pattern
	else
	{
		texture_data = stbi_load("OBJs/NoTexture.jpg", &this->size.x, &this->size.y, &channels, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, colorSpace, this->size.x, this->size.y, 0, colorFormat, GL_UNSIGNED_BYTE, texture_data);

		cl::Log("Failed to load the texture data from \"" + path + "\"" + errorCause + "\n", cl::Level::Error);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(texture_data);
}
Texture::~Texture()
{
	glDeleteTextures(1, &ID);
}