#include "GeometryData.h"
#include <glad/glad.h>
#include "Engine/Rvn.h"
#include "Engine/Geometry/Mesh.h"
#include "engine/io/loaders.h"


void LoadModels()
{
	//TEXT
	{
		RGLData text_gl_data;
		glGenVertexArrays(1, &text_gl_data.VAO);
		glGenBuffers(1, &text_gl_data.VBO);
		glBindVertexArray(text_gl_data.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, text_gl_data.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		auto* text_mesh = new RMesh();
		text_mesh->name = "text";
		text_mesh->gl_data = text_gl_data;
		GeometryCatalogue.insert({text_mesh->name, text_mesh});
	}


	// AABB
	LoadWavefrontObjAsMesh(Paths::Models, "aabb");

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

	auto* slope_mesh = new RMesh();
	slope_mesh->name = "slope";
	slope_mesh->vertices = slope_vertex_vec;
	slope_mesh->indices = slope_vertex_indices;
	slope_mesh->render_method = GL_TRIANGLES;
	slope_mesh->SetupGLData();
	GeometryCatalogue.insert({slope_mesh->name, slope_mesh});

	// PLAYER CAPSULE
	LoadWavefrontObjAsMesh(Paths::Models, "capsule");

	// LIGHTBULB
	LoadWavefrontObjAsMesh(Paths::Models, "lightbulb");

	// TRIGGER (CYLINDER)
	LoadWavefrontObjAsMesh(Paths::Models, "trigger");
}