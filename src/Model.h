#pragma once

#include <vector>

using namespace std;

struct Model {
   Mesh mesh;
   std::vector<Texture> textures;
   GLData gl_data;
};