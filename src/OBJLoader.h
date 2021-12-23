#pragma once

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>

#include <ColorfulLogging.h>

#include <glm.hpp>

#include "Vertex.h"

bool ExtractFromOBJ(std::string path, std::vector<Vertex> &vertices, std::vector<unsigned int>& indices, int& trisCount, int& vertCount);
#endif