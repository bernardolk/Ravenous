#pragma once
#include "engine/entities/Entity.h"

struct EntityType(ESpotLight)
{
	Reflected(ESpotLight)

	Field(vec3, Direction) = vec3(0, -1, 0);
	Field(vec3, Diffuse) = vec3(1);
	Field(vec3, Specular) = vec3(1);
	Field(float, Innercone) = 1;
	Field(float, Outercone) = 0.5;
	Field(float, IntensityConstant) = 0.02f;
	Field(float, IntensityLinear) = 1.0f;
	Field(float, IntensityQuadratic) = 0.032f;
};

struct EntityType(EPointLight)
{
	Reflected(EPointLight)

	Field(vec3, Diffuse) = vec3(1);
	Field(vec3, Specular) = vec3(1);
	Field(float, IntensityConstant) = 0.5f;
	Field(float, IntensityLinear) = 0.4f;
	Field(float, IntensityQuadratic) = 0.032f;
};

struct EntityType(EDirectionalLight)
{
	Reflected(EDirectionalLight)

	Field(vec3, Direction) = vec3(0, -1, 0);
	Field(vec3, Diffuse) = vec3(1);
	Field(vec3, Specular) = vec3(1);
};
