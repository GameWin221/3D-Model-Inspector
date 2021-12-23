#include "OBJLoader.h"

bool ExtractFromOBJ(std::string path, std::vector<Vertex> &vertices, std::vector<unsigned int>& indices, int& trisCount, int& vertCount)
{
	std::ifstream file(path);

	if (file.is_open())
	{
		std::vector < Vertex > temp_vertices;
		std::vector < unsigned int > temp_indices;

		std::vector < glm::vec3 > v_positions;
		std::vector < glm::vec2 > v_texcoords;
		std::vector < glm::vec3 > v_normals;

		std::vector < unsigned int > i_positions;
		std::vector < unsigned int > i_texcoords;
		std::vector < unsigned int > i_normals;

		bool has_position_indices = false;
		bool has_uv_indices = false;
		bool has_normal_indices = false;

		cl::BenchmarkBegin("Loading");
		cl::Log("Loading a model from \"" + path + "\". . .\n", cl::Level::Info);

		std::string tmp_line;
		while (std::getline(file, tmp_line) && file.good())
		{
			// Converting tmp_line to a stringstream in order to get the header and the content later on
			std::istringstream line(tmp_line);

			// Getting the header and the content from a line
			std::string header; 
			std::string content_str;

			// Extracting header from the line
			line >> header;

			// Extracting vertex data from the line
			content_str = line.str().erase(0, header.length());
	
			// Creating a stringstream from content_str's data
			std::istringstream content(content_str);

			// Getting vertex positions
			if (header == "v")
			{
				vertCount++;

				has_position_indices = true;

				glm::vec3 pos;
				// Splitting the content to 3 coordinates: x, y, z
				for (int xyz = 0; xyz < 3; xyz++)
				{
					std::string coordinate;
					content >> coordinate;

					if (xyz == 0) pos.x = std::stod(coordinate);
					else if (xyz == 1) pos.y = std::stod(coordinate);
					else if (xyz == 2) pos.z = std::stod(coordinate);
				}

				v_positions.push_back(pos);
			}

			// Getting the texture coordinates
			else if (header == "vt")
			{
				has_uv_indices = true;
				glm::vec2 uv;
				// Splitting the content to 2 coordinates: u, v
				for (int xy = 0; xy < 2; xy++)
				{
					std::string coordinate;
					content >> coordinate;

					if (xy == 0) uv.x = std::stod(coordinate);
					else if (xy == 1) uv.y = std::stod(coordinate);
				}

				v_texcoords.push_back(uv);
			}

			// Getting vertex normals
			else if (header == "vn")
			{
				has_normal_indices = true;
				glm::vec3 normal;

				// Splitting the content to 3 coordinates: x, y, z
				for (int xyz = 0; xyz < 3; xyz++)
				{
					std::string coordinate;
					content >> coordinate;

					if (xyz == 0) normal.x = std::stod(coordinate);
					else if (xyz == 1) normal.y = std::stod(coordinate);
					else if (xyz == 2) normal.z = std::stod(coordinate);
				}

				v_normals.push_back(normal);
			}

			// Getting the indices
			else if (header == "f")
			{
				trisCount++;

				// Array 'v' is an array of 3 triangles
				std::string tris[3];

				// Splitting the whole line into 3 separate 'triangles'
				content >> tris[0];
				content >> tris[1];
				content >> tris[2];

				// Splitting the triangle indices into 3 separate indices for every vert in a triangle (tris[i])
				for (int i = 0; i < 3; i++)
				{
					std::stringstream tmp_content(tris[i]);
					std::string substr;

					if (has_position_indices)
					{
						std::getline(tmp_content, substr, '/');
						i_positions.push_back(std::stoi(substr));
					}
					if (has_uv_indices)
					{
						std::getline(tmp_content, substr, '/');
						i_texcoords.push_back(std::stoi(substr));
					}
					if (has_normal_indices)
					{
						std::getline(tmp_content, substr, '/');
						i_normals.push_back(std::stoi(substr));
					}
				}
			}
		}
		file.close();

		// Assemblying a mesh from the gathered data
		for (unsigned int i = 0; i < i_positions.size(); i++)
		{
			unsigned int pos_index;
			unsigned int uv_index;
			unsigned int normal_index;

			if (has_position_indices)
				pos_index = i_positions[i];
			if (has_uv_indices)
				uv_index = i_texcoords[i];
			if (has_normal_indices)
				normal_index = i_normals[i];

			glm::vec3 final_position(0.0f, 0.0f, 0.0f);
			glm::vec2 final_uv(0.0f, 0.0f);
			glm::vec3 final_normal(0.0f, 0.0f, 0.0f);

			if (has_position_indices)
				final_position = v_positions[pos_index - 1];
			if (has_uv_indices)
				final_uv = v_texcoords[uv_index - 1];
			if (has_normal_indices)
				final_normal = v_normals[normal_index - 1];

			Vertex vertex = {final_position,
							 final_normal,
							 final_uv		};
			temp_vertices.push_back(vertex);
			temp_indices.push_back(i);
		}

		// Getting the time elapsed during the model loading
		double loadTime = cl::BenchmarkStop("Loading");
		cl::Log("Succesfully loaded the model in " + std::to_string(loadTime * 1000) + " milliseconds\n", cl::Level::Success);

		vertices = temp_vertices;
		indices = temp_indices;
		return true;
	}
	else
	{
		cl::Log("Failed to read OBJ data from: \"" + path + "\"\n", cl::Level::Error);
		return false;
	}
}