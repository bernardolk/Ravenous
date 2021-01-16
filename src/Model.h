#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

unsigned int load_texture_from_file(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    /*  Model Data */
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    vector<float> boundingBox;
    unsigned int bb_vao, bb_vbo;

    Model(Mesh mesh) {
        this->meshes.push_back(mesh);
        calculate_bounding_box();
        setup_bounding_box_mesh();
    }

    // draws the model, and thus all its meshes
    void Draw(Shader shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:

    void setup_bounding_box_mesh() {
        glGenVertexArrays(1, &bb_vao);
        glGenBuffers(1, &bb_vbo);
        glBindVertexArray(bb_vao);
        glBindBuffer(GL_ARRAY_BUFFER, bb_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * boundingBox.size(), &boundingBox[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void calculate_bounding_box() {
        float min_x = numeric_limits<float>::max();
        float min_y = numeric_limits<float>::max();
        float min_z = numeric_limits<float>::max();
        float max_x = numeric_limits<float>::min();
        float max_y = numeric_limits<float>::min();
        float max_z = numeric_limits<float>::min();


        vector<Mesh>::iterator mesh = meshes.begin();   
        for (mesh; mesh != meshes.end(); mesh++) {
            vector<Vertex>::iterator vertex = mesh->vertices.begin();
            for (vertex; vertex != mesh->vertices.end(); vertex++) {
                if (min_x > vertex->Position.x)
                    min_x = vertex->Position.x;
                if (max_x < vertex->Position.x)
                    max_x = vertex->Position.x;
                if (min_y > vertex->Position.y)
                    min_y = vertex->Position.y;
                if (max_y < vertex->Position.y)
                    max_y = vertex->Position.y;
                if (min_z > vertex->Position.z)
                    min_z = vertex->Position.z;
                if (max_z < vertex->Position.z)
                    max_z = vertex->Position.z;
            }
        } // This took 1 sec to scan... holy cow

        boundingBox = {
            min_x, min_y, min_z,    //0 0,0,0 
            max_x, min_y, min_z,    //1 1,0,0

            max_x, min_y, min_z,    //2 1,0,0
            max_x, min_y, max_z,    //3 1,0,1

            max_x, min_y, max_z,    //4 1,0,1
            min_x, min_y, max_z,    //5 0,0,1
            
            min_x, min_y, max_z,    //6 0,0,1
            min_x, min_y, min_z,    //7 0,0,0

            min_x, min_y, min_z,    //8 0,0,0
            min_x, max_y, min_z,    //9 0,1,0

            max_x, min_y, min_z,    //10 1,0,0
            max_x, max_y, min_z,    //11 1,1,0

            max_x, min_y, max_z,    //12 1,0,1
            max_x, max_y, max_z,    //13 1,1,1

            min_x, min_y, max_z,    //14 0,0,1
            min_x, max_y, max_z,    //15 0,1,1

            min_x, max_y, min_z,    //16 0,1,0
            max_x, max_y, min_z,    //17 1,1,0

            max_x, max_y, min_z,    //18 1,1,0
            max_x, max_y, max_z,    //19 1,1,1

            max_x, max_y, max_z,    //20 1,1,1
            min_x, max_y, max_z,    //21 0,1,1

            min_x, max_y, max_z,    //22 0,1,1
            min_x, max_y, min_z,    //23 0,1,0
        };
    }
};


unsigned int load_texture_from_file(const char* filename, const string& directory, bool gamma)
{
    string name = string(filename);
  
    string path;
    if (path.substr(0, path.length() - 2) == "/")
        path = directory + name;
    else
        path = directory + "/" + name;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
