#pragma once

#include "engine/core/core.h"

struct RVertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	vec3 Tangent;
	vec3 Bitangent;

	RVertex() : Position({}), Normal({}), TexCoords({}), Tangent({}), Bitangent({}) {}
	RVertex(vec3 Position) : Position(Position), Normal({}), TexCoords({}), Tangent({}), Bitangent({}) {}
	RVertex(vec3 Position, vec3 Normal, vec2 TexCoords) : Position(Position), Normal(Normal), TexCoords(TexCoords), Tangent({}), Bitangent({}) {}
	RVertex(vec3 Position, vec3 Normal, vec2 TexCoords, vec3 Tangent, vec3 Bitangent) : Position(Position), Normal(Normal), TexCoords(TexCoords), Tangent(Tangent), Bitangent(Bitangent) {}
	
	RVertex operator*(mat4 Mat)
	{
		// should do something for tex_coords ???

		auto NPosition = vec3(Mat * vec4(Position, 1.f));
		auto NNormal = vec3(Mat * vec4(Normal, 1.f));
		auto NTangent = vec3(Mat * vec4(Tangent, 1.f));
		auto NBitangent = vec3(Mat * vec4(Bitangent, 1.f));

		return RVertex{NPosition, NNormal, TexCoords, NTangent, NBitangent};
	};

	RVertex& operator*=(mat4 Mat)
	{
		// should do something for tex_coords ???

		Position = vec3(Mat * vec4(Position, 1.f));
		Normal = vec3(Mat * vec4(Normal, 1.f));
		Tangent = vec3(Mat * vec4(Tangent, 1.f));
		Bitangent = vec3(Mat * vec4(Bitangent, 1.f));

		return *this;
	};
};
