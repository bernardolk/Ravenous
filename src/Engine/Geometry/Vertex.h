#pragma once

#include "engine/core/core.h"

struct Vertex
{
	vec3 position;
	vec3 normal;
	vec2 tex_coords;
	vec3 tangent;
	vec3 bitangent;

	Vertex operator*(mat4 mat)
	{
		// should do something for tex_coords ???

		auto n_position = vec3(mat * vec4(position, 1.f));
		auto n_normal = vec3(mat * vec4(normal, 1.f));
		auto n_tangent = vec3(mat * vec4(tangent, 1.f));
		auto n_bitangent = vec3(mat * vec4(bitangent, 1.f));

		return Vertex{n_position, n_normal, tex_coords, n_tangent, n_bitangent};
	};

	Vertex& operator*=(mat4 mat)
	{
		// should do something for tex_coords ???

		position = vec3(mat * vec4(position, 1.f));
		normal = vec3(mat * vec4(normal, 1.f));
		tangent = vec3(mat * vec4(tangent, 1.f));
		bitangent = vec3(mat * vec4(bitangent, 1.f));

		return *this;
	};
};
