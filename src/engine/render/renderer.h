#pragma once

struct Mesh;

struct RenderOptions
{
	bool wireframe = false;
	bool always_on_top = false;
	float point_size = 1.0;
	float line_width = 1.0;
	// for immediate point shader
	vec3 color = vec3{-1.0};
	float opacity = 1.0;
	bool dont_cull_face = false;
};

void render_mesh(Mesh* mesh, RenderOptions opts = RenderOptions{});
