#pragma once

inline void load_models()
{
	//TEXT
	{
		GLData text_gl_data;
		glGenVertexArrays(1, &text_gl_data.VAO);
		glGenBuffers(1, &text_gl_data.VBO);
		glBindVertexArray(text_gl_data.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, text_gl_data.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		auto text_mesh = new Mesh();
		text_mesh->name = "text";
		text_mesh->gl_data = text_gl_data;
		Geometry_Catalogue.insert({text_mesh->name, text_mesh});
	}


	// AABB
	load_wavefront_obj_as_mesh(MODELS_PATH, "aabb");

	// SLOPE
	// with Z coming at the screen, X to the right, slope starts at x=0 high and goes low on x=1
	std::vector<Vertex> slope_vertex_vec = {
	// bottom
	Vertex{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0.5f, 0.5f)}, //0
	Vertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 0.5f)}, //1
	Vertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 1.0f)}, //2
	Vertex{vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0.5f, 1.0f)}, //3
	// right   
	Vertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.5f, 0.5f, 0.0f), vec2(1.0f, 0.5f)}, //4
	Vertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec2(0.5f, 0.5f)}, //5
	Vertex{vec3(0.0f, 1.0f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec2(1.0f, 1.0f)}, //6
	Vertex{vec3(0.0f, 1.0f, 1.0f), vec3(0.5f, 0.5f, 0.0f), vec2(0.5f, 1.0f)}, //7
	// front       
	Vertex{vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f)}, //8
	Vertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.5f, 0.0f)}, //9
	Vertex{vec3(0.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.5f)}, //10
	// back
	Vertex{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.0f)}, //11
	Vertex{vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.5f)}, //12
	Vertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.5f, 0.0f)}, //13
	// left
	Vertex{vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)}, //14
	Vertex{vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.5f, 0.0f)}, //15
	Vertex{vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.5f)}, //16
	Vertex{vec3(0.0f, 1.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.5f, 0.5f)}, //17
	};

	std::vector<u32> slope_vertex_indices =
	{
	0, 1, 2, 2, 3, 0,       // bottom face
	8, 9, 10,               // front
	11, 12, 13,             // back
	14, 15, 16, 17, 16, 15, // left face
	4, 5, 6, 6, 7, 4        // right face (slope)
	};

	auto slope_mesh = new Mesh();
	slope_mesh->name = "slope";
	slope_mesh->vertices = slope_vertex_vec;
	slope_mesh->indices = slope_vertex_indices;
	slope_mesh->render_method = GL_TRIANGLES;
	slope_mesh->setup_gl_data();
	Geometry_Catalogue.insert({slope_mesh->name, slope_mesh});

	// PLAYER CAPSULE
	load_wavefront_obj_as_mesh(MODELS_PATH, "capsule");

	// LIGHTBULB
	load_wavefront_obj_as_mesh(MODELS_PATH, "lightbulb");

	// TRIGGER (CYLINDER)
	load_wavefront_obj_as_mesh(MODELS_PATH, "trigger");
}
