#pragma once
#include "engine/entities/Entity.h"

struct EntityType(ESpotLight)
{
	Reflected()

	vec3 Direction = vec3(0, -1, 0);
	vec3 Diffuse = vec3(1);
	vec3 Specular = vec3(1);
	float Innercone = 1;
	float Outercone = 0.5;
	float IntensityConstant = 0.02f;
	float IntensityLinear = 1.0f;
	float IntensityQuadratic = 0.032f;
};

struct EntityType(EPointLight)
{
	Reflected()

	vec3 Diffuse = vec3(1);
	vec3 Specular = vec3(1);
	float IntensityConstant = 0.5f;
	float IntensityLinear = 0.4f;
	float IntensityQuadratic = 0.032f;
};

struct EntityType(EDirectionalLight)
{
	Reflected()

	vec3 Direction = vec3(0, -1, 0);
	vec3 Diffuse = vec3(1);
	vec3 Specular = vec3(1);
};
