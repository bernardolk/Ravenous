#include "GeometryData.h"
#include <glad/glad.h>
#include "Engine/Rvn.h"
#include "Engine/Geometry/Mesh.h"
#include "engine/io/loaders.h"

void LoadModels()
{
	auto Filenames = GetFilesINFolder(Paths::Models);
	if (Filenames.size() > 0)
	{
		for (const auto& ModelFilename : Filenames)
		{
			// check if filename is not a folder (we want only .obj files here)
			auto ExtensionTest = ModelFilename.substr(ModelFilename.length() - 3);
			if (ExtensionTest != "obj")
				continue;
			
			auto ModelName = ModelFilename.substr(0, ModelFilename.length() - 4);
			LoadWavefrontObjAsMesh(Paths::Models, ModelName);
		}
	}
	
	//TEXT
	{
		RGLData TextGlData;
		glGenVertexArrays(1, &TextGlData.VAO);
		glGenBuffers(1, &TextGlData.VBO);
		glBindVertexArray(TextGlData.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, TextGlData.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		auto* TextMesh = new RMesh();
		TextMesh->Name = "text";
		TextMesh->GLData = TextGlData;
		GeometryCatalogue.insert({TextMesh->Name, TextMesh});
	}

	// SLOPE
	// with Z coming at the screen, X to the right, slope starts at x=0 high and goes low on x=1
	std::vector<RVertex> slope_vertex_vec = {
	// bottom
	RVertex{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0.5f, 0.5f)}, //0
	RVertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 0.5f)}, //1
	RVertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 1.0f)}, //2
	RVertex{vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0.5f, 1.0f)}, //3
	// right   
	RVertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.5f, 0.5f, 0.0f), vec2(1.0f, 0.5f)}, //4
	RVertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec2(0.5f, 0.5f)}, //5
	RVertex{vec3(0.0f, 1.0f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec2(1.0f, 1.0f)}, //6
	RVertex{vec3(0.0f, 1.0f, 1.0f), vec3(0.5f, 0.5f, 0.0f), vec2(0.5f, 1.0f)}, //7
	// front       
	RVertex{vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f)}, //8
	RVertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.5f, 0.0f)}, //9
	RVertex{vec3(0.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.5f)}, //10
	// back
	RVertex{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.0f)}, //11
	RVertex{vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.5f)}, //12
	RVertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.5f, 0.0f)}, //13
	// left
	RVertex{vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)}, //14
	RVertex{vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.5f, 0.0f)}, //15
	RVertex{vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.5f)}, //16
	RVertex{vec3(0.0f, 1.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.5f, 0.5f)}, //17
	};

	std::vector<uint> slope_vertex_indices =
	{
	0, 1, 2, 2, 3, 0,       // bottom face
	8, 9, 10,               // front
	11, 12, 13,             // back
	14, 15, 16, 17, 16, 15, // left face
	4, 5, 6, 6, 7, 4        // right face (slope)
	};

	auto* SlopeMesh = new RMesh();
	SlopeMesh->Name = "slope";
	SlopeMesh->Vertices = slope_vertex_vec;
	SlopeMesh->Indices = slope_vertex_indices;
	SlopeMesh->RenderMethod = GL_TRIANGLES;
	SlopeMesh->SetupGLData();
	GeometryCatalogue.insert({SlopeMesh->Name, SlopeMesh});
}
